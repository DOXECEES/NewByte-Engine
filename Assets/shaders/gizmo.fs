#version 430 core

in vec3 vColor;
out vec4 FragColor;

void main()
{
    // Просто выводим интерполированный цвет ребер или полигонов
    FragColor = vec4(vColor, 1.0);
}