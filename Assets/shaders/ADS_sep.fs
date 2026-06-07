#version 450 core
#extension GL_ARB_bindless_texture : require

out vec4 FragColor;

in vec3 FragPos;
in vec2 v_TexCoords;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform float u_BaseColorFactor = 1.0;

uniform float u_RoughnessFactor = 1.0;
uniform float u_MetallicFactor  = 1.0;
uniform float u_OcclusionFactor = 1.0;

layout(bindless_sampler) uniform sampler2D u_EmissionMap;
uniform float u_EmissionMapStrength = 1.0;

layout(bindless_sampler) uniform sampler2D shadowMap;
layout(bindless_sampler) uniform sampler2D u_SsaoMap;

layout(bindless_sampler) uniform sampler2D u_OcclusionMap;
layout(bindless_sampler) uniform sampler2D u_RoughnessMap;
layout(bindless_sampler) uniform sampler2D u_MetallicMap;

uniform bool u_NormalMapFlipY   = false;
uniform float u_NormalMapStrength = 1.0;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D   u_BrdfLUT;

uniform vec3  u_CameraPos;
uniform float u_Exposure    = 1.0;
uniform float u_IBLStrength = 1.0;
uniform bool  u_UseIBL      = true;
uniform bool  u_UseSSAO     = true;
uniform bool  u_EnableFog   = false;

// Fog
uniform vec3  u_FogColor                = vec3(0.5, 0.6, 0.7);
uniform float u_FogDensity              = 0.015;
uniform float u_FogHeight               = 0.0;
uniform float u_FogHeightFalloff        = 0.1;
uniform float u_FogInscatteringIntensity = 1.0;
uniform float u_FogPhaseG               = 0.2;
uniform float u_FogAmbientIntensity     = 0.1;

uniform int   u_VolumetricSteps    = 64;
uniform float u_VolumetricDensity  = 0.03;
uniform float u_VolumetricStrength = 1.5;
uniform bool  u_EnableVolumetric   = true;

uniform mat4 lightView;
uniform mat4 lightProj;
uniform float u_LightSize = 0.002;
uniform vec2  u_ScreenResolution;

// ----------------- Constants -----------------
const float PI      = 3.14159265359;
const float INV_PI  = 0.31830988618;
const float EPSILON = 1e-7;

// ----------------- Light Structures -----------------
struct DirectionalLight {
    vec3  direction;
    vec3  Ld;
};

struct PointLight {
    vec3        Ld;
    vec3        position;
    float       intensity;
    float       point_const_coof;
    float       point_linear_coof;
    float       point_exp_coof;
    float       farPlane;
    int         hasShadow;
    samplerCube pointShadowMap;
};

layout(std140, binding = 0) uniform PointLightBlock {
    PointLight lightPoint[32];
    int _COUNT_OF_POINTLIGHT_;
};

uniform DirectionalLight light[8];
uniform int _COUNT_OF_DIRECTIONLIGHT_;

// ----------------- Helpers -----------------
float InterleavedGradientNoise(vec2 px) {
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(magic.xy, magic.xy)));
}

vec2 VogelDiskSample(int i, int n, float angle) {
    const float goldenAngle = 2.39996323;
    float r     = sqrt((float(i) + 0.5) / float(n));
    float theta = float(i) * goldenAngle + angle;
    return vec2(cos(theta), sin(theta)) * r;
}

// ----------------- Shadow -----------------
float FindBlockerDepth(vec2 uv, float zReceiver, float searchRadiusUV, float rotation) {
    float sumDepth = 0.0;
    int   blockers = 0;
    for (int i = 0; i < 24; i++) {
        vec2  offset = VogelDiskSample(i, 24, rotation) * searchRadiusUV;
        float depth  = texture(shadowMap, uv + offset).r;
        if (depth < zReceiver) { sumDepth += depth; blockers++; }
    }
    return (blockers == 0) ? -1.0 : sumDepth / float(blockers);
}

