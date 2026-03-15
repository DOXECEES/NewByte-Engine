#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    // Просто пробрасываем текстурные координаты
    TexCoords = aTexCoords;
    // Рисуем полноэкранный квадрат
    gl_Position = vec4(aPos.xy * 0.25, 0.0 1.0);
}