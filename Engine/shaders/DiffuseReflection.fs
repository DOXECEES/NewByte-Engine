#version 330 core

out vec4 FragPos;

in vec3 oNormal;

uniform vec3 lightPos;
uniform vec3 Kd;
uniform vec3 Ld;

void main()
{
	vec3 result = Kd * Ld * max(dot(oNormal, lightPos), 0.0);
	FragPos = vec4(result, 1.0);
} 
