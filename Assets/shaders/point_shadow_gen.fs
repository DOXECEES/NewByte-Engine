#version 450 core
in vec3 FragPos;
uniform vec3 u_LightPos;
uniform float u_FarPlane;
out vec4 FragColor; // Рендерим в GL_R32F
void main() {
    float lightDistance = length(FragPos - u_LightPos);
    FragColor = vec4(lightDistance / u_FarPlane, 0.0, 0.0, 1.0); // Линейная дистанция 0..1
}