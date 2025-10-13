#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 oNormal;
in vec2 TexCoords;
in mat3 TBN;
in vec3 oModelFragPos;
in vec4 FragPosLightSpace;


uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float shine;

uniform vec3 viewPos;

uniform sampler2D ourTexture;
uniform sampler2D textureNormal;  

uniform sampler2D depthMap;

uniform int _COUNT_OF_POINTLIGHT_;
uniform int _COUNT_OF_SPOTLIGHT_;
uniform int _COUNT_OF_DIRECTIONLIGHT_;

struct DirectionalLight
{
    vec3 direction;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};


struct PointLight
{
    vec3 La;
    vec3 Ld;
    vec3 Ls; 
	vec3 position;
	float point_const_coof;
	float point_linear_coof;
	float point_exp_coof;
	float intensity;
};

uniform sampler2D shadowMap;

uniform DirectionalLight light[32];
uniform PointLight lightPoint[32];


vec3 calculatePointLight(PointLight pointLight, vec3 normal, vec3 viewDir)
{
	
    vec3 lightDir = normalize(pointLight.position - oModelFragPos);
    float distance = length(pointLight.position - oModelFragPos);
	if(distance < 0.0)
	{
		lightDir *= -1;
	}

    // Расчёт затухания
    float attenuation = 1.0 / (pointLight.point_const_coof + 
                               pointLight.point_linear_coof * distance + 
                               pointLight.point_exp_coof * (distance * distance));

    // Амбиент
    vec3 ambient = pointLight.La * pointLight.intensity;

    // Диффузное освещение
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = pointLight.Ld * diff * pointLight.intensity;

    // Спекулярное освещение (Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shine);
    vec3 specular = pointLight.Ls * spec * pointLight.intensity;

    return ((ambient + diffuse) * attenuation);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}


void main()
{
	vec3 norm = texture(textureNormal, TexCoords * 32).rgb;
    norm = normalize(norm * 2.0 - 1.0);   
    norm = normalize(TBN * norm);  // Применяем TBN к нормали

    
	vec3 viewDir = normalize(viewPos - FragPos);

    vec3 ambient = vec3(0.0);
    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i)
    {
        ambient += light[i].La;
    }
    ambient *= Ka;

    vec3 result = ambient;


    for (int i = 0; i < _COUNT_OF_DIRECTIONLIGHT_; ++i)
    {
        vec3 lightDir = normalize(light[i].direction); 
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light[i].Ld * (diff * Kd);

        // Specular
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = light[i].Ls * (spec);

		float shadow = ShadowCalculation(FragPosLightSpace, norm, light[i].direction);
		result += (1.0 - shadow) * (diffuse + specular);
	}
	

		
	result += calculatePointLight(lightPoint[0], norm, viewDir);

	FragColor = texture(ourTexture,TexCoords * 32) * vec4(result, 1.0);
}
