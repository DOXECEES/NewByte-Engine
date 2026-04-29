#version 430 core
out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 0) uniform sampler2D u_ColorMap;
layout (binding = 1) uniform sampler2D u_NormalMap;
layout (binding = 2) uniform sampler2D u_DepthMap;

uniform mat4 u_Projection;
uniform mat4 u_InvProjection;
uniform vec2 u_ScreenSize;

// Функция восстановления позиции в View Space из глубины
vec3 getPos(vec2 uv) {
    float depth = texture(u_DepthMap, uv).r * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 view = u_InvProjection * clip;
    return view.xyz / view.w;
}

void main() {
    // Получаем данные из G-буфера
    vec3 color = texture(u_ColorMap, TexCoords).rgb;
    vec3 normal = texture(u_NormalMap, TexCoords).rgb; // Должны быть в View Space
    vec3 pos = getPos(TexCoords);

    // Если нормаль пустая (например, небо), не считаем отражения
    if (length(normal) < 0.1) {
        FragColor = vec4(color, 1.0);
        return;
    }

    // Направление взгляда и вектор отражения
    vec3 viewDir = normalize(pos);
    vec3 reflectDir = normalize(reflect(viewDir, normal));

    // Настройки Ray Marching
    float step = 0.2;       // Длина шага луча
    int maxSteps = 50;      // Макс. количество шагов
    float thickness = 0.2;  // "Толщина" объектов сцены
    
    vec3 ssrColor = vec3(0.0);
    vec3 currentPos = pos;
    bool hit = false;

    for(int i = 0; i < maxSteps; i++) {
        currentPos += reflectDir * step;

        // Проецируем текущую точку луча на экран, чтобы получить UV
        vec4 proj = u_Projection * vec4(currentPos, 1.0);
        vec2 sampleUV = (proj.xy / proj.w) * 0.5 + 0.5;

        // Проверка границ экрана
        if(sampleUV.x < 0 || sampleUV.x > 1 || sampleUV.y < 0 || sampleUV.y > 1) break;

        // Получаем глубину сцены в этой точке экрана
        float sceneDepth = getPos(sampleUV).z;

        // Проверяем: луч зашел за поверхность?
        if(currentPos.z < sceneDepth) {
            // Проверка на толщину, чтобы не отражать "задники"
            if(abs(currentPos.z - sceneDepth) < thickness) {
                // Плавное затухание у краев экрана
                float edgeFactor = min(1.0, (0.5 - abs(sampleUV.x - 0.5)) * 10.0) * 
                                   min(1.0, (0.5 - abs(sampleUV.y - 0.5)) * 10.0);
                
                ssrColor = texture(u_ColorMap, sampleUV).rgb * edgeFactor;
                hit = true;
                break;
            }
        }
    }

    // Смешиваем оригинальный цвет и отражение
    // Можно добавить коэффициент Френеля для реализма
    float fresnel = pow(1.0 - max(dot(normal, -viewDir), 0.0), 3.0);
    
    FragColor = vec4(color + ssrColor * fresnel * 0.5, 1.0);
}