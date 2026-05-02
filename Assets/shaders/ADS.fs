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

layout(bindless_sampler) uniform sampler2D u_EmissionMap;
uniform float     u_EmissionMapStrength = 1.0;

layout(bindless_sampler) uniform sampler2D shadowMap;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D   u_BrdfLUT;

uniform vec3  u_CameraPos;
uniform float u_Exposure = 1.0;
uniform float u_IBLStrength = 0.1;
uniform bool  u_UseIBL = true;

// ----------------- Updated Fog Uniforms -----------------
uniform vec3  u_FogColor = vec3(0.5, 0.6, 0.7);
uniform float u_FogDensity = 0.015;                // Глобальная плотность (у земли)
uniform float u_FogHeightFalloff = 0.1;           // Как быстро туман редеет с высотой
uniform float u_FogHeight = 0.0;                   // Уровень "земли" для тумана
uniform float u_FogInscatteringExp = 25.0;
uniform float u_FogInscatteringIntensity = 0.8;

uniform float u_NormalMapStrength = 1.0;
uniform mat4 lightView;
uniform mat4 lightProj;
uniform float u_LightSize = 0.002;

const float PI = 3.14159265359;

// (Структуры Light и функции теней/PBR остаются без изменений...)
struct DirectionalLight { vec3 direction; vec3 Ld; };
struct PointLight {
    vec3 position; vec3 Ld; float intensity;
    float point_const_coof; float point_linear_coof; float point_exp_coof;
    float farPlane; int hasShadow;
};

uniform DirectionalLight light[8];
uniform int _COUNT_OF_DIRECTIONLIGHT_;
uniform PointLight lightPoint[32];
uniform int _COUNT_OF_POINTLIGHT_;
layout(bindless_sampler) uniform samplerCube u_PointShadowMaps[32];

// ... (Функции ShadowCalculation, PointShadowCalculation, PBR функции пропускаю для краткости) ...
float DistributionGGX(vec3 N, vec3 H, float roughness) { float a = roughness*roughness; float a2 = a*a; float NdotH = max(dot(N, H), 0.0); float NdotH2 = NdotH*NdotH; float denom = (NdotH2 * (a2 - 1.0) + 1.0); return a2 / (PI * denom * denom + 1e-7); }
float GeometrySchlickGGX(float NdotV, float roughness) { float r = (roughness + 1.0); float k = (r * r) / 8.0; float denom = NdotV * (1.0 - k) + k; return NdotV / max(denom, 1e-6); }
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) { float NdotV = max(dot(N, V), 0.0); float NdotL = max(dot(N, L), 0.0); float ggx2 = GeometrySchlickGGX(NdotV, roughness); float ggx1 = GeometrySchlickGGX(NdotL, roughness); return ggx1 * ggx2; }
vec3 fresnelSchlick(float cosTheta, vec3 F0) { return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); }
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) { return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); }
float InterleavedGradientNoise(vec2 px) { vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189); return fract(magic.z * fract(dot(px, magic.xy))); }
vec2 VogelDiskSample(int i, int n, float angle) { float goldenAngle = 2.39996323; float r = sqrt((float(i) + 0.5) / float(n)); float theta = float(i) * goldenAngle + angle; return vec2(cos(theta), sin(theta)) * r; }
float FindBlockerDepth(vec2 uv, float zReceiver, float searchRadiusUV, float rotation) { float sumDepth = 0.0; int blockers = 0; for (int i = 0; i < 24; i++) { vec2 offset = VogelDiskSample(i, 24, rotation) * searchRadiusUV; float depth = texture(shadowMap, uv + offset).r; if (depth < zReceiver) { sumDepth += depth; blockers++; } } return (blockers == 0) ? -1.0 : sumDepth / float(blockers); }
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) { vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; projCoords = projCoords * 0.5 + 0.5; if (projCoords.z > 1.0 || projCoords.z < 0.0) return 0.0; vec2 uv = projCoords.xy; float zReceiver = projCoords.z; vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0)); float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831853; float ndotl = clamp(dot(normal, lightDir), 0.0, 1.0); float bias = max(0.0025 * (1.0 - ndotl), 0.00035); float searchRadiusUV = clamp(u_LightSize * 250.0, 2.0, 16.0) * texelSize.x; float avgBlockerDepth = FindBlockerDepth(uv, zReceiver - bias, searchRadiusUV, rotation); if (avgBlockerDepth < 0.0) return 0.0; float penumbra = (zReceiver - avgBlockerDepth); float filterRadiusUV = clamp(penumbra * u_LightSize * 3500.0, 2.0, 25.0) * texelSize.x; float shadow = 0.0; for (int i = 0; i < 64; i++) { vec2 offset = VogelDiskSample(i, 64, rotation) * filterRadiusUV; float pcfDepth = texture(shadowMap, uv + offset).r; shadow += ((zReceiver - bias) > pcfDepth) ? 1.0 : 0.0; } return shadow / float(64); }
float PointShadowCalculation(vec3 fragPos, vec3 lightPos, float farPlane, samplerCube shadowCube, vec3 N) { vec3 fragToLight = fragPos - lightPos; float currentDepth = length(fragToLight); float adaptiveBias = max(0.05 * (1.0 - dot(N, normalize(lightPos - fragPos))), 0.005); vec3 offsetFragPos = fragPos + N * 0.015; vec3 dir = offsetFragPos - lightPos; int samples = 32; float filterRadius = 0.05; float shadow = 0.0; float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831; for (int i = 0; i < samples; ++i) { float r = sqrt(float(i) + 0.5) / sqrt(float(samples)); float theta = float(i) * 2.39996 + rotation; vec3 offset = vec3(cos(theta) * r, sin(theta) * r, (r - 0.5) * 2.0) * filterRadius; float closestDepth = texture(shadowCube, dir + offset).r; closestDepth *= farPlane; if (currentDepth - adaptiveBias > closestDepth) shadow += 1.0; } return shadow / float(samples); }

