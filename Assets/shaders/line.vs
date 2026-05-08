#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOther;
layout (location = 2) in vec3 aColor;
layout (location = 3) in float aSide; 

uniform mat4 u_View;
uniform mat4 u_Proj;
uniform float u_Thickness; 
uniform vec2 u_ViewportSize; 

out vec3 vColor;

void main() {
    vColor = aColor;
    
    mat4 viewProj = u_Proj * u_View;
    vec4 clipPos = viewProj * vec4(aPos, 1.0);
    vec4 clipOther = viewProj * vec4(aOther, 1.0);

    if (clipPos.w <= 0.0) {
        float t = (0.001 - clipPos.w) / (clipOther.w - clipPos.w);
        clipPos = mix(clipPos, clipOther, t);
    }
    if (clipOther.w <= 0.0) {
        float t = (0.001 - clipOther.w) / (clipPos.w - clipOther.w);
        clipOther = mix(clipOther, clipPos, t);
    }

    vec2 screenPos = (clipPos.xy / clipPos.w) * u_ViewportSize;
    vec2 screenOther = (clipOther.xy / clipOther.w) * u_ViewportSize;
    
    vec2 dir = screenOther - screenPos;
    if (length(dir) < 0.001) {
        dir = vec2(1.0, 0.0);
    } else {
        dir = normalize(dir);
    }
    
    vec2 normal = vec2(-dir.y, dir.x);
    
    vec2 offset = normal * aSide * u_Thickness * 0.5;
    
    vec2 finalScreenPos = screenPos + offset;
    gl_Position = vec4((finalScreenPos / u_ViewportSize) * clipPos.w, clipPos.z, clipPos.w);

    gl_Position.z -= 0.0001; 
}