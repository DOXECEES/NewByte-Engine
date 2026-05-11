#version 450 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler) uniform sampler2D gFinalImage;       
layout(bindless_sampler) uniform sampler2D gPosition;         
layout(bindless_sampler) uniform sampler2D gNormal;           
layout(bindless_sampler) uniform sampler2D gExtraComponents;  // R: Metallic, G: Roughness

uniform mat4 projection;
uniform mat4 invView;

noperspective in vec2 TexCoords;
out vec4 outColor;

// SSR params
const float rayStep = 0.25;          
const int maxSteps = 80;             
const int binarySearchSteps = 6;
const float maxThickness = 0.4;      
const float maxDistance = 50.0;      

#define Scale vec3(.8, .8, .8)
#define K 19.19

vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx) * a.zyx);
}

vec2 projectToUV(vec3 viewPos)
{
    vec4 clip = projection * vec4(viewPos, 1.0);
    clip.xyz /= clip.w;
    return clip.xy * 0.5 + 0.5;
}

vec3 binarySearch(vec3 rayDir, vec3 hitPos)
{
    float stepSize = rayStep;

    for (int i = 0; i < binarySearchSteps; i++)
    {
        stepSize *= 0.5;
        vec2 uv = projectToUV(hitPos);

        float sceneZ = texture(gPosition, uv).z;
        float diff = hitPos.z - sceneZ;

        // Мы хотим diff -> 0
        if (diff > 0.0)
        {
            hitPos -= rayDir * stepSize;
        }
        else
        {
            hitPos += rayDir * stepSize;
        }
    }

    return vec3(projectToUV(hitPos), hitPos.z);
}

vec4 rayMarch(vec3 rayDir, vec3 startPos)
{
    vec3 hitPos = startPos;

    for (int i = 0; i < maxSteps; i++)
    {
        hitPos += rayDir * rayStep;

        float dist = length(hitPos - startPos);
        if (dist > maxDistance)
        {
            return vec4(0.0);
        }

        vec2 uv = projectToUV(hitPos);

        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        {
            return vec4(0.0);
        }

        float sceneZ = texture(gPosition, uv).z;

        // View-space Z обычно отрицательный.
        // Если hitPos.z > sceneZ значит луч ближе к камере, чем поверхность.
        float diff = hitPos.z - sceneZ;

        // Попали "внутрь" поверхности (перешли глубину)
        if (diff > 0.0 && diff < maxThickness)
        {
            vec3 refined = binarySearch(rayDir, hitPos);
            return vec4(refined.xy, dist, 1.0);
        }
    }

    return vec4(0.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 albedo = texture(gFinalImage, TexCoords).rgb;

    outColor = vec4(albedo, 1.0);
    return;
    vec2 extra = texture(gExtraComponents, TexCoords).rg;
    float metallic = extra.r;
    float roughness = extra.g;

    vec3 viewPos = texture(gPosition, TexCoords).xyz;
    vec3 viewNormal = normalize(texture(gNormal, TexCoords).xyz);


    // Если пиксель пустой (небо или фон)
    if (length(viewPos) < 0.0001)
    {
        outColor = vec4(0, 0, 0, 1);
        return;
    }

    // SSR только для металлов (иначе грязь на пластике)
    if (metallic < 0.02)
    {
        outColor = vec4(0, 0, 0, 1);
        return;
    }

    // Вектор взгляда: от пикселя к камере (камера в (0,0,0) в view space)
    vec3 V = normalize(-viewPos);

    // Правильное отражение
    vec3 R = normalize(reflect(-V, viewNormal));

    // Jitter зависит от roughness
    vec3 worldPos = (invView * vec4(viewPos, 1.0)).xyz;
    vec3 noise = hash(worldPos) * 2.0 - 1.0;

    float jitterStrength = roughness * 0.25;
    R = normalize(R + noise * jitterStrength);

    // Отсекаем отражения, которые идут в камеру или почти параллельно экрану
    if (R.z > -0.05)
    {
        outColor = vec4(0, 0, 0, 1);
        return;
    }

    // Запускаем трассировку
    vec4 hit = rayMarch(R, viewPos);

    if (hit.w < 0.5)
    {
        outColor = vec4(0, 0, 0, 1);
        return;
    }

    vec2 hitUV = hit.xy;
    float hitDist = hit.z;

    // Fade по краям экрана
    float edgeFade = 1.0;
    edgeFade *= smoothstep(0.0, 0.15, hitUV.x);
    edgeFade *= smoothstep(0.0, 0.15, hitUV.y);
    edgeFade *= smoothstep(0.0, 0.15, 1.0 - hitUV.x);
    edgeFade *= smoothstep(0.0, 0.15, 1.0 - hitUV.y);

    // Fade по расстоянию
    float distFade = 1.0 - clamp(hitDist / maxDistance, 0.0, 1.0);

    // Fresnel
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlick(clamp(dot(viewNormal, V), 0.0, 1.0), F0);

    // LOD размытие по roughness
    float lod = roughness * 7.0;
    vec3 reflectedColor = textureLod(gFinalImage, hitUV, 0.0).rgb;

    float strength = metallic * edgeFade * distFade;

    vec3 result = reflectedColor * F * strength;

    outColor = vec4(result, 1.0);
}