// Slope-scale depth bias для directional shadows
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0 || projCoords.z < 0.0) return 0.0;

    vec2  uv        = projCoords.xy;
    float zReceiver = projCoords.z;
    vec2  texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float rotation  = InterleavedGradientNoise(gl_FragCoord.xy) * 2.0 * PI;

    float ndotl     = clamp(dot(normal, lightDir), 0.0, 1.0);
    float slopeBias = 0.0025 * tan(acos(ndotl));
    float bias      = clamp(slopeBias, 0.0001, 0.005);

    float searchRadiusUV  = clamp(u_LightSize * 250.0, 2.0, 16.0) * texelSize.x;
    float avgBlockerDepth = FindBlockerDepth(uv, zReceiver - bias, searchRadiusUV, rotation);
    if (avgBlockerDepth < 0.0) return 0.0;

    float penumbra      = zReceiver - avgBlockerDepth;
    float filterRadiusUV = clamp(penumbra * u_LightSize * 3500.0, 2.0, 25.0) * texelSize.x;

    float shadow = 0.0;
    for (int i = 0; i < 64; i++) {
        vec2  offset   = VogelDiskSample(i, 64, rotation) * filterRadiusUV;
        float pcfDepth = texture(shadowMap, uv + offset).r;
        shadow += ((zReceiver - bias) > pcfDepth) ? 1.0 : 0.0;
    }
    return shadow / 64.0;
}

