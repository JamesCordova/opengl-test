#version 330 core
out vec4 FragColor;
in vec3 FragPos;

uniform samplerCube environmentMap;

const float PI =3.14159265359;

void main()
{
    vec3 normal = normalize(FragPos);

    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0; phi < 2 * PI; phi += sampleDelta)
    {
        for(float theta = 0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // we generated angles based on sampleDelta, is in spherical, so we pass it to cartessian
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // the vector generated is in tangent space, so we translate it to world space
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
            
            // the cos represents the lighting strength if is almost perpendicular to normal, then less strenght
            // the sin tries to balancce how much of the sphere is represented by each tile,
            // in the ecuator there are less samples for the sphere surface, in the polar part are too much samples,
            // so sin can handle that where 0 is polar and 90 ecuator  
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / nrSamples);
    
    FragColor = vec4(irradiance, 1.0);
}