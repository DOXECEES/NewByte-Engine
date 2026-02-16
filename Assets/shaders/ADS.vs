#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; // Переименовал для ясности
layout(location = 1) in vec3 aColor;  // Изменил локацию на 2
layout(location = 3) in vec2 aTexCoords;
layout(location = 4) in vec4 aTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 lightSpaceMatrix;

out vec3 oNormal;
out vec3 FragPos;      // Мы будем передавать это в World Space
out vec2 TexCoords;
out vec3 oModelFragPos; // Это тоже World Space (можно объединить с FragPos)
out vec4 FragPosLightSpace;
out mat3 TBN;

void main()
{
    // 1. Координаты в мировом пространстве (World Space)
    vec4 worldPos = model * vec4(aPos, 1.0);
    oModelFragPos = worldPos.xyz;
    FragPos = worldPos.xyz; // Для освещения в фрагментном шейдере лучше использовать World Space
    
    TexCoords = aTexCoords;

    // 2. Матрица нормалей (World Space)
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    oNormal = normalize(normalMatrix * aNormal);
    
    // 3. Расчет TBN для Normal Mapping
    vec3 T = normalize(normalMatrix * aTangent.xyz);
    vec3 N = oNormal;
    vec3 B = cross(N, T) * aTangent.w;
    TBN = mat3(T, B, N);

    // 4. ТЕНИ: Умножаем матрицу света на МИРОВЫЕ координаты
    // КРИТИЧНО: Используем worldPos, а не FragPos (который в View Space)
    FragPosLightSpace = lightSpaceMatrix * worldPos;

    // 5. Позиция на экране (Clip Space)
    gl_Position = proj * view * worldPos;
}