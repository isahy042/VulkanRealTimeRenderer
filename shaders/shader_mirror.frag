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

layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 cameraPosition;


layout(location = 0) out vec4 outColor;

void main() {
    vec3 I = cameraPosition - position;
    vec3 R = reflect(normalize(I), normalize(surfaceNormal));
    outColor = texture(cubeMapTexture, R);
}

// sources: 
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
