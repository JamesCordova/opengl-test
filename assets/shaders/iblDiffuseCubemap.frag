#version 330 core
out vec4 FragColor;
  
uniform sampler2D equirectangularMap;

in vec3 Normal;
in vec3 FragPos;

const vec2 invAtan = vec2(0.1591, 0.3183); 
vec2 SampleSphericalMap(vec3 v);

void main()
{
    vec2 uv = SampleSphericalMap(normalize(FragPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}