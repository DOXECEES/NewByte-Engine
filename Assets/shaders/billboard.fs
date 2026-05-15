#version 450 core
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord;
out vec4 FragColor;

layout(bindless_sampler) uniform sampler2D uTexture; 

void main()
{
    vec4 texColor = texture(uTexture, TexCoord);
    
    if (texColor.a < 0.1)
        discard;
    
    FragColor = texColor;
}