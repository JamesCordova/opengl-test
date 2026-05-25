#version 420 core

// shader outputs mrt's
layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;

// material color
uniform vec4 color;

void main()
{
    float weight = max(min(1.0, max(max(color.r, color.g), color.b) * color.a), color.a) * clamp(0.03 / (1e-5 + pow(gl_FragCoord.z / 200, 4.0)), 1e-2, 3e3); 

    accum = vec4(color.rgb * color.a, color.a) * weight;
    reveal = color.a;
}
