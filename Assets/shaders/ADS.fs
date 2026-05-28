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
uniform float u_BaseColorFactor = 1.0f;

uniform float u_RoughnessFactor = 1.0f;
uniform float u_MetallicFactor = 1.0f;
uniform float u_OcclusionFactor = 1.0f;

layout(bindless_sampler) uniform sampler2D u_EmissionMap;
uniform float     u_EmissionMapStrength = 1.0;

layout(bindless_sampler) uniform sampler2D shadowMap; 
layout(bindless_sampler) uniform sampler2D u_SsaoMap; 


layout(bindless_sampler) uniform sampler2D u_OcclusionMap;
layout(bindless_sampler) uniform sampler2D u_RoughnessMap;
layout(bindless_sampler) uniform sampler2D u_MetallicMap;
uniform bool u_UseSeparateMaps = false;

uniform bool u_NormalMapFlipY = false; // true для DirectX, false для OpenGL


uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D   u_BrdfLUT;

uniform vec3  u_CameraPos;
uniform float u_Exposure = 1.0;
uniform float u_IBLStrength = 0.1;
uniform bool  u_UseIBL = true;
uniform bool u_EnableFog = false;

// ----------------- Fog Uniforms -----------------
uniform vec3  u_FogColor = vec3(0.5, 0.6, 0.7);     
uniform float u_FogDensity = 0.015;                 
uniform float u_FogHeight = 0.0;                    
uniform float u_FogHeightFalloff = 0.1;             
uniform float u_FogInscatteringIntensity = 1.0;    
uniform float u_FogPhaseG = 0.5;                    
uniform float u_FogAmbientIntensity = 0.1;          

uniform float u_NormalMapStrength = 1.0;
uniform mat4 lightView;
uniform mat4 lightProj;
uniform float u_LightSize = 0.002;

uniform vec2 u_ScreenResolution;
uniform bool  u_UseSSAO = true;


const float PI = 3.14159265359;

// ----------------- Light Structures -----------------
struct DirectionalLight {
    vec3 direction;
    vec3 Ld;
};

struct PointLight {
    vec3 Ld;
    vec3 position;
    float intensity;
    float point_const_coof;
    float point_linear_coof;
    float point_exp_coof;
    float farPlane;
    int hasShadow;
    samplerCube pointShadowMap;
};

layout (std140, binding = 0) uniform PointLightBlock {
    PointLight lightPoint[32];
    int _COUNT_OF_POINTLIGHT_;
};


uniform DirectionalLight light[8];
uniform int _COUNT_OF_DIRECTIONLIGHT_;

// uniform PointLight lightPoint[32];
// uniform int _COUNT_OF_POINTLIGHT_;

//layout(bindless_sampler) uniform samplerCube u_PointShadowMaps[32];

// ----------------- Helpers & Shadows -----------------
float InterleavedGradientNoise(vec2 px) {
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(px, magic.xy)));
}

vec2 VogelDiskSample(int i, int n, float angle) {
    float goldenAngle = 2.39996323;
    float r = sqrt((float(i) + 0.5) / float(n));
    float theta = float(i) * goldenAngle + angle;
    return vec2(cos(theta), sin(theta)) * r;
}

float FindBlockerDepth(vec2 uv, float zReceiver, float searchRadiusUV, float rotation) {
    float sumDepth = 0.0; int blockers = 0;
    for (int i = 0; i < 24; i++) {
        vec2 offset = VogelDiskSample(i, 24, rotation) * searchRadiusUV;
        float depth = texture(shadowMap, uv + offset).r;
        if (depth < zReceiver) { sumDepth += depth; blockers++; }
    }
    return (blockers == 0) ? -1.0 : sumDepth / float(blockers);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0 || projCoords.z < 0.0) return 0.0;
    vec2 uv = projCoords.xy;
    float zReceiver = projCoords.z;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831853;
    float ndotl = clamp(dot(normal, lightDir), 0.0, 1.0);
    float bias = max(0.0025 * (1.0 - ndotl), 0.00035);
    float searchRadiusUV = clamp(u_LightSize * 250.0, 2.0, 16.0) * texelSize.x;
    float avgBlockerDepth = FindBlockerDepth(uv, zReceiver - bias, searchRadiusUV, rotation);
    if (avgBlockerDepth < 0.0) return 0.0;
    float penumbra = (zReceiver - avgBlockerDepth);
    float filterRadiusUV = clamp(penumbra * u_LightSize * 3500.0, 2.0, 25.0) * texelSize.x;
    float shadow = 0.0;
    for (int i = 0; i < 64; i++) {
        vec2 offset = VogelDiskSample(i, 64, rotation) * filterRadiusUV;
        float pcfDepth = texture(shadowMap, uv + offset).r;
        shadow += ((zReceiver - bias) > pcfDepth) ? 1.0 : 0.0;
    }
    return shadow / 64.0;
}

