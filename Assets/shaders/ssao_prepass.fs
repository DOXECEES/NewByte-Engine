#version 450 core

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gViewPos; // Опционально, если не хотите восстанавливать позицию из глубины

in vec3 v_Normal;
in vec3 v_ViewPos;

void main()
{
    // Сохраняем нормали в View Space
    // Если текстура формата RGB16F или RGB32F, значения [-1, 1] сохраняются корректно
    gNormal = vec4(normalize(v_Normal),1.0);
    // Сохраняем позицию в View Space (если используете 2-й attachment)
    gViewPos = vec4(v_ViewPos, 1.0);
}