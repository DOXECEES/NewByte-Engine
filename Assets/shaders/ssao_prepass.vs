#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoords;


out vec3 v_Normal;
out vec3 v_ViewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Позиция в пространстве вида
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    v_ViewPos = viewPos.xyz;

    // Нормаль в пространстве вида (используем Normal Matrix)
    // transpose(inverse(mat3(view * model)))
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    v_Normal = normalize(normalMatrix * aNormal);

    gl_Position = projection * viewPos;
}