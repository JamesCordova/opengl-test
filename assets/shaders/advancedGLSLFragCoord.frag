#version 330 core
out vec4 FragColor;

uniform samplerCube cubemap;

void main()
{
    if (gl_FragCoord.x < 400)
        FragColor = vec4(0.14, 0.89, 0.86, 1.0);
    else
        FragColor = vec4(0.89, 0.14, 0.86, 1.0);

}