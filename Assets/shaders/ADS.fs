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
uniform sampler2D shadowMap;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D   u_BrdfLUT;

uniform vec3  u_CameraPos;
uniform float u_Exposure = 1.0;
uniform float u_IBLStrength = 0.5;
uniform bool  u_UseIBL = true;

uniform mat4 lightView;
uniform mat4 lightProj;

uniform float u_LightSize = 0.002; 
const float PI = 3.14159265359;

struct DirectionalLight {
    vec3 direction;
    vec3 Ld;
};
struct PointLight {
    vec3 position; vec3 Ld; float intensity;
    float point_const_coof; float point_linear_coof; float point_exp_coof;
};

uniform DirectionalLight light[8];
uniform int _COUNT_OF_DIRECTIONLIGHT_;
uniform PointLight lightPoint[32];
uniform int _COUNT_OF_POINTLIGHT_;

#define SHADOW_SAMPLES 64
#define BLOCKER_SAMPLES 24

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

    if (blockers == 0)
    {
        return -1.0;
    }

    return sumDepth / float(blockers);
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

    vec2 texSize = vec2(textureSize(shadowMap, 0));
    vec2 texelSize = 1.0 / texSize;

    float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831853;

    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float bias = max(0.0025 * (1.0 - cosTheta), 0.0004);

    float searchRadiusTexels = clamp(u_LightSize * 250.0, 2.0, 16.0);
    float searchRadiusUV = searchRadiusTexels * texelSize.x;

    float avgBlockerDepth = FindBlockerDepth(uv, zReceiver - bias, searchRadiusUV, rotation);

    if (avgBlockerDepth < 0.0)
    {
        return 0.0;
    }

    float penumbra = (zReceiver - avgBlockerDepth);
    float filterRadiusTexels = penumbra * u_LightSize * 3500.0;
    filterRadiusTexels = clamp(filterRadiusTexels, 2.0, 25.0);
    float filterRadiusUV = filterRadiusTexels * texelSize.x;

    float shadow = 0.0;

    for (int i = 0; i < SHADOW_SAMPLES; i++)
    {
        vec2 offset = VogelDiskSample(i, SHADOW_SAMPLES, rotation) * filterRadiusUV;
        float pcfDepth = texture(shadowMap, uv + offset).r;

        shadow += (zReceiver - bias > pcfDepth) ? 1.0 : 0.0;
    }

    shadow /= float(SHADOW_SAMPLES);

    return shadow;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2 = roughness * roughness * roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 1e-7);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float k = ((roughness + 1.0) * (roughness + 1.0)) / 8.0;
    float g1v = max(dot(N, V), 0.0) / (max(dot(N, V), 0.0) * (1.0 - k) + k);
    float g1l = max(dot(N, L), 0.0) / (max(dot(N, L), 0.0) * (1.0 - k) + k);
    return g1v * g1l;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 ApplyPostProcessing(vec3 color) {
    float a = 2.51; float b = 0.03; float c = 2.43; float d = 0.59; float e = 0.14;
    color = clamp((color*(a*color+b))/(color*(c*color+d)+e), 0.0, 1.0);
    return pow(color, vec3(1.0/2.2));
}

void main() {
    vec2 uv = v_TexCoords;
    vec3 albedo = pow(texture(u_AlbedoMap, uv).rgb, vec3(2.2));
    vec3 nMap = texture(u_NormalMap, uv).rgb * 2.0 - 1.0;
    vec3 orm = texture(u_ORMMap, uv).rgb;
    
    float ao = orm.r;
    float roughness = clamp(orm.g, 0.05, 1.0);
    float metallic = orm.b;

    vec3 N = normalize(TBN * nMap);
    vec3 V = normalize(u_CameraPos - FragPos);
    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);
    float shadow = 0.0;

    for(int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3 L = normalize(-light[i].direction);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        
        shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        
        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * light[i].Ld * NdotL;
    }

    for(int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        vec3 L = normalize(lightPoint[i].position - FragPos);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        float dist = length(lightPoint[i].position - FragPos);
        float atten = 1.0 / (lightPoint[i].point_const_coof + lightPoint[i].point_linear_coof * dist + lightPoint[i].point_exp_coof * dist * dist);
        vec3 radiance = lightPoint[i].Ld * lightPoint[i].intensity * atten;
        
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
        Lo += ((vec3(1.0)-F)*(1.0-metallic) * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    if (u_UseIBL) {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);
        
        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * 4.0).rgb;
        vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;

        float specOcclusion = clamp(pow(NdotV + ao, 0.5) - 1.0 + ao, 0.0, 1.0);
        ambient = (kD_ibl * irradiance * albedo + prefiltered * (F_ibl * brdf.x + brdf.y) * specOcclusion) * ao * u_IBLStrength;
    }

    vec3 color = ambient + Lo;
    FragColor = vec4(ApplyPostProcessing(color * u_Exposure), 1.0);
}