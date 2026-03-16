#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out VS_OUT
{
    out vec3 FragPos;
    out vec3 Normal;
    out vec2 TexCoords;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    // Normal = aNormal; // work with translation only, ignore scaling and rotation for nows
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
}