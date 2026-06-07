#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoords;
layout(location = 4) in vec4 aTangent; 

// Новые атрибуты для скелетной анимации
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 lightView;
uniform mat4 lightProj;

// Юниформы скелетной анимации
const int MAX_BONES = 200;
uniform mat4 gBones[MAX_BONES];
uniform bool u_UseSkinning = false;

out vec2 v_TexCoords;
out vec3 FragPos;
out vec4 FragPosLightSpace;
out mat3 TBN;

void main()
{
    vec4 localPos     = vec4(aPos, 1.0);
    vec3 localNormal  = aNormal;
    vec3 localTangent = aTangent.xyz;

    // Вычисляем деформацию, если включена анимация
    if (u_UseSkinning)
    {
        mat4 boneTransform = mat4(0.0);
        bool hasInfluence  = false;

        for (int i = 0; i < 4; i++)
        {
            int boneID = aBoneIDs[i];
            if (boneID >= 0 && boneID < MAX_BONES)
            {
                boneTransform += gBones[boneID] * aWeights[i];
                hasInfluence = true;
            }
        }

        if (hasInfluence)
        {
            localPos     = boneTransform * vec4(aPos, 1.0);
            localNormal  = mat3(boneTransform) * aNormal;
            localTangent = mat3(boneTransform) * aTangent.xyz;
        }
    }

    // Расчет позиций на основе деформированных координат
    vec4 worldPos = model * localPos;
    FragPos = worldPos.xyz;
    v_TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    
    // Используем трансформированные костями нормали и тангенты
    vec3 N = normalize(normalMatrix * localNormal);
    vec3 T = normalize(normalMatrix * localTangent);
    
    T = normalize(T - dot(T, N) * N);
    
    // Сохраняем вашу оригинальную проверку знака направления
    vec3 B = cross(N, T) * (aTangent.w < 0.0 ? -1.0 : 1.0);
    
    TBN = mat3(T, B, N);

    FragPosLightSpace = (lightProj * lightView) * worldPos;

    gl_Position = proj * view * worldPos;
}