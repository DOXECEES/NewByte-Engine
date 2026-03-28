#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform vec3 channelMask;
uniform float gamma;
uniform float exposure;

void main()
{
    vec4 color = texture(sceneTexture, TexCoords);
    color.rgb = color.rgb * channelMask;
    color.rgb = color.rgb * exposure;
    color.rgb = pow(color.rgb, vec3(1.0 / gamma));

    FragColor = color;
}
