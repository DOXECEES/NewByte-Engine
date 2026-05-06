#version 450 core
#extension GL_ARB_bindless_texture : require

layout (location = 0) out vec4 FragColor;
in vec2 TexCoords;

// Bindless семплеры объявляются как обычные uniform
layout(bindless_sampler) uniform sampler2D gPosition;
layout(bindless_sampler) uniform sampler2D gNormal;
layout(bindless_sampler) uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;
uniform vec2 noiseScale;

uniform float radius = 0.5;
uniform float bias = 0.025;

void main() {
    // Использование абсолютно такое же, как с обычными текстурами
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal  = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < 64; ++i) {
        vec3 samplePos = TBN * samples[i]; 
        samplePos = fragPos + samplePos * radius; 
        
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        float sampleDepth = texture(gPosition, offset.xy).z;
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    
    FragColor = vec4(1.0 - (occlusion / 64.0), 0.0, 0.0, 1.0);
    //FragColor = vec4(0.12, 0.0,0.0, 1.0);
}