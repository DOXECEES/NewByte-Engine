#version 450 core

in vec3 vColor;
out vec4 FragColor;

void main()
{
    // Выводим цвет из вершины. Альфа = 1.0 (полная непрозрачность)
    FragColor = vec4(vColor, 1.0);
}