#version 330 core
out vec4 FragColor;
  
uniform vec3 objectColor;

in vec3 result;

void main()
{


    FragColor = vec4(result * objectColor, 0.0f);
}