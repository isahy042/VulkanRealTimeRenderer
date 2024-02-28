#version 450

//layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;
layout (binding = 1) uniform samplerCube cubeMapTexture;

layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;

layout(location = 0) out vec4 outColor;

void main() {
    // just environment
    outColor = texture(cubeMapTexture, surfaceNormal);
}

// sources: 
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
