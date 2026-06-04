#version 450 core
#extension GL_ARB_bindless_texture : require

out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D fboTexture;

uniform bool isGrayscale = false;
uniform bool isDepth = false;
uniform float nearPlane = 0.1;
uniform float farPlane = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Перевод из диапазона [0, 1] в NDC [-1, 1]
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
    vec4 color = texture(fboTexture, TexCoords);

	if (isDepth)
    {
        float depthVal = texture(fboTexture, TexCoords).r;

        float linearDepth = LinearizeDepth(depthVal);

        float visualDepth = linearDepth / farPlane;

        color = vec4(vec3(visualDepth), 1.0);
    }

    if(isGrayscale)
    {
        float luma = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        color.rgb = vec3(luma);
    }

    FragColor = color;
}