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
    vec3 cameraPos = (ubo.cameraTrans * vec4(0.0,0.0,0.0, 1.0)).xyz;
    vec3 I = normalize(position - cameraPos);
    vec3 R = reflect(I, normalize(surfaceNormal));
    outColor = vec4(texture(cubeMapTexture, R).rgb, 1.0);
}

// sources: 
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
