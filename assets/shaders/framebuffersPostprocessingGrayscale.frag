#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec4 texColor = texture(screenTexture, TexCoords);

    // float average = (texColor.r + texColor.g + texColor.b) / 3.0;
    float average = 0.2126 * texColor.r + 0.7152 * texColor.g + 0.0722 * texColor.b;
    // if (texColor.a < 0.1)
    //     discard;
    FragColor = vec4(average, average, average, texColor.a);
}