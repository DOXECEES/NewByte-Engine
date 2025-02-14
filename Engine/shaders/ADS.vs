#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 oNormal;
out vec3 FragPos;

void main()
{
	FragPos = vec3(view * model * vec4(aPos, 1.0));

	oNormal = mat3(transpose(inverse(model))) * Normal;
	gl_Position = proj  * vec4(FragPos, 1.0);
}