#version 330 core
struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct PointLight
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 1

uniform Material material;
uniform PointLight pointLight[NR_POINT_LIGHTS];
uniform vec3 viewPos;
uniform bool blinnMode;

in VS_OUT
{
    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoords;

}
fs_in;

out vec4 FragColor;

// functions
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // properties
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0);

    // phase 1: directional lighting
    // phase 2: point lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result += CalculatePointLight(pointLight[i], norm, fs_in.FragPos, viewDir);
    }
    // phase 3: spot light

    FragColor = vec4(result, 1.0);
    // FragColor = texture(material.texture_diffuse1, fs_in.TexCoords);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    float spec = 0.0;
    if (blinnMode)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, fs_in.TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}