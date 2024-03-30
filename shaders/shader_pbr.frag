#version 450

//layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;
layout (binding = 1) uniform samplerCube albedoTexture;
layout (binding = 2) uniform samplerCube roughnessTexture;
layout (binding = 3) uniform samplerCube metalnessTexture;
layout (binding = 4) uniform samplerCube mipmapTexture;
layout (binding = 5) uniform samplerCube normalMap;
layout (binding = 6) uniform samplerCube displacementMap;
layout (binding = 7) uniform sampler2D texSampler;

layout(binding = 8) uniform LightObject1 {
    mat4 model;
    mat4 data;
} sphereLight[10];

layout(binding = 9) uniform LightObject2 {
    mat4 model;
    mat4 data;
} spotLight[10];

layout(binding = 10) uniform LightObject3 {
    mat4 model;
    mat4 data;
} sunLight[5];

layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 cameraPosition;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

layout(location = 0) out vec3 outColor;


// Apply Reinhard tone mapping operator
vec3 toneMapReinhard(vec3 linearRGB) {
    float exposure =1.0;
    return linearRGB / (linearRGB + vec3(1.0)) * exposure;
}

float gammaEncode(float value) {
    if (value <= 0.0031308) {
        return 12.92 * value;
    } else {
        return 1.055 * pow(value, 1.0 / 2.4) - 0.055;
    }
}

vec3 gammaEncode(vec3 color) {
    return vec3(gammaEncode(color.r), gammaEncode(color.g), gammaEncode(color.b));
}

vec4 gammaEncode(vec4 color) {
    return vec4(gammaEncode(color.x), gammaEncode(color.y), gammaEncode(color.z), gammaEncode(color.w));
}

vec3 rgbe2rgb(vec4 rgbe) {
    // Extract exponent
    
    if (rgbe.x + rgbe.y + rgbe.z + rgbe.w == 0) return vec3(0.0);

    rgbe = rgbe * 255;
    int exponent = 128;//int(floor(rgbe.w));
    // Convert RGBE to linear RGB' using the specified formula
    return exp2(exponent - 128) * ((rgbe.xyz+ 0.5)/256);
}

// functions for calculating BRDF
//https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#appendix-b-brdf-implementation

float mixBRDF(float brdf1, float brdf2, float p){
    return (p*brdf1) + ((1-p)*brdf2);
}

vec3 mixColor(vec3 c1, vec3 c2, float p){
    return (p*c1) +((1-p)*c2);
}

void main() {
    float roughness = gammaEncode(texture(roughnessTexture, surfaceNormal).xyz).x;
    float metalness = gammaEncode(texture(metalnessTexture, surfaceNormal).xyz).x;
    vec3 albedo =  rgbe2rgb(texture(albedoTexture, surfaceNormal));

    vec3 I = position - cameraPosition;

    float NoV = clamp(dot(normalize(surfaceNormal), normalize(-I)), 0, 1.0); //cosine theta
    vec3 reflected = reflect(normalize(I), normalize(surfaceNormal));

    vec3 prefilteredColor = rgbe2rgb(textureLod(mipmapTexture, reflected, roughness/0.2));

    vec2 EnvBRDF = gammaEncode(texture(texSampler, vec2(roughness,  NoV)).xyz).xy;
    vec3 c = (vec3(1) * EnvBRDF.x) + EnvBRDF.y;

    vec3 metal_brdf = prefilteredColor * c;

    vec3 dielectric_brdf = mixColor(albedo/3.14159, prefilteredColor,
       1-((0.04 * EnvBRDF.x) + EnvBRDF.y));

    outColor = toneMapReinhard(mixColor(metal_brdf, dielectric_brdf, metalness));
}
