#version 450 core

out vec2 TexCoords;

void main() 
{
    // Генерируем UV-координаты:
    // id 0 -> (0, 0)
    // id 1 -> (2, 0)
    // id 2 -> (0, 2)
    TexCoords = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);

    // Переводим UV в координаты NDC (Normalized Device Coordinates) [-1, 1]:
    // (0,0) -> (-1, -1)
    // (2,0) -> ( 3, -1)
    // (0,2) -> (-1,  3)
    gl_Position = vec4(TexCoords * 2.0 - 1.0, 0.0, 1.0);
}