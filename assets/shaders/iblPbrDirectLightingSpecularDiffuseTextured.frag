#version 330 core
out vec4 FragColor;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}
fs_in;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 viewPos; //camPos

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 getNormalMap();

void main()
{
    vec3 albedo = pow(texture(albedoMap, fs_in.TexCoords).rgb, vec3(2.2)); // needs fix TODO gamma correction
    float metallic = texture(metallicMap, fs_in.TexCoords).r;
    float roughness = texture(roughnessMap, fs_in.TexCoords).r;
    float ao = texture(aoMap, fs_in.TexCoords).r;

    // vec3 N = normalize(fs_in.Normal);
    vec3 N = getNormalMap();
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < 4; i++)
    {
        vec3 L = normalize(lightPositions[i] - fs_in.FragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - fs_in.FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance   = lightColors[i] * attenuation;

        float NdotV = max(dot(N, V), 0.0);
        float clamppedHdotV = clamp(dot(H, V), 0.0, 1.0);

        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F  = fresnelSchlick(clamppedHdotV, F0);

        vec3 ks = F;
        vec3 kd = vec3(1.0) - ks;
        kd     *= 1.0 - metallic;

        vec3 lambert = albedo / PI;

        float NdotL                 = max(dot(N, L), 0.0); 
        vec3 numerator_CT           = D * G * F;
        float denominator_CT        = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular_cookTorrance  = numerator_CT / denominator_CT;

        vec3 brdf       = kd * lambert + specular_cookTorrance;
        Lo             += brdf * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic; 
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    vec3 color =  ambient + Lo;
    // tone mapping
    color = color / (color + vec3(1.0));
    // gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a_2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH_2 = NdotH * NdotH;

    float numerator = a_2;
    float denominator = (NdotH_2 * (a_2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return numerator / denominator;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;

    return numerator / denominator;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    float clampCosTheta = clamp(1.0 - cosTheta, 0.0, 1.0); // HdotV [0,1]
    return F0 + (1.0 - F0) * pow(clampCosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    float clampCosTheta = clamp(1.0 - cosTheta, 0.0, 1.0); // HdotV [0,1]
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clampCosTheta, 5.0);
}

vec3 getNormalMap()
{
    // vec3 tangentNormal = texture(normalMap, fs_in.TexCoords).rgb;
    // tangentNormal = tangentNormal * vec3(2.0) - vec3(1.0);
    // vec3 normalX = normalize(fs_in.Tangent) * tangentNormal.x;
    // vec3 normalY = normalize(fs_in.Bitangent) * tangentNormal.y;
    // vec3 normalZ = normalize(fs_in.Normal) * tangentNormal.z;
    // vec3 normalResult = vec3(normalX + normalY + normalZ); 
    // // vec3 normalResult = fs_in.TBN * tangentNormal;
    // return normalize(normalResult);
    vec3 tangentNormal = texture(normalMap, fs_in.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(fs_in.FragPos);
    vec3 Q2 = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N = normalize(fs_in.Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}