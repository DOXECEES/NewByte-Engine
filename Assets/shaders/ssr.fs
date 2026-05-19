#version 450 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler) uniform sampler2D gFinalImage;       
layout(bindless_sampler) uniform sampler2D gPosition;         
layout(bindless_sampler) uniform sampler2D gNormal;           
layout(bindless_sampler) uniform sampler2D gExtraComponents; 

uniform mat4 projection;
noperspective in vec2 TexCoords;
out vec4 outColor;

// --- ПАРАМЕТРЫ ДЛЯ ИДЕАЛЬНОГО ПРИЛЕГАНИЯ ---
const float rayStep = 0.04;           // Маленький шаг для точности у основания
const int maxSteps = 250;            
const int binarySearchSteps = 16;     // Высокая точность границ
const float rayBias = 0.008;          // МИНИМАЛЬНЫЙ отступ (убирает разрыв)
const float normalBias = 0.01;        // МИНИМАЛЬНОЕ поднятие
const float thickness = 0.15;         

vec2 projectToUV(vec3 viewPos) {
    vec4 clip = projection * vec4(viewPos, 1.0);
    clip.xyz /= clip.w;
    return clip.xy * 0.5 + 0.5;
}

vec3 binarySearch(vec3 rayDir, vec3 hitPos) {
    float step = rayStep;
    for (int i = 0; i < binarySearchSteps; i++) {
        step *= 0.5;
        vec2 uv = projectToUV(hitPos);
        float sceneZ = texture(gPosition, uv).z;
        if (sceneZ > hitPos.z) hitPos -= rayDir * step;
        else                  hitPos += rayDir * step;
    }
    return hitPos;
}

vec4 rayMarch(vec3 rayDir, vec3 startPos, vec3 startNormal) {
    // Начинаем максимально близко к точке поверхности
    vec3 currentPos = startPos + (startNormal * normalBias) + rayDir * rayBias;
    
    for (int i = 0; i < maxSteps; i++) {
        currentPos += rayDir * rayStep;
        vec2 uv = projectToUV(currentPos);
        
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) return vec4(0.0);
        
        float sceneZ = texture(gPosition, uv).z;
        if (sceneZ > -0.01) continue;
        
        float diff = sceneZ - currentPos.z;
        
        if (diff > 0.0 && diff < thickness) {
            // Проверка, что мы не попали в "заднюю стенку" объекта
            vec3 hitNormal = texture(gNormal, uv).xyz;
            if (dot(hitNormal, rayDir) > 0.0) continue; 

            vec3 refinedPos = binarySearch(rayDir, currentPos);
            return vec4(projectToUV(refinedPos), length(refinedPos - startPos), 1.0);
        }
    }
    return vec4(0.0);
}

void main() {
    vec3 viewPos = texture(gPosition, TexCoords).xyz;
    if (viewPos.z > -0.01) { outColor = vec4(0,0,0,1); return; }
    
    vec3 normal = normalize(texture(gNormal, TexCoords).xyz);
    vec2 extra = texture(gExtraComponents, TexCoords).rg;
    float roughness = extra.g;
    float metallic = extra.r;
    
    vec3 V = normalize(-viewPos);
    vec3 R = reflect(-V, normal);
    if (R.z > -0.01) { outColor = vec4(0,0,0,1); return; }

    vec4 hit = rayMarch(R, viewPos, normal);
    if (hit.w < 0.5) { outColor = vec4(0,0,0,1); return; }

    // ЧЕТКОСТЬ: Mip 0 для зеркал
    float mipLevel = (roughness < 0.03) ? 0.0 : roughness * 5.0; 
    vec3 reflectedColor = textureLod(gFinalImage, hit.xy, mipLevel).rgb;

    // ЗАТУХАНИЯ: Чтобы края были мягче, но центр четким
    float edgeFade = smoothstep(0.0, 0.1, hit.x) * smoothstep(0.0, 0.1, 1.0 - hit.x) *
                     smoothstep(0.0, 0.1, hit.y) * smoothstep(0.0, 0.1, 1.0 - hit.y);
    
    // Плавный контакт у основания (уменьшили зону тени)
    float contactFade = smoothstep(0.0, 0.03, hit.z);

    vec3 albedo = texture(gFinalImage, TexCoords).rgb;
    vec3 F0 = mix(vec3(0.08), albedo, metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - max(dot(normal, V), 0.0), 5.0);

    outColor = vec4(reflectedColor * F * edgeFade * contactFade * 1.5, 1.0);
}