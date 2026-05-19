#version 450 core

layout (location = 0) in vec3 aPos;       
layout (location = 3) in vec2 aTexCoord; 

out vec2 TexCoord;

uniform mat4 uView;      
uniform mat4 uProjection;
uniform vec3 uPosition; 

void main()
{
    vec3 cameraRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 cameraUp    = vec3(uView[0][1], uView[1][1], uView[2][1]);

    vec3 worldPos = uPosition 
                  + cameraRight * aPos.x 
                  + cameraUp * aPos.y;

    gl_Position = uProjection * uView * vec4(worldPos, 1.0);
    TexCoord = aTexCoord;
}