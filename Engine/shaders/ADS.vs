#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 aTexCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 oNormal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
	FragPos = vec3(view * model * vec4(aPos, 1.0));
	TexCoords = aTexCoords;
	oNormal = mat3(transpose(inverse(model))) * Normal;
	gl_Position = proj  * vec4(FragPos, 1.0);
}