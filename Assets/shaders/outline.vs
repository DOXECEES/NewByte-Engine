#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;   

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

uniform float u_OutlineWidth;          
uniform bool  u_UseScreenSpace = false;
uniform float u_ScreenSpaceWidth = 2.0; 
uniform vec2  u_ViewportSize;           

out vec4 vWorldPos;  

void main()
{
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    vec3 worldNormal = normalize(mat3(transpose(inverse(u_Model))) * aNormal);

    float offset = u_OutlineWidth;
    if (u_UseScreenSpace)
    {
        vec4 clipPos = u_Proj * u_View * worldPos;
        float distanceToCamera = abs(clipPos.w); 
        offset = u_ScreenSpaceWidth * distanceToCamera / u_ViewportSize.y;
    }

    vec3 offsetWorldPos = worldPos.xyz + worldNormal * offset;
    vec4 finalWorldPos = vec4(offsetWorldPos, 1.0);

    gl_Position = u_Proj * u_View * finalWorldPos;
    vWorldPos = finalWorldPos; 
}