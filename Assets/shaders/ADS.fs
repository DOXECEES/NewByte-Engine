#version 430 core

out vec4 FragColor;

// --- Входные данные ---
in vec3 FragPos;
in vec2 v_TexCoords;
in mat3 TBN;
in vec4 FragPosLightSpace;

// --- Текстуры ---
layout (binding = 0) uniform sampler2D albedoMap;
layout (binding = 2) uniform sampler2D normalMap;
layout (binding = 1) uniform sampler2D ormMap;        // R: AO, G: Roughness, B: Metallic
layout (binding = 3) uniform sampler2D shadowMap;

// IBL текстуры
layout (binding = 4) uniform samplerCube u_IrradianceMap;
layout (binding = 5) uniform samplerCube u_PrefilterMap;
layout (binding = 6) uniform sampler2D   u_BrdfLUT;

// --- Параметры ---
uniform vec3  u_CameraPos;
uniform float u_Exposure = 1.0;
uniform float u_IBLStrength = 0.5;
uniform bool  u_UseIBL = true;

uniform mat4 lightView;
uniform mat4 lightProj;


uniform float u_LightSize = 0.002; 
const float PI = 3.14159265359;

// --- Структуры света ---
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

// ────────────────────────────────────────────────
// Математика теней (Шум и Vogel Disk)
// ────────────────────────────────────────────────

#define SHADOW_SAMPLES 64
#define BLOCKER_SAMPLES 24

float InterleavedGradientNoise(vec2 px)
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(px, magic.xy)));
}

// Vogel Disk Sampling (лучше чем твой псевдо-хаммерсли)
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

    // Вне shadowmap
    if (projCoords.z > 1.0 || projCoords.z < 0.0)
    {
        return 0.0;
    }

    vec2 uv = projCoords.xy;
    float zReceiver = projCoords.z;

    vec2 texSize = vec2(textureSize(shadowMap, 0));
    vec2 texelSize = 1.0 / texSize;

    // стабильный шум
    float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831853;

    // --- Bias (стабильный, slope-based) ---
    float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
    float bias = max(0.0025 * (1.0 - cosTheta), 0.0004);

    // --- STEP 1: Blocker Search ---
    // Радиус поиска в texels (важно!)
    float searchRadiusTexels = clamp(u_LightSize * 250.0, 2.0, 16.0);
    float searchRadiusUV = searchRadiusTexels * texelSize.x;

    float avgBlockerDepth = FindBlockerDepth(uv, zReceiver - bias, searchRadiusUV, rotation);

    // Нет блокеров = нет тени
    if (avgBlockerDepth < 0.0)
    {
        return 0.0;
    }

    // --- STEP 2: Penumbra size ---
    // Для directional света в ortho: (receiver - blocker) даёт масштаб пенумбры
    float penumbra = (zReceiver - avgBlockerDepth);

    // переводим в texels (очень важно)
    float filterRadiusTexels = penumbra * u_LightSize * 3500.0;

    // ограничиваем, иначе расползётся
    filterRadiusTexels = clamp(filterRadiusTexels, 2.0, 25.0);

    float filterRadiusUV = filterRadiusTexels * texelSize.x;

    // --- STEP 3: PCF Filtering ---
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

// ────────────────────────────────────────────────
// PBR Функции
// ────────────────────────────────────────────────

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

// ────────────────────────────────────────────────
// MAIN
// ────────────────────────────────────────────────

void main() {
    // 1. Сбор данных материала
    vec2 uv = v_TexCoords;
    vec3 albedo = (textureSize(albedoMap, 0).x < 1) ? 
                  mix(vec3(0.2), vec3(0.7), mod(floor(uv.x*10)+floor(uv.y*10), 2.0)) : 
                  pow(texture(albedoMap, uv).rgb, vec3(2.2));
    
    vec3 nMap = (textureSize(normalMap, 0).x < 1) ? vec3(0,0,1) : texture(normalMap, uv).rgb * 2.0 - 1.0;
    vec3 orm = (textureSize(ormMap, 0).x < 1) ? vec3(1.0, 0.5, 0.0) : texture(ormMap, uv).rgb;
    
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

    // 2. Прямой свет
    for(int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3 L = normalize(-light[i].direction);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        
        // Считаем тень только для первого источника
        shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        
        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * light[i].Ld * NdotL;
    }

    // 3. Точечный свет ( Lo += ... ) — аналогично, без теней для скорости
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

    // 4. Ambient (IBL)
    vec3 ambient = vec3(0.03) * albedo * ao;
    if (u_UseIBL) {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);
        
        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        vec3 diffuseIBL = irradiance * albedo;

        vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * 4.0).rgb;
        vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
        vec3 specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y);

        // Specular Occlusion: гасим отражения неба в глубоких тенях и складках
        float specOcclusion = clamp(pow(NdotV + ao, 0.5) - 1.0 + ao, 0.0, 1.0);
        
        // ВАЖНО: Тень shadow НЕ должна полностью чернить ambient, 
        // но specular должен затухать.
        ambient = (kD_ibl * irradiance * albedo + prefiltered * (F_ibl * brdf.x + brdf.y) * specOcclusion) * ao * u_IBLStrength;

    }

    // 5. Итог
    vec3 color = ambient + Lo;
    FragColor = vec4(ApplyPostProcessing(color * u_Exposure), 1.0);
//FragColor = vec4(vec3(ao), 1.0);
}