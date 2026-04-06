#version 330 core
out float FragColor;

in VS_OUT
{
    vec2 TexCoords;
}
fs_in;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

uniform int width;
uniform int height;
uniform int kernelSize;
uniform float radius;
uniform float bias;

vec2 noiseScale = vec2(800.0 / 4.0, 600.0 / 4.0);

void main()
{
    noiseScale = vec2(width / 4.0, height / 4.0);
    vec3 fragPos = texture(gPosition, fs_in.TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, fs_in.TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, fs_in.TexCoords * noiseScale).xyz);

    vec3 proyected = normal * dot(randomVec, normal);
    // we could change randomvec to change this
    vec3 tangent = normalize(randomVec - proyected);
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; i++)
    {
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(gPosition, offset.xy).z;
        // occlusion += (sampleDepth >= samplePos.z ? 1.0 : 0.0);
        // range check becasue we are in cylinder comparizon not, sphere I suppose
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}