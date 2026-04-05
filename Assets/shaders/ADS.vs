#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoords;
layout(location = 4) in vec4 aTangent; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 lightView;
uniform mat4 lightProj;

out vec2 v_TexCoords;
out vec3 FragPos;
out vec4 FragPosLightSpace;
out mat3 TBN;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    v_TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent.xyz);
    
    T = normalize(T - dot(T, N) * N);
    
    vec3 B = cross(N, T) * (aTangent.w < 0.0 ? -1.0 : 1.0);
    
    TBN = mat3(T, B, N);

    FragPosLightSpace = (lightProj * lightView) * worldPos;

    gl_Position = proj * view * worldPos;
}