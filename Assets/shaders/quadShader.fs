#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap; // Ваша основная текстура (цвета)
uniform vec2 screenSize;    // Размер экрана

// Эти настройки будут активны только если определен USE_FXAA
#ifdef USE_FXAA
    #define FXAA_REDUCE_MIN   (1.0/128.0)
    #define FXAA_REDUCE_MUL   (1.0/8.0)
    #define FXAA_SPAN_MAX     8.0
#endif

void main()
{
    vec3 finalColor;

#ifdef USE_FXAA
    // --- ЛОГИКА FXAA ---
    vec2 inverseScreenSize = vec2(1.0) / screenSize;

    vec3 rgbNW = texture(depthMap, TexCoords + (vec2(-1.0, -1.0) * inverseScreenSize)).rgb;
    vec3 rgbNE = texture(depthMap, TexCoords + (vec2(1.0, -1.0) * inverseScreenSize)).rgb;
    vec3 rgbSW = texture(depthMap, TexCoords + (vec2(-1.0, 1.0) * inverseScreenSize)).rgb;
    vec3 rgbSE = texture(depthMap, TexCoords + (vec2(1.0, 1.0) * inverseScreenSize)).rgb;
    vec3 rgbM  = texture(depthMap, TexCoords).rgb;

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
        texture(depthMap, TexCoords + dir * (1.0/3.0 - 0.5)).rgb +
        texture(depthMap, TexCoords + dir * (2.0/3.0 - 0.5)).rgb);
    
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(depthMap, TexCoords + dir * (0.0/3.0 - 0.5)).rgb +
        texture(depthMap, TexCoords + dir * (3.0/3.0 - 0.5)).rgb);

    float lumaB = dot(rgbB, luma);

    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        finalColor = rgbA;
    } else {
        finalColor = rgbB;
    }
#else
    // --- БЕЗ СГЛАЖИВАНИЯ ---
    finalColor = texture(depthMap, TexCoords).rgb;
#endif

    // --- ОБЩИЕ ЭФФЕКТЫ (ВИНИТКА) ---
    // Выполняются всегда, независимо от выбора сглаживания
    vec2 center = vec2(0.5, 0.5);
    float dist = length(TexCoords - center);
    float vignette = smoothstep(0.45, 0.75, dist);
    finalColor *= mix(1.0, 0.7, vignette);
    FragColor = vec4(finalColor, 1.0);
}