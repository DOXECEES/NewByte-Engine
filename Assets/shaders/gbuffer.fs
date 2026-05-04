#version 450 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
//layout (location = 3) out vec4 gARM_Emission; // AO, Roughness, Metallic, Emission Strength

in vec3 FragPos;
in vec2 v_TexCoords;
in mat3 TBN;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_ORMMap;
uniform sampler2D u_EmissionMap;
uniform float     u_NormalMapStrength = 1.0;
uniform float     u_EmissionMapStrength = 1.0;

void main() {    
    gPosition = FragPos;

    vec3 nMap = texture(u_NormalMap, v_TexCoords).rgb * 2.0 - 1.0;
    nMap.xy *= u_NormalMapStrength;
    gNormal = normalize(TBN * nMap);

    gAlbedoSpec.rgb = pow(texture(u_AlbedoMap, v_TexCoords).rgb, vec3(2.2));
    
    vec3 orm = texture(u_ORMMap, v_TexCoords).rgb;
    float emission = texture(u_EmissionMap, v_TexCoords).r * u_EmissionMapStrength;
    //gARM_Emission = vec4(orm, emission);
}