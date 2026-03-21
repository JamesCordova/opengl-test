#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{
    float depthValue = texture(depthMap, TexCoords).r;
    // if (texColor.a < 0.1)
    //     discard;
    FragColor = vec4(vec3(depthValue), 1.0);
}