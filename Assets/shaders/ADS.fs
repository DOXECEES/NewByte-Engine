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
uniform bool  u_UseIBL = false; // ГЛОБАЛЬНЫЙ ПЕРЕКЛЮЧАТЕЛЬ IBL

// --- ТВОИ ОРИГИНАЛЬНЫЕ СТРУКТУРЫ СВЕТА ---
struct DirectionalLight {
    vec3 direction;
    vec3 Ld;
};

struct PointLight {
    vec3 position;
    vec3 Ld;
    float intensity;
    float point_const_coof;
    float point_linear_coof;
    float point_exp_coof;
};

uniform DirectionalLight light[8];
uniform int _COUNT_OF_DIRECTIONLIGHT_;

uniform PointLight lightPoint[32];
uniform int _COUNT_OF_POINTLIGHT_;

const float PI = 3.14159265359;

// ────────────────────────────────────────────────
// PBR Функции
// ────────────────────────────────────────────────

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k + 0.0000001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) * 
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ────────────────────────────────────────────────
// Тени и Пост-обработка
// ────────────────────────────────────────────────

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

vec3 ApplyPostProcessing(vec3 color) {
    float a = 2.51; float b = 0.03; float c = 2.43; float d = 0.59; float e = 0.14;
    color = clamp((color*(a*color+b))/(color*(c*color+d)+e), 0.0, 1.0);
    return pow(max(color, vec3(0.0)), vec3(1.0/2.2));
}

// ────────────────────────────────────────────────
// MAIN
// ────────────────────────────────────────────────

void main() {
    // 1. Сбор данных материала
    vec3 orm = texture(ormMap, v_TexCoords).rgb;
    float ao        = orm.r;
    float roughness = max(orm.g, 0.05);
    float metallic  = orm.b;

    vec3 albedo = pow(texture(albedoMap, v_TexCoords).rgb, vec3(2.2));
    vec3 nMap   = texture(normalMap, v_TexCoords).rgb * 2.0 - 1.0;
    vec3 N      = normalize(TBN * nMap);
    vec3 V      = normalize(u_CameraPos - FragPos);
    vec3 R      = reflect(-V, N);
    
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // 2. Направленные источники
    for(int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3 L = normalize(light[i].direction);
        vec3 H = normalize(V + L);
        vec3 radiance = light[i].Ld; 

        float shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 specular = (D * G * F) / (4.0 * NdotV * max(dot(N, L), 0.0) + 0.001);
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        
        //Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);
    }

    // 3. Точечные источники
    for(int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        vec3 L = normalize(lightPoint[i].position - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPoint[i].position - FragPos);
        float attenuation = 1.0 / (lightPoint[i].point_const_coof + lightPoint[i].point_linear_coof * distance + lightPoint[i].point_exp_coof * (distance * distance));
        vec3 radiance = lightPoint[i].Ld * lightPoint[i].intensity * attenuation;

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 specular = (D * G * F) / (4.0 * NdotV * max(dot(N, L), 0.0) + 0.001);
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        Lo += (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);
    }

    // 4. Расчет Ambient (IBL или Fallback)
    vec3 ambient;
    if (u_UseIBL) {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);
        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        vec3 diffuseIBL = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefiltered = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
        vec3 specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y);

        ambient = (kD_ibl * diffuseIBL + specularIBL) * ao * u_IBLStrength;
    } else {
        // Простая заглушка, если IBL выключен
        ambient = vec3(0.03) * albedo * ao;
    }

    // 5. Итог
    vec3 color = ambient + Lo;
    
    FragColor = vec4(ApplyPostProcessing(color * u_Exposure), 1.0);
}