float PointShadowCalculation(vec3 fragPos, vec3 lightPos, float farPlane, samplerCube shadowCube, vec3 N) {
    vec3 fragToLight  = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    vec3  forward      = normalize(fragToLight);

    vec3 up      = abs(forward.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 right   = normalize(cross(up, forward));
    vec3 tangent = cross(forward, right);

    float ndotL = clamp(dot(N, -forward), 0.0, 1.0);
    float bias   = max(0.002 * (1.0 - ndotL), 0.0005);

    float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 2.0 * PI;
    float radius   = mix(0.002, 0.04, currentDepth / farPlane);

    float shadow = 0.0;
    for (int i = 0; i < 32; ++i) {
        vec2 disk       = VogelDiskSample(i, 32, rotation) * radius;
        vec3 sampleDir  = normalize(forward + right * disk.x + tangent * disk.y);
        float closestDepth = texture(shadowCube, sampleDir).r * farPlane;
        shadow += (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
    }
    return shadow / 32.0;
}

// ----------------- PBR Math -----------------
float DistributionGGX(float NdotH, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d + EPSILON);
}

float GeometrySmithDirect(float NdotV, float NdotL, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float gv = NdotV / (NdotV * (1.0 - k) + k);
    float gl = NdotL / (NdotL * (1.0 - k) + k);
    return gv * gl;
}

float GeometrySmithIBL(float NdotV, float NdotL, float roughness) {
    float a = roughness * roughness;
    float k = a / 2.0;
    float gv = NdotV / (NdotV * (1.0 - k) + k);
    float gl = NdotL / (NdotL * (1.0 - k) + k);
    return gv * gl;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float PointLightAttenuation(float dist, float constC, float linC, float expC) {
    if (constC < EPSILON && linC < EPSILON && expC < EPSILON)
        return 1.0 / max(dist * dist, EPSILON);
    return 1.0 / max(constC + linC * dist + expC * dist * dist, EPSILON);
}

// ----------------- FOG -----------------
float HenyeyGreenstein(float cosTheta, float g) {
    float g2    = g * g;
    float denom = 1.0 + g2 - 2.0 * g * cosTheta;
    return (1.0 - g2) / (4.0 * PI * pow(max(denom, EPSILON), 1.5));
}

float CalculateHeightFog(vec3 camPos, vec3 worldPos) {
    vec3  viewDir = worldPos - camPos;
    float dist    = length(viewDir);
    vec3  dir     = viewDir / max(dist, EPSILON);

    float fogAtCamera = u_FogDensity * exp(-u_FogHeightFalloff * (camPos.y - u_FogHeight));
    float slope = dir.y;
    if (abs(slope) < 0.0001) slope = 0.0001 * sign(slope + EPSILON);
    float fogAmount = (fogAtCamera / (u_FogHeightFalloff * slope))
                    * (1.0 - exp(-u_FogHeightFalloff * slope * dist));
    return clamp(fogAmount, 0.0, 1.0);
}

vec3 ApplyLightAwareFog(vec3 surfaceColor, vec3 worldPos, vec3 camPos, vec3 V) {
    float fogFactor = CalculateHeightFog(camPos, worldPos);
    if (fogFactor <= 0.001) return surfaceColor;

    vec3 fogLighting = u_FogColor * u_FogAmbientIntensity;

    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3  L        = normalize(-light[i].direction);
        float cosTheta = dot(-V, L);
        float phase    = HenyeyGreenstein(cosTheta, u_FogPhaseG);
        float shadow   = (i == 0) ? ShadowCalculation(FragPosLightSpace, vec3(0.0, 1.0, 0.0), L) : 0.0;
        fogLighting   += light[i].Ld * phase * u_FogInscatteringIntensity * (1.0 - shadow);
    }

    for (int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        float dist  = length(lightPoint[i].position - worldPos);
        float atten = PointLightAttenuation(dist,
            lightPoint[i].point_const_coof,
            lightPoint[i].point_linear_coof,
            lightPoint[i].point_exp_coof);
        fogLighting += lightPoint[i].Ld * lightPoint[i].intensity * atten * 0.3;
    }

    return mix(surfaceColor, fogLighting, fogFactor);
}

// ----------------- Tonemapping -----------------
vec3 ACESFilmic(vec3 x) {
    x *= 0.6;
    float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 ApplyPostProcessing(vec3 color) {
    vec3 tonemapped = ACESFilmic(color);
    return pow(tonemapped, vec3(1.0 / 2.2));
}

// ----------------- Specular Occlusion -----------------
float SpecularOcclusion(float NdotV, float ao, float roughness) {
    return clamp(pow(NdotV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

// ----------------- Volumetric Light -----------------
vec3 VolumetricLight(vec3 worldPos, vec3 camPos) {
    vec3  rayVec  = worldPos - camPos;
    float rayLen = min(length(rayVec), 50.0);

    vec3  rayDir  = rayVec / rayLen;
    float stepLen = rayLen / float(u_VolumetricSteps);
    vec3  rayStep = rayDir * stepLen;

    float noise = InterleavedGradientNoise(gl_FragCoord.xy);
    vec3  pos   = camPos + rayStep * noise;

    vec3  scatter    = vec3(0.0);
    float extinction = u_VolumetricDensity * stepLen;

    vec3  L        = normalize(-light[0].direction);
    float cosTheta = dot(rayDir, L);
    float phase    = HenyeyGreenstein(cosTheta, u_FogPhaseG);

    for (int i = 0; i < u_VolumetricSteps; i++) {
        pos += rayStep;

        vec4  lsPos = lightProj * lightView * vec4(pos, 1.0);
        vec3  proj  = lsPos.xyz / lsPos.w * 0.5 + 0.5;

        float lit = 1.0;

        if (proj.x > 0.01 && proj.x < 0.99 &&
            proj.y > 0.01 && proj.y < 0.99 &&
            proj.z > 0.0  && proj.z < 0.99)
        {
            float shadowDepth = texture(shadowMap, proj.xy).r;
            float bias = 0.0015;
            lit = (proj.z - bias > shadowDepth) ? 0.0 : 1.0;
        }

        float transmittance = exp(-u_VolumetricDensity * float(i) * stepLen);
        scatter += light[0].Ld * lit * phase * extinction * transmittance;
    }

    return scatter * u_VolumetricStrength;
}

// ======================== MAIN ========================
void main() {
    vec2 uv = v_TexCoords;
    vec2 screenUV = gl_FragCoord.xy / u_ScreenResolution;

    // ---------- Albedo ----------
    vec4 albedoSample = texture(u_AlbedoMap, uv);
    if (albedoSample.a < 0.01) discard;
    vec3 albedo = pow(albedoSample.rgb, vec3(2.2)) * u_BaseColorFactor;

    // ---------- Material Maps (Separate) ----------
    float ao        = texture(u_OcclusionMap, uv).r;
    float roughness = texture(u_RoughnessMap, uv).r;
    float metallic  = texture(u_MetallicMap, uv).r;

    ao        = mix(1.0, ao, u_OcclusionFactor);
    roughness = clamp(roughness * u_RoughnessFactor, 0.04, 1.0);
    metallic  = clamp(metallic  * u_MetallicFactor,  0.0,  1.0);

    // ---------- SSAO ----------
    float ssao = 1.0;
    if (u_UseSSAO) {
        ssao = texture(u_SsaoMap, screenUV).r;
    }
    float combinedAO = ao * ssao;

    // ---------- Normal Map ----------
    vec3 nMap = texture(u_NormalMap, uv).rgb * 2.0 - 1.0;
    if (u_NormalMapFlipY) nMap.y = -nMap.y;
    nMap.xy  *= u_NormalMapStrength;
    nMap       = normalize(nMap);

    vec3 N = normalize(TBN * nMap);
    vec3 V = normalize(u_CameraPos - FragPos);
    vec3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 0.0);
    vec3  F0    = mix(vec3(0.04), albedo, metallic);

    // ---------- Emission ----------
    vec3 emission = pow(texture(u_EmissionMap, uv).rgb, vec3(2.2)) * u_EmissionMapStrength;

    // ---------- Direct Lighting ----------
    vec3 Lo = vec3(0.0);

    // Directional lights
    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i) {
        vec3  L     = normalize(-light[i].direction);
        vec3  H     = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float shadow = (i == 0) ? ShadowCalculation(FragPosLightSpace, N, L) : 0.0;

        float D  = DistributionGGX(NdotH, roughness);
        float G  = GeometrySmithDirect(NdotV, NdotL, roughness);
        vec3  F  = fresnelSchlick(HdotV, F0);

        vec3 numerator    = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + EPSILON;
        vec3  specular    = numerator / denominator;

        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

        Lo += (1.0 - shadow) * (kD * albedo * INV_PI + specular) * light[i].Ld * NdotL;
    }

    // Point lights
    for (int i = 0; i < _COUNT_OF_POINTLIGHT_; ++i) {
        vec3  L     = normalize(lightPoint[i].position - FragPos);
        vec3  H     = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float dist  = length(lightPoint[i].position - FragPos);
        float atten = PointLightAttenuation(dist,
            lightPoint[i].point_const_coof,
            lightPoint[i].point_linear_coof,
            lightPoint[i].point_exp_coof);

        float shadow = (lightPoint[i].hasShadow == 1)
            ? PointShadowCalculation(FragPos, lightPoint[i].position,
                                     lightPoint[i].farPlane, lightPoint[i].pointShadowMap, N)
            : 0.0;

        float D  = DistributionGGX(NdotH, roughness);
        float G  = GeometrySmithDirect(NdotV, NdotL, roughness);
        vec3  F  = fresnelSchlick(HdotV, F0);

        vec3  specular = (D * G * F) / (4.0 * NdotV * NdotL + EPSILON);
        vec3  kD       = (vec3(1.0) - F) * (1.0 - metallic);

        Lo += (1.0 - shadow) * (kD * albedo * INV_PI + specular)
            * lightPoint[i].Ld * lightPoint[i].intensity * atten * NdotL;
    }

    // ---------- IBL / Ambient ----------
    vec3 ambient = vec3(0.0);

    if (u_UseIBL) {
        vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD_ibl = (vec3(1.0) - F_ibl) * (1.0 - metallic);

        vec3 irradiance  = textureLod(u_IrradianceMap, N, 0.0).rgb;
        vec3 diffuseIBL  = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 7.0;
        vec3  prefiltered = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2  brdf        = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
        vec3  specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y);

        float specOcc = SpecularOcclusion(NdotV, combinedAO, roughness);

        ambient = (kD_ibl * diffuseIBL * combinedAO + specularIBL * specOcc) * u_IBLStrength;
    } else {
        ambient = vec3(0.03) * albedo * combinedAO;
    }

    // ---------- Compose ----------
    vec3 finalColor = ambient + Lo + emission;

    // ---------- Fog ----------
    if (u_EnableFog) {
        finalColor = ApplyLightAwareFog(finalColor, FragPos, u_CameraPos, V);
    }

    // ---------- Tonemap + Gamma ----------
    vec3 tonemapped = ApplyPostProcessing(finalColor * u_Exposure);

    // ---------- Volumetric ----------
    if (u_EnableVolumetric && _COUNT_OF_DIRECTIONLIGHT_ > 0) {
        vec3 vol = VolumetricLight(FragPos, u_CameraPos);
        vol = pow(ACESFilmic(vol * u_Exposure * 0.5), vec3(1.0 / 2.2));
        tonemapped += vol;
    }

    FragColor = vec4(clamp(tonemapped, 0.0, 1.0), 1.0);
}