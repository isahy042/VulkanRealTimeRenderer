#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;

layout(binding = 1) uniform LightObject {
    mat4 model;
    mat4 data;
    mat4 proj;
    mat4 view;
} light;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec3 position;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 lightPosition;
layout(location = 3) out vec3 lightDirection;
layout(location = 4) out float lightMaxReach;

void main() {
    // for spot light only 

    gl_Position = light.proj * light.view * ubo.model * vec4(inPosition, 1.0);

    vec4 t = ubo.model * vec4(inPosition, 1.0);
    position = t.xyz/t.w;
    normal = normalize(mat3(transpose(inverse(ubo.model))) * inNormal);

    lightPosition = vec3(light.model * vec4(0,0,0,1));
    lightDirection = normalize(light.data[3].xyz);
    lightMaxReach = light.data[1][3];
}