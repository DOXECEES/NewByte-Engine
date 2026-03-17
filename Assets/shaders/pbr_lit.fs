#version 330 core

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_FragPos;
in mat3 v_TBN;

// Параметры из .material файла
uniform vec4 u_BaseColor;
uniform float u_Roughness;
uniform float u_Metallic;
uniform float u_Specular;
uniform float u_Emission;
uniform sampler2D u_NormalMap;

// Системные переменные (должны передаваться из Renderer)
uniform vec3 u_CameraPos;
uniform vec3 u_LightPos;   // Для теста один точечный свет
uniform vec3 u_LightColor;

const float PI = 3.14159265359;

// --- Функции PBR (Cook-Torrance) ---

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
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

void main() {
    // 1. Извлекаем нормаль из текстуры и переводим в World Space
    vec3 normal = texture(u_NormalMap, v_TexCoord).rgb;
    normal = normalize(v_TBN * (normal * 2.0 - 1.0));

    vec3 V = normalize(u_CameraPos - v_FragPos);
    vec3 N = normal;

    // F0 для диэлектриков (пластик) в среднем 0.04, для металлов берем базовый цвет
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, u_BaseColor.rgb, u_Metallic);

    // 2. Расчет освещения (Direct Lighting)
    vec3 L = normalize(u_LightPos - v_FragPos);
    vec3 H = normalize(V + L);
    float distance = length(u_LightPos - v_FragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = u_LightColor * attenuation;

    // Cook-Torrance BRDF
    float D = DistributionGGX(N, H, u_Roughness);
    float G = GeometrySmith(N, V, L, u_Roughness);
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - u_Metallic);

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * u_BaseColor.rgb / PI + specular) * radiance * NdotL;

    // 3. Ambient + Emission
    vec3 ambient = vec3(0.03) * u_BaseColor.rgb;
    vec3 emission = u_BaseColor.rgb * u_Emission;
    
    vec3 color = ambient + Lo + emission;

    // 4. HDR Tone Mapping & Gamma Correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, u_BaseColor.a);
}