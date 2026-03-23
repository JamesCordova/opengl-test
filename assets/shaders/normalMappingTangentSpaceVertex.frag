#version 330 core
out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
}
fs_in;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
};

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform Material material;

uniform float shininess;

void main()
{
    vec3 normal = texture(material.texture_normal1, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    // diffuse texture color
    vec3 color = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfwayDir, normal), 0.0), shininess);
    vec3 specular = spec * texture(material.texture_specular1, fs_in.TexCoords).rgb;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
    // FragColor = texture(material.texture_normal1, fs_in.TexCoords);
}