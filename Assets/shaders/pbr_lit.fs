#version 430 core

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_FragPos;
in mat3 v_TBN;

// Текстуры материала
layout (binding = 0) uniform sampler2D u_AlbedoMap;
layout (binding = 1) uniform sampler2D u_NormalMap;
layout (binding = 2) uniform sampler2D u_ORMMap;

// IBL текстуры
layout (binding = 3) uniform samplerCube u_IrradianceMap;
layout (binding = 4) uniform samplerCube u_PrefilterMap;
layout (binding = 5) uniform sampler2D     u_BrdfLUT;

// Материал-факторы
uniform vec4  u_BaseColorFactor   = vec4(1.0);
uniform float u_RoughnessFactor   = 1.0;
uniform float u_MetallicFactor    = 1.0;
uniform float u_OcclusionFactor   = 1.0;
uniform float u_Emission          = 0.0;

// Система
uniform vec3  u_CameraPos;
uniform vec3  u_LightPos          = vec3(0.0);
uniform vec3  u_LightColor        = vec3(0.0);          // можно поставить 0 → прямой свет выключен
uniform float u_Exposure          = 1.8;
uniform float u_StudioIblStrength = 0.85;               // основной контроль яркости IBL
uniform float u_SpecularMul       = 1.15;

const float PI               = 3.14159265359;
const float MAX_REFLECTION_LOD = 5.0;

// ────────────────────────────────────────────────
// PBR вспомогательные функции
// ────────────────────────────────────────────────

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ────────────────────────────────────────────────
// Tone mapping (Filmic approximation ≈ Blender Filmic Medium)
// ────────────────────────────────────────────────
vec3 tonemapFilmic(vec3 x) {
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

// ────────────────────────────────────────────────
// Функция: прямой точечный свет
// ────────────────────────────────────────────────
vec3 computeDirectLight(
    vec3 N, vec3 V, vec3 albedo, float metallic, float roughness,
    vec3 F0, vec3 lightPos, vec3 lightColor
) {
    if (length(lightColor) < 0.001) {
        return vec3(0.0);
    }

    vec3 L = normalize(lightPos - v_FragPos);
    vec3 H = normalize(V + L);

    float dist      = length(lightPos - v_FragPos);
    float att       = 1.0 / (dist * dist + 0.01);
    vec3  radiance  = lightColor * att;

    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3  F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 numerator   = D * G * F;
    float denom      = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular    = numerator / denom;

    vec3 Lo = (kD * albedo / PI + specular) * radiance * max(dot(N, L), 0.0);

    return Lo;
}

// ────────────────────────────────────────────────
// Функция: косвенный свет (IBL)
// ────────────────────────────────────────────────
vec3 computeIBL(
    vec3 N, vec3 V, vec3 R, float NdotV,
    vec3 albedo, float metallic, float roughness, float ao,
    vec3 F0
) {
    // Можно полностью отключить IBL вот здесь
    const bool ENABLE_IBL = true;               // ← основной переключатель
    if (!ENABLE_IBL) {
        return vec3(0.0);
    }

    vec3 F_ibl  = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD_ibl = (1.0 - F_ibl) * (1.0 - metallic);

    vec3 irradiance  = texture(u_IrradianceMap, N).rgb;
    vec3 diffuseIBL  = irradiance * albedo;

    float lod        = roughness * MAX_REFLECTION_LOD;
    vec3 prefiltered = textureLod(u_PrefilterMap, R, lod).rgb;

    vec2 brdf        = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y) * u_SpecularMul;

    vec3 ambient = (kD_ibl * diffuseIBL + specularIBL) * ao * u_StudioIblStrength;

    return ambient;
}

// ────────────────────────────────────────────────
// main
// ────────────────────────────────────────────────
void main() {
    // 1. Чтение материала
    vec4 albedoTex = texture(u_AlbedoMap, v_TexCoord);
    vec3 albedo    = pow(albedoTex.rgb, vec3(2.2));
    float alpha    = albedoTex.a * u_BaseColorFactor.a;

    vec3 orm = texture(u_ORMMap, v_TexCoord).rgb;
    float ao = mix(1.0, orm.r, u_OcclusionFactor * 0.8);

    float roughness = clamp(orm.g * u_RoughnessFactor, 0.04, 1.0);
    float metallic  = clamp(orm.b * u_MetallicFactor, 0.0, 1.0);

    vec3 normalMap = texture(u_NormalMap, v_TexCoord).rgb * 2.0 - 1.0;
    vec3 N = normalize(v_TBN * normalMap);

    vec3 V = normalize(u_CameraPos - v_FragPos);
    vec3 R = reflect(-V, N);
    float NdotV = max(dot(N, V), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // 2. Освещение
    vec3 direct = computeDirectLight(N, V, albedo, metallic, roughness, F0, u_LightPos, u_LightColor);
    vec3 ibl    = computeIBL(N, V, R, NdotV, albedo, metallic, roughness, ao, F0);

    // 3. Emission
    vec3 emission = albedo * u_Emission;

    // 4. Сумма
    vec3 color = direct + ibl + emission;

    // 5. Пост-обработка
    color *= u_Exposure;
    color = tonemapFilmic(color);
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}