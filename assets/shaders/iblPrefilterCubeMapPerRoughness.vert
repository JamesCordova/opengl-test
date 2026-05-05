#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 FragPos;

void main()
{
    FragPos = aPos;

    mat4 rotView = view;
    // mat4 rotView = mat4(mat3(view)); // no translation // Not needed because is not planned to be moved the mat4 uniform
    vec4 clipPos = projection * rotView * vec4(FragPos, 1.0);
    gl_Position = clipPos;
    // gl_Position = clipPos.xyww; // it's not needed beacause it will be the only one thing preprocessing at a time
}