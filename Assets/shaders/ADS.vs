#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 Normal;
layout(location = 1) in vec3 Color;
layout(location = 3) in vec2 aTexCoords;
layout(location = 4) in vec4 aTangent;


uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 lightSpaceMatrix;


out vec3 oNormal;
out vec3 FragPos;
out vec2 TexCoords;
out vec3 oModelFragPos;
out vec4 FragPosLightSpace;

out mat3 TBN;

void main()
{
	
	TexCoords = aTexCoords;
	mat3 normalMatrix = mat3(transpose(inverse(model)));
	oNormal =  normalMatrix * Normal;
	
	vec3 T = normalize(normalMatrix * aTangent.xyz);
    vec3 N = normalize(normalMatrix * Normal);
    vec3 B = cross(N, T);
    
    // Матрица TBN
    TBN = mat3(T, B, N);
	oModelFragPos = vec3(model * vec4(aPos, 1.0));
	
	FragPos = vec3(view * model * vec4(aPos, 1.0));
	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

	gl_Position = proj  * vec4(FragPos, 1.0);
}