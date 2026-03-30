#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdrEnabled;
uniform float exposure;
uniform bool gammaEnabled;
uniform float gammaFactor;

vec3 reinhardToneMapping(vec3 hdrColor);

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = hdrEnabled ? reinhardToneMapping(hdrColor) : hdrColor;
    mapped = gammaEnabled ? pow(mapped, vec3(1.0 / gammaFactor)) : mapped;
    FragColor = vec4(mapped, 1.0);
}

vec3 reinhardToneMapping(vec3 hdrColor)
{
    return hdrColor / (hdrColor + 1.0);
}