float PointShadowCalculation(
    vec3 fragPos,
    vec3 lightPos,
    float farPlane,
    samplerCube shadowCube,
    vec3 N
) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    vec3 forward = normalize(fragToLight);

    vec3 up = abs(forward.y) < 0.999
        ? vec3(0.0, 1.0, 0.0)
        : vec3(1.0, 0.0, 0.0);

    vec3 right = normalize(cross(up, forward));
    vec3 tangent = cross(forward, right);

    float bias =
        max(0.002 * (1.0 - dot(N, -forward)), 0.0005);

    float shadow = 0.0;
    int samples = 32;

    float rotation =
        InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831;

    float radius =
        mix(0.002, 0.03, currentDepth / farPlane);

    for (int i = 0; i < samples; ++i) {

        vec2 disk =
            VogelDiskSample(i, samples, rotation) * radius;

        vec3 sampleDir =
            forward +
            right * disk.x +
            tangent * disk.y;

        float closestDepth =
            texture(
                shadowCube,
                normalize(sampleDir)
            ).r * farPlane;

        shadow +=
            (currentDepth - bias > closestDepth)
            ? 1.0
            : 0.0;
    }

    return shadow / float(samples);
}

// ----------------- PBR Math -----------------
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness; float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 1e-7);
}

float GeometrySchlickGGX(float NdotV, float k) {
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nv = max(dot(N, V), 0.0);
    float nl = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(nv, k) * GeometrySchlickGGX(nl, k);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------- FOG LOGIC -----------------

float HenyeyGreenstein(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 / (4.0 * PI)) * ((1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

float CalculateHeightFog(vec3 camPos, vec3 worldPos) {
    vec3 viewDir = worldPos - camPos;
    float dist = length(viewDir);
    vec3 dir = viewDir / dist;

    float fogAtCamera = u_FogDensity * exp(-u_FogHeightFalloff * (camPos.y - u_FogHeight));
    float slope = dir.y;
    if (abs(slope) < 0.0001) slope = 0.0001;
    float fogAmount = (fogAtCamera / (u_FogHeightFalloff * slope)) * (1.0 - exp(-u_FogHeightFalloff * slope * dist));
    
    return clamp(fogAmount, 0.0, 1.0);
}

vec3 ApplyLightAwareFog(vec3 surfaceColor, vec3 worldPos, vec3 camPos, vec3 V) {
    float fogFactor = CalculateHeightFog(camPos, worldPos);
    if (fogFactor <= 0.001) return surfaceColor;

    vec3 fogLighting = u_FogColor * u_FogAmbientIntensity;

    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3 L = normalize(-light[i].direction);
        float cosTheta = dot(V, -L);
        float phase = HenyeyGreenstein(cosTheta, u_FogPhaseG);
        float shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, vec3(0,1,0), L) : 0.0;
        fogLighting += light[i].Ld * phase * u_FogInscatteringIntensity * (1.0 - shadow);
    }

    for (int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        float d = length(lightPoint[i].position - worldPos);
        float atten = 1.0 / max(lightPoint[i].point_const_coof + lightPoint[i].point_linear_coof * d + lightPoint[i].point_exp_coof * d * d, 0.001);
        fogLighting += lightPoint[i].Ld * lightPoint[i].intensity * atten * 0.5;
    }

    return mix(surfaceColor, fogLighting, fogFactor);
}

// ----------------- Post Process -----------------
vec3 ApplyPostProcessing(vec3 color) {
    float a = 2.51; float b = 0.03; float c = 2.43; float d = 0.59; float e = 0.14;
    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    vec2 uv = v_TexCoords;

    vec2 screenUV = gl_FragCoord.xy / u_ScreenResolution;


    vec4 albedoSample = texture(u_AlbedoMap, uv);

    if (albedoSample.a < 0.01) {
        discard;
    }
    vec3 albedo = pow(albedoSample.rgb, vec3(2.2)) * pow(u_BaseColorFactor, 2.2);
    
    vec3 nMap = texture(u_NormalMap, uv).rgb * 2.0 - 1.0;

    if(u_NormalMapFlipY) {
        nMap.y = -nMap.y;
    }


    nMap.xy *= u_NormalMapStrength;
    vec3 emission = pow(texture(u_EmissionMap, uv).rgb, vec3(2.2)) * u_EmissionMapStrength;
    float ao, roughness, metallic;

    if (u_UseSeparateMaps)
    {
        vec3 orm  = texture(u_ORMMap, uv).rgb;
        ao        = orm.r;
        roughness = orm.g;
        metallic  = orm.b;
        // ao        = texture(u_OcclusionMap, uv).r;
        // roughness = texture(u_RoughnessMap, uv).r;
        // metallic  = texture(u_MetallicMap, uv).r;
    }
    else
    {
        vec3 orm  = texture(u_ORMMap, uv).rgb;
        ao        = orm.r;
        roughness = orm.g;
        metallic  = orm.b;
    }


    float ssao = 1.0;
    if (u_UseSSAO) {
        // Используем экранные координаты для выборки SSAO
        vec2 screenUV = gl_FragCoord.xy / u_ScreenResolution;
        ssao = texture(u_SsaoMap, screenUV).r;
    }


    ao        *= u_OcclusionFactor;
    roughness = clamp(roughness * u_RoughnessFactor, 0.05, 1.0);
    metallic  = clamp(metallic * u_MetallicFactor, 0.0, 1.0);


    vec3 N = normalize(TBN * nMap);
    vec3 V = normalize(u_CameraPos - FragPos);
    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3 L = normalize(-light[i].direction);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        float shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (1.0 - shadow) * (kD * albedo / PI + spec) * light[i].Ld * NdotL;
    }

    for (int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        vec3 L = normalize(lightPoint[i].position - FragPos);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        float dist = length(lightPoint[i].position - FragPos);
        float atten = 1.0 / max(lightPoint[i].point_const_coof + lightPoint[i].point_linear_coof * dist + lightPoint[i].point_exp_coof * dist * dist, 0.001);
        float shadow = (lightPoint[i].hasShadow == 1) ? PointShadowCalculation(FragPos, lightPoint[i].position, lightPoint[i].farPlane, lightPoint[i].pointShadowMap, N) : 0.0;

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (1.0 - shadow) * (kD * albedo / PI + spec) * lightPoint[i].Ld * lightPoint[i].intensity * atten * NdotL;
    }

    float materialAO = roughness;
    float combinedAO = materialAO * ssao; // Объединяем оба вида AO

    vec3 ambient = vec3(0.0);
    
    if (u_UseIBL) {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 irradiance = textureLod(u_IrradianceMap, N, 0.0).rgb;
        vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * 7.0).rgb;
        vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;

        // Диффузная часть IBL
        vec3 diffuseIBL = irradiance * albedo;
        // Спекулярная часть IBL
        vec3 specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y);

        // Применяем AO. SSAO сильнее всего должен влиять на диффузный свет.
        // Для спекуляра можно использовать "Specular Occlusion" (трюк Себастьяна Лагарда)
        float specAO = clamp(pow(NdotV + combinedAO, roughness) - 1.0 + combinedAO, 0.0, 1.0);
        
        ambient = ((vec3(1.0) - F_ibl) * (1.0 - metallic) * diffuseIBL + specularIBL * specAO) * combinedAO * u_IBLStrength;
    } else {
        // Обычный константный эмбиент
        ambient = vec3(0.03) * albedo * combinedAO;
    }


    vec3 finalColor = ambient + Lo + emission;

    // Apply Fog (Light-Aware)
    if (u_EnableFog) {
        finalColor = ApplyLightAwareFog(finalColor, FragPos, u_CameraPos, V);
    }


    FragColor = vec4(ApplyPostProcessing(finalColor * u_Exposure), 1.0);
}