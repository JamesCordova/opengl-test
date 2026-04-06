#version 330 core
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

in VS_OUT
{
    vec2 TexCoords;
}
fs_in;

struct Light
{
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
    float Radius;
};

uniform Light light;
uniform float shininess = 8.0;

void main()
{
    // retrieve data
    vec3 FragPos = texture(gPosition, fs_in.TexCoords).rgb;
    vec3 Normal = texture(gNormal, fs_in.TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, fs_in.TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, fs_in.TexCoords).r;

    // blinn phong
    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 lighting = ambient;
    vec3 viewDir = normalize(-FragPos);
    // diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(lightDir, Normal), 0.0) * Diffuse * light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
    vec3 specular = light.Color * spec;
    // attenuation
    float dist = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * dist + light.Quadratic * dist * dist);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;
    FragColor = vec4(lighting, 1.0);
}