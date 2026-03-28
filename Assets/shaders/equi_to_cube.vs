#version 330 core

layout (location = 0) in vec3 aPos; // Позиция вершины куба (от -1 до 1)

out vec3 localPos; // Передаем во фрагментный шейдер

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // Мы используем позицию вершины как направление вектора из центра куба.
    // Это именно то, что нужно фрагментному шейдеру для функции atan2/asin.
    localPos = aPos;

    // Умножаем позицию на матрицы. 
    // Поскольку мы рендерим "внутри" куба, порядок стандартный.
    gl_Position = projection * view * vec4(aPos, 1.0);
}