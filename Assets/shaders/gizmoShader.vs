#version 450 core

// Соответствует твоей структуре nb::Renderer::Vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor; // RGB цвет из вершины
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec4 aTangent;

// Передаем только View * Projection (модель не нужна, вершины уже в мировых координатах)
uniform mat4 uViewProj;

out vec3 vColor;

void main()
{
    vColor = aColor;
    
    // TinyGizmo генерирует вершины сразу в мировом пространстве, 
    // поэтому мы просто умножаем на матрицы камеры.
    gl_Position = uViewProj * vec4(aPos, 1.0);
}