#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    // Рисуем чистый цвет. 
    // Альфа-канал 1.0 важен, чтобы объект был непрозрачным внутри FBO
    FragColor = vec4(1.0);
}