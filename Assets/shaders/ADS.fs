#version 330 core

out vec4 FragColor;

// --- Входные данные ---
in vec3 FragPos;
in vec3 oNormal;
in vec2 TexCoords;
in mat3 TBN;
in vec4 FragPosLightSpace; // Должно передаваться из вершинного шейдера

// --- Текстуры ---
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D shadowMap; // Карта теней

// --- Параметры ---
uniform vec3  u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AO;
uniform vec3  viewPos;

// --- Освещение ---
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

struct PBRMaterial {
    vec3  albedo;
    float metallic;
    float roughness;
    float ao;
    vec3  normal;
};

// ============================================================================
// 1. Расчет теней (Shadow Mapping)
// ============================================================================

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // 1. Перспективное деление
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 2. Перевод в диапазон [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    
    // Если фрагмент за пределами дальней плоскости пирамиды света — тени нет
    if(projCoords.z > 1.0) return 0.0;

    // 3. Получаем текущую глубину из пространства света
    float currentDepth = projCoords.z;
    
    // 4. Смещение (Bias) для предотвращения Shadow Acne
    // Чем острее угол падения света, тем больше смещение
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // 5. PCF (Percentage Closer Filtering) — Мягкие тени
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

// ============================================================================
// 2. Математика PBR (Cook-Torrance BRDF)
// ============================================================================

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
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalculateBRDF(vec3 L, vec3 V, vec3 F0, PBRMaterial mat, vec3 radiance) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(mat.normal, L), 0.0);
    float NdotV = max(dot(mat.normal, V), 0.0);

    float D = DistributionGGX(mat.normal, H, mat.roughness);   
    float G = GeometrySmith(mat.normal, V, L, mat.roughness);      
    vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = D * G * F; 
    float denominator = 4.0 * NdotV * NdotL + 0.001; 
    vec3 specular     = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - mat.metallic);

    return (kD * mat.albedo / PI + specular) * radiance * NdotL;
}

// ============================================================================
// 3. Функции обработки данных
// ============================================================================

PBRMaterial GetMaterialData() {
    PBRMaterial mat;
    vec2 uv = TexCoords * 4.0; 

    mat.albedo = pow(texture(albedoMap, uv).rgb, vec3(2.2));
    mat.metallic  = texture(metallicMap, uv).r * u_Metallic;
    mat.roughness = max(texture(roughnessMap, uv).r * u_Roughness, 0.05);
    mat.ao = texture(aoMap, uv).r;

    vec3 tangentNormal = texture(normalMap, uv).rgb * 2.0 - 1.0;
    mat.normal = normalize(TBN * tangentNormal);
    
    return mat;
}

vec3 ApplyPostProcessing(vec3 color) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    color = clamp((color*(a*color+b))/(color*(c*color+d)+e), 0.0, 1.0);
    return pow(max(color, vec3(0.0)), vec3(1.0/2.2));
}

// ============================================================================
// MAIN logic
// ============================================================================

void main() {
    PBRMaterial mat = GetMaterialData();
    vec3 V = normalize(viewPos - FragPos);
    vec3 F0 = mix(vec3(0.04), mat.albedo, mat.metallic);

    vec3 Lo = vec3(0.0);

    // 1. Направленные источники (с тенями)
    for(int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3 L = normalize(-light[i].direction);
        
        float shadow = 0.0;
        // Считаем тень только для ПЕРВОГО источника (обычно это солнце)
        if (i == 0) {
            shadow = ShadowCalculation(FragPosLightSpace, mat.normal, L);
        }
        
        vec3 brdf = CalculateBRDF(L, V, F0, mat, light[i].Ld);
		Lo += (1.0 - shadow) * brdf;

    }

    // 2. Точечные источники (без теней)
    for(int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        vec3 L = normalize(lightPoint[i].position - FragPos);
        float distance = length(lightPoint[i].position - FragPos);
        float attenuation = 1.0 / (lightPoint[i].point_const_coof + lightPoint[i].point_linear_coof * distance + lightPoint[i].point_exp_coof * (distance * distance));
        vec3 radiance = lightPoint[i].Ld * lightPoint[i].intensity * attenuation;

        Lo += CalculateBRDF(L, V, F0, mat, radiance);
    }

    // 3. Фоновое освещение
    vec3 ambient = vec3(0.01) * mat.albedo * mat.ao;

    vec3 color = ambient + Lo;
	
    // Коррекция AO
    color = mix(color, color * mat.ao, mat.roughness);

    FragColor = vec4(ApplyPostProcessing(color), 1.0);
}