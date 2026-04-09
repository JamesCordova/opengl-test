#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;
uniform mat4 normalMatrix; // as uniform if we want max performance without instancing support

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}
vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    
    mat3 normalMat = transpose(inverse(mat3(model)));
    vs_out.Normal = normalMat * aNormal;

    vs_out.TexCoords = aTexCoords;
}