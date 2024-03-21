#version 450

//layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;
layout (binding = 1) uniform samplerCube cubeMapTexture;
layout (binding = 2) uniform samplerCube unusedTexture1;
layout (binding = 3) uniform samplerCube unusedTexture2;
layout (binding = 4) uniform samplerCube unusedTexture3;
layout (binding = 5) uniform samplerCube normalMap;
layout (binding = 6) uniform samplerCube displacementMap;
layout (binding = 7) uniform sampler2D texSampler;


layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 cameraPosition;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

layout(location = 0) out vec3 outColor;

vec3 rgbe2rgb(vec4 rgbe) {
    if (rgbe.x + rgbe.y + rgbe.z + rgbe.w == 0) return vec3(0.0);

    // Extract exponent
    rgbe = rgbe * 255;
    int exponent = int(floor(rgbe.w));
    // Convert RGBE to linear RGB' using the specified formula
    return exp2(exponent - 128) * ((rgbe.xyz + 0.5)/256);
}

// Apply Reinhard tone mapping operator
vec3 toneMapReinhard(vec3 linearRGB) {
    float exposure = 1.0;
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


void main() {

    vec3 normalColor = (gammaEncode(texture(normalMap, surfaceNormal).xyz) - 0.5) * 2;
    vec3 newNormal = vec3(
        (normalColor.x * tang.x) + (normalColor.y * bitang.x) + (normalColor.z * surfaceNormal.x), 
        (normalColor.x * tang.y) + (normalColor.y * bitang.y) +(normalColor.z * surfaceNormal.y),
        (normalColor.x * tang.z) + (normalColor.y * bitang.z) +(normalColor.z * surfaceNormal.z));


    vec3 I = cameraPosition - position;
    vec3 R = reflect(normalize(I), normalize(newNormal));
    outColor = toneMapReinhard(rgbe2rgb(texture(cubeMapTexture, R)));
    //outColor = texture(cubeMapTexture, R).xyz;

}

// sources: 
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
