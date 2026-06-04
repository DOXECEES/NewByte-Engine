#version 450 core
#extension GL_ARB_bindless_texture : require

layout(bindless_sampler) uniform sampler2D gFinalImage;
layout(bindless_sampler) uniform sampler2D gPosition;
layout(bindless_sampler) uniform sampler2D gNormal;
layout(bindless_sampler) uniform sampler2D gExtraComponents;

uniform mat4 projection;

noperspective in vec2 TexCoords;
out vec4 outColor;

// -------------------------------------------------------
// Параметры трассировки
// -------------------------------------------------------
const float RAY_STEP        = 0.05;   // Базовый шаг (view-space units)
const int   MAX_STEPS       = 128;    // Достаточно для большинства сцен
const int   BINARY_STEPS    = 16;     // Точность уточнения пересечения
const float THICKNESS       = 0.20;   // Допустимая «толщина» поверхности
const float NORMAL_BIAS     = 0.02;   // Смещение вдоль нормали (против самопересечений)
const float RAY_BIAS        = 0.01;   // Смещение вдоль луча (против z-fighting)
const float MAX_RAY_DIST    = 20.0;   // Максимальная длина луча (view-space)
const float MAX_MIP         = 8.0;    // Количество mip-уровней у gFinalImage

// -------------------------------------------------------
// Проекция view-позиции в UV [0,1]
// -------------------------------------------------------
vec2 projectToUV(vec3 viewPos) {
    vec4 clip = projection * vec4(viewPos, 1.0);
    clip.xyz /= clip.w;
    return clip.xy * 0.5 + 0.5;
}

// -------------------------------------------------------
// Чтение глубины из G-буфера в view-space Z (всегда < 0)
// -------------------------------------------------------
float sampleSceneZ(vec2 uv) {
    return texture(gPosition, uv).z;
}

// -------------------------------------------------------
// Бинарный поиск точки пересечения
// Логика: если сцена «позади» луча (sceneZ < hitPos.z в view-space,
// где Z отрицателен и «глубже» = меньше) — луч перелетел, шагаем назад.
// -------------------------------------------------------
vec3 binarySearch(vec3 rayDir, vec3 hitPos) {
    float halfStep = RAY_STEP * 0.5;
    for (int i = 0; i < BINARY_STEPS; i++) {
        halfStep *= 0.5;
        vec2  uv     = projectToUV(hitPos);
        float sceneZ = sampleSceneZ(uv);

        // В view-space Z отрицателен; луч перелетел если он "дальше" сцены
        // (т.е. hitPos.z < sceneZ, потому что оба отрицательные)
        if (hitPos.z < sceneZ)
            hitPos -= rayDir * halfStep; // перелёт — возвращаемся
        else
            hitPos += rayDir * halfStep; // недолёт — идём вперёд
    }
    return hitPos;
}

// -------------------------------------------------------
// Ray marching в view-space с адаптивным шагом
// Возвращает vec4(uv.xy, distanceFraction, hit)
// -------------------------------------------------------
vec4 rayMarch(vec3 rayDir, vec3 startPos, vec3 startNormal) {
    // Начальная позиция: отступаем от поверхности
    vec3  pos       = startPos + startNormal * NORMAL_BIAS + rayDir * RAY_BIAS;
    float stepScale = RAY_STEP;

    for (int i = 0; i < MAX_STEPS; i++) {
        pos += rayDir * stepScale;

        // За пределами экрана
        vec2 uv = projectToUV(pos);
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            return vec4(0.0);

        // Превысили максимальную длину луча
        float traveled = length(pos - startPos);
        if (traveled > MAX_RAY_DIST)
            return vec4(0.0);

        float sceneZ = sampleSceneZ(uv);

        // Пропускаем «пустые» пиксели (небо / фон без геометрии)
        // gPosition.z == 0.0 означает отсутствие геометрии
        if (sceneZ == 0.0) continue;

        // В view-space Z отрицателен и растёт (по модулю) вглубь.
        // Попадание: луч «ушёл за» поверхность (pos.z < sceneZ),
        // но не слишком глубоко (разница меньше THICKNESS).
        float diff = pos.z - sceneZ; // отрицательное значение при попадании

        if (diff < 0.0 && diff > -THICKNESS) {
            // Отсеиваем попадание в «заднюю» стенку объекта:
            // нормаль попавшего пикселя должна смотреть против луча
            vec3 hitNormal = normalize(texture(gNormal, uv).xyz);
            if (dot(hitNormal, rayDir) >= 0.0) continue;

            // Уточняем точку пересечения
            vec3 refined    = binarySearch(rayDir, pos);
            vec2 refinedUV  = projectToUV(refined);
            float distFrac  = length(refined - startPos) / MAX_RAY_DIST;
            return vec4(refinedUV, distFrac, 1.0);
        }
    }
    return vec4(0.0);
}

