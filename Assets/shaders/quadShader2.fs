#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture; // текстура для отображения

void main()
{
    float depthValue = texture(sceneTexture, TexCoords).r;
    // Выводим как ч/б: чем ближе объект, тем темнее, чем дальше — тем белее
    FragColor = vec4(vec3(depthValue), 1.0);

    //FragColor = texture(sceneTexture, TexCoords);
}
