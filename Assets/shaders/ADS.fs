#version 450 core
#extension GL_ARB_bindless_texture : require

out vec4 FragColor;

in vec3 FragPos;
in vec2 v_TexCoords;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_ORMMap;

layout(bindless_sampler) uniform sampler2D shadowMap; // directional shadowmap

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D   u_BrdfLUT;

uniform vec3  u_CameraPos;
uniform float u_Exposure = 1.0;
uniform float u_IBLStrength = 0.1;
uniform bool  u_UseIBL = true;

uniform mat4 lightView;
uniform mat4 lightProj;

uniform float u_LightSize = 0.002;

const float PI = 3.14159265359;

// ----------------- Lights -----------------
struct DirectionalLight
{
    vec3 direction;
    vec3 Ld;
};

struct PointLight
{
    vec3 position;
    vec3 Ld;
    float intensity;

    float point_const_coof;
    float point_linear_coof;
    float point_exp_coof;

    float farPlane;
    int hasShadow;
};

uniform DirectionalLight light[8];
uniform int _COUNT_OF_DIRECTIONLIGHT_;

uniform PointLight lightPoint[32];
uniform int _COUNT_OF_POINTLIGHT_;

// bindless cubemap array for point light shadows
layout(bindless_sampler) uniform samplerCube u_PointShadowMaps[32];

// ----------------- Shadow constants -----------------
#define SHADOW_SAMPLES 64
#define BLOCKER_SAMPLES 24

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

// ----------------- Random / sampling -----------------
float InterleavedGradientNoise(vec2 px)
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(px, magic.xy)));
}

vec2 VogelDiskSample(int i, int n, float angle)
{
    float goldenAngle = 2.39996323;
    float r = sqrt((float(i) + 0.5) / float(n));
    float theta = float(i) * goldenAngle + angle;
    return vec2(cos(theta), sin(theta)) * r;
}

// ----------------- Directional shadow (PCSS) -----------------
float FindBlockerDepth(vec2 uv, float zReceiver, float searchRadiusUV, float rotation)
{
    float sumDepth = 0.0;
    int blockers = 0;

    for (int i = 0; i < BLOCKER_SAMPLES; i++)
    {
        vec2 offset = VogelDiskSample(i, BLOCKER_SAMPLES, rotation) * searchRadiusUV;
        float depth = texture(shadowMap, uv + offset).r;

        if (depth < zReceiver)
        {
            sumDepth += depth;
            blockers++;
        }
    }

    return (blockers == 0) ? -1.0 : sumDepth / float(blockers);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.z < 0.0)
    {
        return 0.0;
    }

    vec2 uv = projCoords.xy;
    float zReceiver = projCoords.z;

    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831853;

    float ndotl = clamp(dot(normal, lightDir), 0.0, 1.0);
    float bias = max(0.0025 * (1.0 - ndotl), 0.00035);

    float searchRadiusUV = clamp(u_LightSize * 250.0, 2.0, 16.0) * texelSize.x;

    float avgBlockerDepth = FindBlockerDepth(uv, zReceiver - bias, searchRadiusUV, rotation);
    if (avgBlockerDepth < 0.0)
    {
        return 0.0;
    }

    float penumbra = (zReceiver - avgBlockerDepth);
    float filterRadiusUV = clamp(penumbra * u_LightSize * 3500.0, 2.0, 25.0) * texelSize.x;

    float shadow = 0.0;

    for (int i = 0; i < SHADOW_SAMPLES; i++)
    {
        vec2 offset = VogelDiskSample(i, SHADOW_SAMPLES, rotation) * filterRadiusUV;
        float pcfDepth = texture(shadowMap, uv + offset).r;
        shadow += ((zReceiver - bias) > pcfDepth) ? 1.0 : 0.0;
    }

    return shadow / float(SHADOW_SAMPLES);
}

