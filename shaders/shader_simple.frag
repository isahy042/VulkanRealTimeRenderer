#version 450

//layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;
layout (binding = 1) uniform samplerCube UnusedTexture1;
layout (binding = 2) uniform samplerCube UnusedTexture2;
layout (binding = 3) uniform samplerCube UnusedTexture3;
layout (binding = 4) uniform samplerCube UnusedTexture4;
layout (binding = 5) uniform samplerCube UnusedTexture5;
layout (binding = 6) uniform samplerCube UnusedTexture6;

layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 cameraPosition;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

layout(location = 0) out vec3 outColor;

// vec3 rgbe2rgb(vec4 rgbe) {
//     // Extract exponent
//     rgbe = rgbe * 255;
//     int exponent = int(floor(rgbe.w));
//     // Convert RGBE to linear RGB' using the specified formula
//     return exp2(exponent - 128) * ((rgbe.xyz + 0.5)/256);
// }

// // Apply Reinhard tone mapping operator
// vec3 toneMapReinhard(vec3 linearRGB) {
//     float exposure = 1.0;
//     return linearRGB / (linearRGB + vec3(1.0)) * exposure;
// }

void main() {
    outColor = inColor.xyz;
}

