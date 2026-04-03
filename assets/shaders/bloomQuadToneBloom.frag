#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

#define REINHARD_TONE_MAPPING 1
#define EXPOSURE_TONE_MAPPING 2

uniform sampler2D hdrBuffer;
uniform sampler2D bloomBlur;
uniform int hdrMode;
uniform bool bloomEnabled;
uniform float exposure;
uniform bool gammaEnabled;
uniform float gammaFactor;

vec3 applyToneMapping(vec3 hdrColor);
vec3 reinhardToneMapping(vec3 hdrColor);
vec3 exposureToneMapping(vec3 hdrColor);

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    if (bloomEnabled)
        hdrColor += texture(bloomBlur, TexCoords).rgb;
    vec3 mapped = applyToneMapping(hdrColor);
    mapped = gammaEnabled ? pow(mapped, vec3(1.0 / gammaFactor)) : mapped;
    FragColor = vec4(mapped, 1.0);
}

vec3 applyToneMapping(vec3 hdrColor)
{
    switch (hdrMode)
    {
        case REINHARD_TONE_MAPPING:
            hdrColor = reinhardToneMapping(hdrColor);
            break;
        case EXPOSURE_TONE_MAPPING:
            hdrColor = exposureToneMapping(hdrColor);
            break;
        default:
            break;
    }

    return hdrColor;
}

vec3 reinhardToneMapping(vec3 hdrColor)
{
    return hdrColor / (hdrColor + 1.0);
}

vec3 exposureToneMapping(vec3 hdrColor)
{
    return vec3(1.0) - exp(-hdrColor * exposure);
}