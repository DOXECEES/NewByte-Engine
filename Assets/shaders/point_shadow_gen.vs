#version 450 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 FragPos;
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}