#version 330 core
out vec4 FragColor;

uniform vec3 u_Color;
uniform float u_Alpha = 1.0; 

void main()
{
    FragColor = vec4(u_Color, u_Alpha);
}