// ----------------- Height Fog Calculation -----------------
// Интеграл экспоненциальной плотности по высоте
float CalculateHeightFog(vec3 camPos, vec3 worldPos, float density, float falloff, float baseHeight)
{
    vec3 viewDir = worldPos - camPos;
    float dist = length(viewDir);
    
    // Плотность в точке камеры и в точке фрагмента
    float fogAtCamera = density * exp(-falloff * (camPos.y - baseHeight));
    
    // Избегаем деления на ноль, если луч горизонтален
    float slope = viewDir.y / dist;
    if (abs(slope) < 0.0001) slope = 0.0001;
    
    // Аналитическое решение интеграла: density * exp(-falloff * y)
    float fogAmount = (fogAtCamera / (falloff * slope)) * (1.0 - exp(-falloff * slope * dist));
    
    return clamp(fogAmount, 0.0, 1.0);
}

vec3 ApplyFog(vec3 color, vec3 camPos, vec3 worldPos, vec3 V, vec3 sunDir, vec3 sunColor)
{
    // Расчет коэффициента высотного тумана
    float fogFactor = CalculateHeightFog(camPos, worldPos, u_FogDensity, u_FogHeightFalloff, u_FogHeight);

    // Рассеивание света от солнца (In-scattering / Sun Glow)
    float scattering = pow(max(dot(V, -sunDir), 0.0), u_FogInscatteringExp);
    vec3 finalFogColor = mix(u_FogColor, sunColor * u_FogInscatteringIntensity, scattering);

    return mix(color, finalFogColor, fogFactor);
}

// ----------------- Tonemap -----------------
vec3 ApplyPostProcessing(vec3 color)
{
    float a = 2.51; float b = 0.03; float c = 2.43; float d = 0.59; float e = 0.14;
    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
    return pow(color, vec3(1.0 / 2.2));
}

void main()
{
    vec2 uv = v_TexCoords;

    vec3 albedo = pow(texture(u_AlbedoMap, uv).rgb, vec3(2.2));
    vec3 nMap   = texture(u_NormalMap, uv).rgb * 2.0 - 1.0;
    nMap.xy *= u_NormalMapStrength;
    vec3 orm    = texture(u_ORMMap, uv).rgb;
    vec3 emission = pow(texture(u_EmissionMap, uv).rgb, vec3(2.2)) * u_EmissionMapStrength;

    float ao        = orm.r;
    float roughness = clamp(orm.g, 0.05, 1.0);
    float metallic  = orm.b;

    vec3 N = normalize(TBN * nMap);
    vec3 V = normalize(u_CameraPos - FragPos);
    vec3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // (Directional Lights и Point Lights остаются без изменений)
    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i)
    {
        vec3 L = normalize(-light[i].direction);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;
        float shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * light[i].Ld * NdotL;
    }

    for (int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i)
    {
        vec3 L = normalize(lightPoint[i].position - FragPos);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;
        float dist = length(lightPoint[i].position - FragPos);
        float atten = 1.0 / max(lightPoint[i].point_const_coof + lightPoint[i].point_linear_coof * dist + lightPoint[i].point_exp_coof * dist * dist, 0.001);
        vec3 radiance = lightPoint[i].Ld * lightPoint[i].intensity * atten;
        float pShadow = (lightPoint[i].hasShadow == 1) ? PointShadowCalculation(FragPos, lightPoint[i].position, lightPoint[i].farPlane, u_PointShadowMaps[i], N) : 0.0;
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (1.0 - pShadow) * (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient/IBL
    vec3 ambient = vec3(0.03) * albedo * ao;
    if (u_UseIBL)
    {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);
        vec3 irradiance = textureLod(u_IrradianceMap, N, 0.0).rgb;
        vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * 7.0).rgb;
        vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
        float specOcclusion = clamp(pow(NdotV + ao, 0.5) - 1.0 + ao, 0.0, 1.0);
        ambient = (kD_ibl * irradiance * albedo + prefiltered * (F_ibl * brdf.x + brdf.y) * specOcclusion) * ao * u_IBLStrength;
    }

    vec3 finalColor = ambient + Lo + emission;

    // ----------------- Apply Height Fog -----------------
    if (_COUNT_OF_DIRECTIONLIGHT_ > 0)
    {
        finalColor = ApplyFog(finalColor, u_CameraPos, FragPos, V, light[0].direction, light[0].Ld);
    }
    else
    {
        // Базовый цвет без солнца
        float fogFactor = CalculateHeightFog(u_CameraPos, FragPos, u_FogDensity, u_FogHeightFalloff, u_FogHeight);
        finalColor = mix(finalColor, u_FogColor, fogFactor);
    }

    FragColor = vec4(ApplyPostProcessing(finalColor * u_Exposure), 1.0);
}