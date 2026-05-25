#version 420 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenImage;

void main()
{
    vec3 color = texture(screenImage, TexCoords).rgb; 
    FragColor = vec4(color, 1.0);
}
