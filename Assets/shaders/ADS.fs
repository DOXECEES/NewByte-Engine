#version 330 core

out vec4 FragColor;

// --- Входные данные ---
in vec3 FragPos;
in vec3 oNormal;
in vec2 TexCoords;
in mat3 TBN;
in vec4 FragPosLightSpace;

// --- Текстуры ---
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

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
    // Коэффициенты затухания
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
// 1. Математика PBR (Cook-Torrance BRDF)
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

// Универсальная функция расчета отражения (BRDF)
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
// 2. Функции обработки данных
// ============================================================================

PBRMaterial GetMaterialData() {
    PBRMaterial mat;
    vec2 uv = TexCoords * 4.0; // Твой тайлинг

    mat.albedo = pow(texture(albedoMap, uv).rgb, vec3(2.2));
    mat.metallic  = texture(metallicMap, uv).r * u_Metallic;
    mat.roughness = max(texture(roughnessMap, uv).r * u_Roughness, 0.05);
    
    // Читаем AO из красного канала текстуры
    mat.ao = texture(aoMap, uv).r;

    vec3 tangentNormal = texture(normalMap, uv).rgb * 2.0 - 1.0;
    mat.normal = normalize(TBN * tangentNormal);
    
    return mat;
}


// Расчет направленного света
vec3 CalculateDirectionalLight(DirectionalLight l, PBRMaterial mat, vec3 V, vec3 F0) {
    vec3 L = normalize(-l.direction);
    return CalculateBRDF(L, V, F0, mat, l.Ld);
}

// Расчет точечного света
vec3 CalculatePointLight(PointLight l, PBRMaterial mat, vec3 V, vec3 F0) {
    vec3 L = normalize(l.position - FragPos);
    float distance = length(l.position - FragPos);
    
    // Расчет затухания (Attenuation)
    float attenuation = 1.0 / (l.point_const_coof + l.point_linear_coof * distance + l.point_exp_coof * (distance * distance));
    vec3 radiance = l.Ld * l.intensity * attenuation;

    return CalculateBRDF(L, V, F0, mat, radiance);
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

    // 1. Направленные источники
    for(int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        Lo += CalculateDirectionalLight(light[i], mat, V, F0);
    }

    // 2. Точечные источники
    for(int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        Lo += CalculatePointLight(lightPoint[i], mat, V, F0);
    }

    // 3. Фоновое освещение
    vec3 ambient = vec3(0.08) * mat.albedo * mat.ao;

    vec3 color = ambient + Lo;
	
	color = mix(color, color * mat.ao, mat.roughness);

    FragColor = vec4(ApplyPostProcessing(color), 1.0);
}