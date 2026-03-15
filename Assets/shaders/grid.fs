#version 330 core
out vec4 FragColor;

void main() {
    // Размер клетки 16 пикселей
    float size = 16.0;
    
    // Рассчитываем координаты клетки
    // floor(пиксель / 16) даст номер клетки.
    // mod(x + y, 2.0) даст 0 или 1 в шахматном порядке.
    vec2 gridPos = floor(gl_FragCoord.xy / size);
    float checker = mod(gridPos.x + gridPos.y, 2.0);
    
    // Цвета: светло-серый (0.8) и чуть темнее (0.7)
    vec3 color = mix(vec3(0.7), vec3(0.8), checker);
    
    FragColor = vec4(color, 1.0);
}