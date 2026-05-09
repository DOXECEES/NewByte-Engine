#version 450 core
#extension GL_ARB_bindless_texture : enable

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gViewPos; 
layout (location = 2) out vec4 gColorMap; 


layout(bindless_sampler) uniform sampler2D u_AlbedoMap; 


in vec3 v_Normal;
in vec3 v_ViewPos;
in vec2 v_TexCoodrs;


void main()
{
    gNormal = vec4(normalize(v_Normal),1.0);
    gViewPos = vec4(v_ViewPos, 1.0);
    gColorMap = vec4(texture(u_AlbedoMap, v_TexCoodrs).rgb, 1.0);
}