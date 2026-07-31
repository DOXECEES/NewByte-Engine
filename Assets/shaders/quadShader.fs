#version 450 core
#extension GL_ARB_bindless_texture : enable

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D depthMap; 
uniform vec2 screenSize;   
layout(bindless_sampler) uniform sampler2D u_SSRTexture; 
layout(bindless_sampler) uniform sampler2D u_DepthMap; 

layout(bindless_sampler) uniform sampler2D u_OutlineMask;
uniform vec3 u_OutlineColor;
uniform int u_OutlineThickness; 
layout(bindless_sampler) uniform sampler2D lookupTableTexture;
uniform bool u_UseLut = true;


const int DOF_SAMPLES = 16;            
const float GOLDEN_ANGLE = 2.39996323; 

const float u_Exposure = 1.0; 

#ifdef USE_FXAA
    #define FXAA_REDUCE_MIN   (1.0/128.0)
    #define FXAA_REDUCE_MUL   (1.0/8.0)
    #define FXAA_SPAN_MAX     8.0
#endif

#ifdef USE_DOF

    layout(std140, binding = 0) uniform DofParams {
        float near;
        float far;
        float focusDistance; 
        float focusRange;  
    } u_Dof;

    
#endif


vec3 tonemap(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 getSceneWithOutline(vec2 coords) {
    vec3 baseColor = texture(depthMap, coords).rgb;
    vec3 reflection = texture(u_SSRTexture, coords).rgb;
    
    // Суммируем HDR данные
    vec3 colorWithSSR = baseColor + reflection;
    
    // Логика обводки (Outline)
    float mask = texture(u_OutlineMask, coords).r;
    float outlineAlpha = 0.0;
    if (mask < 0.5) {
        vec2 texelSize = 1.0 / screenSize;
        for (int x = -u_OutlineThickness; x <= u_OutlineThickness; x++) {
            for (int y = -u_OutlineThickness; y <= u_OutlineThickness; y++) {
                if (x*x + y*y <= u_OutlineThickness * u_OutlineThickness) {
                    vec2 sampleCoord = coords + vec2(x, y) * texelSize;

                    if (sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 && 
                        sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) 
                    {
                        float neighborMask = texture(u_OutlineMask, sampleCoord).r;
                        outlineAlpha = max(outlineAlpha, neighborMask);
                    }
                }
            }
        }
    }
    return mix(colorWithSSR, u_OutlineColor, outlineAlpha * 0.8);
}

vec3 applyLut(vec3 inColor) {
    vec3 color = clamp(inColor, 0.0, 1.0);
    float size = 16.0;
    float blueValue = color.b * (size - 1.0);
    float index1 = floor(blueValue);
    float index2 = ceil(blueValue);
    float v = (color.g * (size - 1.0) + 0.5) / size;
    float u1 = (index1 * size + color.r * (size - 1.0) + 0.5) / (size * size);
    float u2 = (index2 * size + color.r * (size - 1.0) + 0.5) / (size * size);
    vec3 col1 = textureLod(lookupTableTexture, vec2(u1, v), 0.0).rgb;
    vec3 col2 = textureLod(lookupTableTexture, vec2(u2, v), 0.0).rgb;
    return mix(col1, col2, fract(blueValue));
}

vec3 applyDOF(vec2 uv, float coc)
{
    vec3 colorAccum = vec3(0.0);
    float weightAccum = 0.0;
    vec2 maxBlurRadius = vec2(12.0) / screenSize; 
    
    for (int i = 0; i < DOF_SAMPLES; i++)
    {
        float r = sqrt(float(i) / float(DOF_SAMPLES));
        float theta = float(i) * GOLDEN_ANGLE;
        vec2 offset = vec2(cos(theta), sin(theta)) * r * coc * maxBlurRadius;
        
        vec2 sampleUV = clamp(uv + offset, vec2(0.0), vec2(1.0));
        
        colorAccum += getSceneWithOutline(sampleUV);
        weightAccum += 1.0;
    }
    return colorAccum / weightAccum;
}




void main() {
    vec3 finalColor;

#ifdef USE_FXAA
    vec2 inverseScreenSize = vec2(1.0) / screenSize;
    vec3 rgbNW = getSceneWithOutline(TexCoords + (vec2(-1.0, -1.0) * inverseScreenSize));
    vec3 rgbNE = getSceneWithOutline(TexCoords + (vec2(1.0, -1.0) * inverseScreenSize));
    vec3 rgbSW = getSceneWithOutline(TexCoords + (vec2(-1.0, 1.0) * inverseScreenSize));
    vec3 rgbSE = getSceneWithOutline(TexCoords + (vec2(1.0, 1.0) * inverseScreenSize));
    vec3 rgbM  = getSceneWithOutline(TexCoords);

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), 
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), 
          dir * rcpDirMin)) * inverseScreenSize;

    vec3 rgbA = 0.5 * (
        getSceneWithOutline(TexCoords + dir * (1.0/3.0 - 0.5)) +
        getSceneWithOutline(TexCoords + dir * (2.0/3.0 - 0.5)));
    
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        getSceneWithOutline(TexCoords + dir * (0.0/3.0 - 0.5)) +
        getSceneWithOutline(TexCoords + dir * (3.0/3.0 - 0.5)));

    float lumaB = dot(rgbB, luma);

    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        finalColor = rgbA;
    } else {
        finalColor = rgbB;
    }
#else
    finalColor = getSceneWithOutline(TexCoords);
#endif

#ifdef USE_DOF

    float depth = texture(u_DepthMap, TexCoords).r;
    float depthVal = depth * 2.0 - 1.0; 
    float linearDepth = (2.0 * u_Dof.near * u_Dof.far) / (u_Dof.far + u_Dof.near - depthVal * (u_Dof.far - u_Dof.near));
    
    float coc = clamp(abs(linearDepth - u_Dof.focusDistance) / u_Dof.focusRange, 0.0, 1.0);

    if (coc >= 0.1) {
        finalColor = applyDOF(TexCoords, coc); 
    }

#endif

    

    // 1. Применяем экспозицию (умножение HDR данных)
    finalColor *= u_Exposure;

    // 2. Сжимаем HDR в LDR диапазон (ACES). Теперь пересветы исчезнут.
    //finalColor = tonemap(finalColor);

    // 3. Гамма-коррекция. Делает переходы плавными и возвращает детали из теней.
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    // 4. Применяем LUT уже к исправленному цвету
    if(u_UseLut) {
        finalColor = applyLut(finalColor);
    }

    // 5. Виньетка
    float dist = length(TexCoords - vec2(0.5, 0.5));
    finalColor *= mix(1.0, 0.75, smoothstep(0.4, 0.85, dist));

    FragColor = vec4(finalColor, 1.0);
}