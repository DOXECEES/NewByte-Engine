#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;

uniform vec2 screenSize; // Размер экрана

void main()
{
    // Получаем цвет из текстуры
    vec3 color = texture(depthMap, TexCoords).rgb;

    // Вычисляем координаты относительно центра экрана
    vec2 center = vec2(0.5, 0.5);
    vec2 uv = TexCoords;

    // Рассчитываем расстояние от центра
    float dist = length(uv - center);

    // Применяем эффект виньетки: чем дальше от центра, тем темнее пиксель
    float vignette = smoothstep(0.1, 0.1 + 0.1, dist);

    // Умножаем цвет на интенсивность виньетки
    color *= mix(vec3(1.0), vec3(1.0 - 0.1), vignette);

    FragColor = vec4(color, 1.0f);
}
