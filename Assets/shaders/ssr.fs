#version 430 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;
in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D u_ColorMap;
layout(bindless_sampler) uniform sampler2D u_NormalMap;
layout(bindless_sampler) uniform sampler2D u_DepthMap;

uniform mat4 projection;
uniform mat4 invProjection;

vec3 getPos(vec2 uv)
{
    float depth = texture(u_DepthMap, uv).r * 2.0 - 1.0;
    vec4 clip = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 view = invProjection * clip;
    return view.xyz / view.w;
}

vec2 projectToUV(vec3 viewPos)
{
    vec4 proj = projection * vec4(viewPos, 1.0);
    vec2 ndc = proj.xy / proj.w;
    return ndc * 0.5 + 0.5;
}

void main()
{
    vec3 color = texture(u_ColorMap, TexCoords).rgb;
    
    vec3 normal = normalize(texture(u_NormalMap, TexCoords).xyz);
    
    vec3 pos = getPos(TexCoords);
    vec3 viewDir = normalize(pos); 
    vec3 reflectDir = normalize(reflect(viewDir, normal));

    float stepSize = 0.2;
    int maxSteps = 80;
    int binarySearchSteps = 6;
    float thickness = 0.3; 

    vec3 currentPos = pos + reflectDir * 0.15;
    vec2 sampleUV = vec2(0.0);
    bool hit = false;

    for (int i = 0; i < maxSteps; i++)
    {
        currentPos += reflectDir * stepSize;
        sampleUV = projectToUV(currentPos);

        if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0)
            break;

        float sceneZ = getPos(sampleUV).z;
        float diff = currentPos.z - sceneZ;

        if (diff < 0.0 && diff > -thickness)
        {
            vec3 a = currentPos - reflectDir * stepSize;
            vec3 b = currentPos;

            for (int j = 0; j < binarySearchSteps; j++)
            {
                vec3 mid = mix(a, b, 0.5);
                vec2 midUV = projectToUV(mid);
                float midZ = getPos(midUV).z;
                if (mid.z < midZ) b = mid;
                else a = mid;
            }

            sampleUV = projectToUV(b);
            hit = true;
            break;
        }
    }

    vec3 ssrColor = vec3(0.0);
    float fade = 0.0;

    if (hit)
    {
        vec2 edgeFade = smoothstep(0.0, 0.1, sampleUV) * (1.0 - smoothstep(0.9, 1.0, sampleUV));
        fade = edgeFade.x * edgeFade.y;

        float distFade = 1.0 - clamp(length(currentPos - pos) / 25.0, 0.0, 1.0);
        
        float fresnel = pow(1.0 - max(dot(normal, -viewDir), 0.0), 5.0);
        fresnel = clamp(fresnel, 0.1, 1.0);

        ssrColor = texture(u_ColorMap, sampleUV).rgb * fade * distFade * fresnel;
    }

    FragColor = vec4(color + ssrColor, 1.0);
}