#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 surfaceNormal;
//layout(location = 1) out vec2 fragTexCoord;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    surfaceNormal = inNormal;//vec3(transpose(inverse(ubo.proj * ubo.view * ubo.model)) * vec4(inNormal, 0.0));
    
    //vec3 fragColor = finColor;    // Determine which face of the cube the surface normal points towards


    // if (abs(surfaceNormal.x) >= abs(surfaceNormal.y) && abs(surfaceNormal.x) >= abs(surfaceNormal.z)){
    //     if (surfaceNormal.x >= 0){
    //         // Right face
    //         fragTexCoord = vec2((-surfaceNormal.z + 1.0) / 2.0, (-surfaceNormal.y + 1.0) / 12.0);
    //     } else {
    //         // Left face
    //         fragTexCoord = vec2((surfaceNormal.z + 1.0) / 2.0, (1.0/6.0)+(-surfaceNormal.y + 1.0) / 12.0);
    //     }
    // } else if (abs(surfaceNormal.y) >= abs(surfaceNormal.x) && abs(surfaceNormal.y) >= abs(surfaceNormal.z)) {
    //     if (surfaceNormal.y >= 0){
    //         // Top face
    //         fragTexCoord = vec2((surfaceNormal.x + 1.0) / 2.0, (2.0/6.0)+(surfaceNormal.z + 1.0) / 12.0);
    //     } else {
    //         // Bottom face
    //         fragTexCoord = vec2((surfaceNormal.x + 1.0) / 2.0, (3.0/6.0)+(-surfaceNormal.z + 1.0) / 12.0);
    //     }
    // } else {
    //     if (surfaceNormal.z >= 0){
    //         // Front face
    //         fragTexCoord = vec2((surfaceNormal.x + 1.0) / 2.0, (4.0/6.0)+(-surfaceNormal.y + 1.0) / 12.0);
    //     } else {
    //         // Back face
    //         fragTexCoord = vec2((-surfaceNormal.x + 1.0) / 2.0, (5.0/6.0)+(-surfaceNormal.y + 1.0) / 12.0);
    //     }
    // }

    //fragTexCoord = inTexCoord;
}