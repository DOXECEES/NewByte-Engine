#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal; // Переименовал для ясности
layout(location = 2) in vec3 a_Color;  // Изменил локацию на 2
layout(location = 3) in vec2 a_TexCoord;
layout(location = 4) in vec4 a_Tangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out mat3 v_TBN;

void main() {
    v_TexCoord = a_TexCoord;
    v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    
	mat3 normalMatrix = mat3(transpose(inverse(u_Model)));
    vec3 oNormal = normalize(normalMatrix * a_Normal);
	
    vec3 T = normalize(normalMatrix * a_Tangent.xyz);
    vec3 N = oNormal;
    vec3 B = cross(N, T) * a_Tangent.w;
    v_TBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}