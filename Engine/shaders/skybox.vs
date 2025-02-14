#version 330 core
layout (location = 0) in vec3 aPos;


out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{

    TexCoords = aPos;  

    // Оставляем только вращение и масштабирование для матрицы вида, игнорируем трансляцию
    mat4 viewNoTranslation = mat4(mat3(view)); // Убираем трансляцию из матрицы вида
    gl_Position = projection * viewNoTranslation * vec4(aPos, 1.0);
}
