#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture; // текстура для отображения

void main()
{
    FragColor = texture(sceneTexture, TexCoords);
}
