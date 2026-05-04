#version 450 core
#extension GL_ARB_bindless_texture : require

layout (location = 0) out vec4 FragColor;
in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D ssaoInput;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            result += texture(ssaoInput, TexCoords + vec2(x, y) * texelSize).r;
        }
    }
    FragColor = vec4(result / 16.0, 0.0,0.0, 1.0);
    //FragColor = vec4(texture(ssaoInput, TexCoords).rgb, 1.0);
}