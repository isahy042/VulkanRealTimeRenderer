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

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 lightPosition;
layout(location = 3) in vec3 lightDirection;
layout(location = 4) in float lightMaxReach;

layout(location = 0) out vec3 outColor;

void main() {
    float d = length(position-lightPosition);
    if (d<lightMaxReach){
        outColor = vec3(d/lightMaxReach);
    }else{
        outColor = vec3(0,0,0);
    }
}

