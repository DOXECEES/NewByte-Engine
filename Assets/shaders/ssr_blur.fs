#version 450 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler) uniform sampler2D u_SSRTexture;      
layout(bindless_sampler) uniform sampler2D gPosition;         
layout(bindless_sampler) uniform sampler2D gNormal;           
layout(bindless_sampler) uniform sampler2D gExtraComponents;

noperspective in vec2 TexCoords;
out vec4 outColor;

uniform vec2 u_Direction;  
uniform vec2 u_ScreenSize; 

const float weights[5] = float[](0.227, 0.194, 0.121, 0.054, 0.016);

void main() {
    float roughness = texture(gExtraComponents, TexCoords).g;
    
    // ИДЕАЛЬНАЯ ЧЕТКОСТЬ: Если пол зеркальный, блюр НЕ НУЖЕН
    if (roughness < 0.03) {
        outColor = texture(u_SSRTexture, TexCoords);
        return;
    }

    vec3 centerNormal = texture(gNormal, TexCoords).xyz;
    float centerDepth = texture(gPosition, TexCoords).z;

    float blurScale = roughness * 4.0; 
    vec2 texelSize = 1.0 / u_ScreenSize;
    
    vec3 result = texture(u_SSRTexture, TexCoords).rgb * weights[0];
    float totalWeight = weights[0];

    for(int i = 1; i < 5; ++i) {
        vec2 offset = u_Direction * texelSize * i * blurScale;
        vec2 uvs[2] = { TexCoords + offset, TexCoords - offset };
        
        for(int j = 0; j < 2; j++) {
            vec3 n = texture(gNormal, uvs[j]).xyz;
            float d = texture(gPosition, uvs[j]).z;

            // Билатеральный вес (сохранение краев)
            float weight = weights[i] 
                * pow(max(0.0, dot(centerNormal, n)), 32.0) 
                * exp(-abs(centerDepth - d) * 2.0);

            result += texture(u_SSRTexture, uvs[j]).rgb * weight;
            totalWeight += weight;
        }
    }
    outColor = vec4(result / max(totalWeight, 0.001), 1.0);
}