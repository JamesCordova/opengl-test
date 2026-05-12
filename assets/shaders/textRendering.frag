#version 330
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D characterBitmap;
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(characterBitmap, TexCoords).r);
    FragColor = vec4(textColor, 1.0) * sampled;
}