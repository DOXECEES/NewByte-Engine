#version 430 core

layout (location = 0) in vec3 aPos;       // Позиция из Vertex
layout (location = 1) in vec3 aNormal;    // Нормаль из Vertex
layout (location = 2) in vec3 aColor;     // Цвет Jolt из Vertex

out vec3 vColor;

uniform mat4 uViewProj;
uniform mat4 model;

void main()
{
    vColor = aColor;
    gl_Position = uViewProj * model * vec4(aPos, 1.0);
}