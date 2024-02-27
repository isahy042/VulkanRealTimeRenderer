#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 surfaceNormal;
layout(location = 1) out vec3 position;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    surfaceNormal = mat3(transpose(inverse(ubo.model))) * inNormal;//vec3(transpose(inverse(ubo.proj * ubo.view * ubo.model)) * vec4(inNormal, 0.0));
    position = vec3(ubo.model * vec4(inPosition, 1.0));
}