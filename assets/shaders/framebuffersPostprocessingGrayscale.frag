#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec4 texColor = texture(screenTexture, TexCoords);

    float average = (texColor.r + texColor.g + texColor.b) / 3.0;
    // if (texColor.a < 0.1)
    //     discard;
    FragColor = vec4(average, average, average, texColor.a);
}