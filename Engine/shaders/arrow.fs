#version 330 core

out vec4 FragColor;
in vec3 gsColor;

void main() 
{    
    FragColor = vec4(gsColor, 1.0); 
}
