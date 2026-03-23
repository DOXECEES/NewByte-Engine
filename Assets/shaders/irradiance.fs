#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
    vec3 N = normalize(localPos);

    vec3 irradiance = vec3(0.0);

    // Более стабильный tangent space (избегаем singularity на полюсах)
    vec3 up       = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 right    = normalize(cross(up, N));
    vec3 tangentUp = cross(N, right);

    const float sampleDelta = 0.010;  // 0.008–0.012 для финальной карты (очень много сэмплов!)
    float nrSamples = 0.0;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                cos(theta)
            );

            vec3 sampleVec = tangentSample.x * right +
                             tangentSample.y * tangentUp +
                             tangentSample.z * N;

            sampleVec = normalize(sampleVec);  // на всякий случай

            vec3 radiance = texture(environmentMap, sampleVec).rgb;

            // Очень мягкий clamp — защищает от солнца, но сохраняет динамику
            radiance = min(radiance, vec3(35.0));  // 20–50 — подбери под свою HDRI

            float cosTheta = max(cos(theta), 0.0);
            float sinTheta = sin(theta);

            irradiance += radiance * cosTheta * sinTheta;
            nrSamples += 1.0;
        }
    }

    // Правильный нормализующий коэффициент
    irradiance *= (PI / nrSamples);

    FragColor = vec4(irradiance, 1.0);
}