#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;


layout(location = 0) out vec3 surfaceNormal;
layout(location = 1) out vec3 position;
layout(location = 2) out vec3 cameraPosition;
layout(location = 3) out vec4 outColor;
layout(location = 4) out vec3 tang;
layout(location = 5) out vec3 bitang;



void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    //gl_Position = light.proj * light.view * ubo.model * vec4(inPosition, 1.0);

    surfaceNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    
    position = vec3(ubo.model * vec4(inPosition, 1.0));
    cameraPosition = transpose(ubo.cameraTrans)[3].xyz;
    outColor = inColor;

    tang =  mat3(transpose(inverse(ubo.model))) * inTangent.xyz;
    bitang = cross(normalize(inTangent.xyz), normalize(surfaceNormal)) * inTangent.w;

}