#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; // Переименовал для ясности
layout(location = 1) in vec3 aColor;  // Изменил локацию на 2
layout(location = 3) in vec2 aTexCoords;
layout(location = 4) in vec4 aTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 vColor;

void main() {
    vColor = aColor;
    // Умножаем матрицы как обычно
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}