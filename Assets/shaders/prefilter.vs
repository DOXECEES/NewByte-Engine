#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // localPos - это и есть вектор направления из центра куба
    localPos = aPos;  
    
    // Обычная трансформация для отрисовки грани куба во фреймбуфер
    gl_Position = projection * view * vec4(aPos, 1.0);
}