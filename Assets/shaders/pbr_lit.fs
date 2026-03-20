#version 430 core

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_FragPos;
in mat3 v_TBN;

// --- ТЕКСТУРЫ МАТЕРИАЛА (Строгие слоты) ---
layout (binding = 0) uniform sampler2D u_AlbedoMap;         // Цвет
layout (binding = 1) uniform sampler2D u_NormalMap;         // Нормаль
layout (binding = 2) uniform sampler2D u_ORMMap;            // R=AO, G=Rough, B=Metal

// --- СИСТЕМНЫЕ IBL ТЕКСТУРЫ ---
layout (binding = 3) uniform samplerCube u_IrradianceMap;   // Диффузный IBL
layout (binding = 4) uniform samplerCube u_PrefilterMap;    // Спекулярный IBL
layout (binding = 5) uniform sampler2D   u_BrdfLUT;         // Таблица BRDF

// --- МНОЖИТЕЛИ (Если текстуры нет, эти значения управляют видом) ---
uniform vec4  u_BaseColorFactor;  // По умолчанию: (1, 1, 1, 1)
uniform float u_RoughnessFactor;  // По умолчанию: 1.0
uniform float u_MetallicFactor;   // По умолчанию: 1.0
uniform float u_OcclusionFactor;  // По умолчанию: 1.0
uniform float u_Emission;         // По умолчанию: 0.0

// --- СИСТЕМНЫЕ ПЕРЕМЕННЫЕ ---
uniform vec3 u_CameraPos;
uniform vec3 u_LightPos;   
uniform vec3 u_LightColor;

const float PI = 3.14159265359;

// --- МАТЕМАТИКА PBR ---

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return num / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    // Для прямого света используется этот коэффициент K
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // 1. ПОДГОТОВКА ДАННЫХ
    // Читаем Альбедо и переводим в линейное пространство
    vec4 albedoTex = texture(u_AlbedoMap, v_TexCoord);
    vec3 albedo = pow(albedoTex.rgb, vec3(2.2));
    float alpha = albedoTex.a * u_BaseColorFactor.a;

    // Читаем ORM (R=AO, G=Rough, B=Metal)
    vec3 orm = texture(u_ORMMap, v_TexCoord).rgb;
    float ao        = mix(1.0, orm.r, u_OcclusionFactor);
    float roughness = clamp(orm.g * u_RoughnessFactor, 0.05, 1.0); // Защита от 0
    float metallic  = clamp(orm.b * u_MetallicFactor, 0.0, 1.0);

    // Нормали (World Space)
    vec3 normal = texture(u_NormalMap, v_TexCoord).rgb;
    normal = normalize(v_TBN * (normal * 2.0 - 1.0));
    
    vec3 N = normal;
    vec3 V = normalize(u_CameraPos - v_FragPos);
    vec3 R = reflect(-V, N); 

    float NdotV = max(dot(N, V), 0.0);

    // Базовое отражение (F0)
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // --- 2. ПРЯМОЙ СВЕТ (ТОЧЕЧНЫЙ) ---
    vec3 L = normalize(u_LightPos - v_FragPos);
    vec3 H = normalize(V + L);
    float distance = length(u_LightPos - v_FragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = u_LightColor * attenuation;

    // Cook-Torrance BRDF
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 numerator    = D * G * F;
    float denominator = 4.0 * NdotV * max(dot(N, L), 0.0) + 0.0001; // Защита от 0
    vec3 specularDirect = numerator / denominator;

    vec3 Lo = (kD * albedo / PI + specularDirect) * radiance * max(dot(N, L), 0.0);

    // --- 3. КОСВЕННЫЙ СВЕТ (IBL) ---
    vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kS_ibl = F_ibl;
    vec3 kD_ibl = (vec3(1.0) - kS_ibl) * (1.0 - metallic);

    // Диффузная составляющая (Irradiance)
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    // Зеркальная составляющая (Prefilter + LUT)
    const float MAX_REFLECTION_LOD = 4.0; 
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F_ibl * brdf.x + brdf.y);

    vec3 ambient = (kD_ibl * diffuseIBL + specularIBL) * ao; 

    // --- 4. ФИНАЛ ---
    vec3 emission = albedo * u_Emission;
    vec3 color = ambient + Lo + emission;

    // Tone Mapping (Reinhard)
    color = color / (color + vec3(1.0));
    // Gamma Correction (Linear -> sRGB)
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}