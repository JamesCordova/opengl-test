#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 modelByInstance;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    gl_Position = projection * view * modelByInstance * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    TexCoords = aTexCoords;
    Normal = mat3(transpose(inverse(view * modelByInstance))) * aNormal;
    FragPos = vec3(view * modelByInstance * vec4(aPos, 1.0f));
}