// -------------------------------------------------------
// Fresnel-Schlick (от угла NdotV, как для зеркального лепестка)
// -------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
void main() {
    // Читаем G-буфер
    vec3 viewPos = texture(gPosition, TexCoords).xyz;

    // Нет геометрии в этом пикселе
    if (viewPos.z == 0.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3  normal    = normalize(texture(gNormal, TexCoords).xyz);
    vec2  extra     = texture(gExtraComponents, TexCoords).rg;
    float metallic  = extra.r;
    float roughness = extra.g;

    // Слишком шероховатые поверхности — SSR не применяем
    // (дорогостоящий эффект бесполезен при diffuse-доминировании)
    if (roughness > 0.7) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 V = normalize(-viewPos); // Направление к камере (view-space)
    vec3 R = reflect(-V, normal); // Направление отражения

    // Луч идёт «от» камеры (R.z > 0 в view-space = за камеру) — пропускаем
    // (допускаем небольшое граничное значение для касательных отражений)
    if (R.z > 0.01) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Трассировка
    vec4 hit = rayMarch(R, viewPos, normal);
    if (hit.w < 0.5) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // ----------------------------------------------------------
    // Сэмплирование отражённого цвета
    // Mip зависит от roughness²  — перцептивно более линейная шкала
    // ----------------------------------------------------------
    float mip = roughness * roughness * MAX_MIP;
    vec3 reflectedColor = textureLod(gFinalImage, hit.xy, mip).rgb;

    // ----------------------------------------------------------
    // Затухания (Confidence / Visibility)
    // ----------------------------------------------------------

    // 1. Затухание у краёв экрана
    vec2 edgeDist   = min(hit.xy, 1.0 - hit.xy);
    float edgeFade  = smoothstep(0.0, 0.05, edgeDist.x)
                    * smoothstep(0.0, 0.05, edgeDist.y);

    // 2. Затухание по длине луча (дальние отражения менее достоверны)
    float distFade  = 1.0 - smoothstep(0.0, 1.0, hit.z);

    // 3. Затухание по углу отражения (грейзинг-углы — артефакты)
    float NdotV     = max(dot(normal, V), 0.0);
    float grazeFade = smoothstep(0.0, 0.15, NdotV);

    float confidence = edgeFade * distFade * grazeFade;

    // ----------------------------------------------------------
    // Fresnel — физически корректный вес отражения
    // F0 для диэлектриков ≈ 0.04, для металлов = albedo
    // ----------------------------------------------------------
    vec3 albedo = texture(gFinalImage, TexCoords).rgb;
    vec3 F0     = mix(vec3(0.04), albedo, metallic);
    // Угол между нормалью и вектором взгляда (геометрия лепестка)
    vec3 F      = fresnelSchlick(NdotV, F0);

    // ----------------------------------------------------------
    // Финальный цвет: Fresnel задаёт долю отражения,
    // confidence — достоверность трассированного луча.
    // Нет магических множителей — энергия сохраняется.
    // ----------------------------------------------------------
    vec3 finalReflection = reflectedColor * F * confidence;

    outColor = vec4(finalReflection, confidence);
    // Alpha = confidence: в финальном проходе используйте его
    // для блендинга со средой (IBL/cubemap fallback).
}