// ----------------- Point shadow (Cubemap PCF) -----------------
float PointShadowCalculation(vec3 fragPos, vec3 lightPos, float farPlane, samplerCube shadowCube, vec3 N)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    
    // Смещение, чтобы убрать acne
    float adaptiveBias = max(0.05 * (1.0 - dot(N, normalize(lightPos - fragPos))), 0.005);
    vec3 offsetFragPos = fragPos + N * 0.015; 
    vec3 dir = offsetFragPos - lightPos;

    // --- НАСТРОЙКИ МЯГКОСТИ ---
    int samples = 32; // Увеличим до 32 для гладкости
    float viewDistance = length(u_CameraPos - fragPos);
    
    // Радиус размытия. Увеличьте это число, если края всё еще острые.
    float filterRadius = 0.05; 
    // Можно сделать, чтобы тени размывались сильнее вдали от камеры:
    // float filterRadius = (1.0 + (viewDistance / farPlane)) * 0.02;

    float shadow = 0.0;
    float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831;

    for (int i = 0; i < samples; ++i)
    {
        // Используем Vogel Disk вместо фиксированного массива
        float r = sqrt(float(i) + 0.5) / sqrt(float(samples));
        float theta = float(i) * 2.39996 + rotation;
        
        // Превращаем 2D точку диска в 3D смещение
        vec3 offset = vec3(cos(theta) * r, sin(theta) * r, (r - 0.5) * 2.0) * filterRadius;

        float closestDepth = texture(shadowCube, dir + offset).r;
        closestDepth *= farPlane;

        if (currentDepth - adaptiveBias > closestDepth)
            shadow += 1.0;
    }

    return shadow / float(samples);
}
// ----------------- PBR -----------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 1e-7);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, 1e-6);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) *
                pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------- Tonemap -----------------
vec3 ApplyPostProcessing(vec3 color)
{
    // ACES approximation
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;

    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);

    // gamma
    return pow(color, vec3(1.0 / 2.2));
}

// ----------------- MAIN -----------------
void main()
{
    vec2 uv = v_TexCoords;

    vec3 albedo = pow(texture(u_AlbedoMap, uv).rgb, vec3(2.2));
    vec3 nMap   = texture(u_NormalMap, uv).rgb * 2.0 - 1.0;
    vec3 orm    = texture(u_ORMMap, uv).rgb;

    float ao        = orm.r;
    float roughness = clamp(orm.g, 0.05, 1.0);
    float metallic  = orm.b;

    vec3 N = normalize(TBN * nMap);
    vec3 V = normalize(u_CameraPos - FragPos);
    vec3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // ----------------- Directional Lights -----------------
    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i)
    {
        vec3 L = normalize(-light[i].direction);
        vec3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        float shadow = 0.0;

        // Only first directional light casts shadow
        if (i == 0)
        {
            shadow = ShadowCalculation(FragPosLightSpace, N, L);
        }

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

        vec3 radiance = light[i].Ld;

        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ----------------- Point Lights -----------------
    for (int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i)
    {
        vec3 L = normalize(lightPoint[i].position - FragPos);
        vec3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        float dist = length(lightPoint[i].position - FragPos);

        float attenDenom =
            lightPoint[i].point_const_coof +
            lightPoint[i].point_linear_coof * dist +
            lightPoint[i].point_exp_coof * dist * dist;

        float atten = 1.0 / max(attenDenom, 0.001);

        vec3 radiance = lightPoint[i].Ld * lightPoint[i].intensity * atten;

        float pShadow = 0.0;
        if (lightPoint[i].hasShadow == 1)
        {
            pShadow = PointShadowCalculation(
                FragPos,
                lightPoint[i].position,
                lightPoint[i].farPlane,
                u_PointShadowMaps[i],
                N // Добавьте нормаль здесь
            );
        }

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

        Lo += (1.0 - pShadow) * (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ----------------- IBL -----------------
    vec3 ambient = vec3(0.03) * albedo * ao;

    if (u_UseIBL)
    {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);

        vec3 irradiance = textureLod(u_IrradianceMap, N, 0.0).rgb;

        float maxMip = 7.0;
        vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * maxMip).rgb;

        vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;

        float specOcclusion = clamp(pow(NdotV + ao, 0.5) - 1.0 + ao, 0.0, 1.0);

        vec3 diffuseIBL  = irradiance * albedo;
        vec3 specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y) * specOcclusion;

        ambient = (kD_ibl * diffuseIBL + specularIBL) * ao * u_IBLStrength;
    }

    // ----------------- Final -----------------
    vec3 color = ambient + Lo;

    FragColor = vec4(ApplyPostProcessing(color * u_Exposure), 1.0);
}