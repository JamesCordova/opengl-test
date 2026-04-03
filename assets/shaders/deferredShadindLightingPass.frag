#version 330 core
out vec4 FragColor;

in VS_OUT
{
    vec2 TexCoords;
}
fs_in;

struct Light
{
    vec3 Position;
    vec3 Color;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform vec3 viewPos;
uniform float shininess = 32;

void main()
{
    // retrieve dat from g buffers
    vec3 FragPos = texture(gPosition, fs_in.TexCoords).rgb;
    vec3 Normal = texture(gNormal, fs_in.TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, fs_in.TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, fs_in.TexCoords).a;

    // calculations lighting
    vec3 lighting = Albedo * 0.1; // ambient
    vec3 viewDir = normalize(viewPos - FragPos);
    for (int i = 0; i < NR_LIGHTS; i++)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].Color;
        lighting += diffuse;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
        lighting += spec * Specular;
    }
    FragColor = vec4(lighting, 1.0);
}