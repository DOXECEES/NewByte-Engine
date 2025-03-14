#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 oNormal;
in vec2 TexCoords;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float shine;

uniform vec3 viewPos;

uniform sampler2D ourTexture;


struct DirectionalLight
{
    vec3 LightPos; // Направление света
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};

uniform DirectionalLight light[32];

void main()
{
    vec3 norm = normalize(oNormal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 ambient = vec3(0.0);
    for (int i = 0; i < 2; ++i)
    {
        ambient += light[i].La;
    }
    ambient *= Ka;

    vec3 result = ambient;

    for (int i = 0; i < 2; ++i)
    {
        vec3 lightDir = normalize(-light[i].LightPos); 
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light[i].Ld * (diff * Kd);

        // Specular
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = light[i].Ls * (spec);

        result += diffuse ;
		result += specular * 32;
		}

    FragColor = texture(ourTexture, TexCoords);
}
