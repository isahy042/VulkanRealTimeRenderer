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

layout(binding = 8) uniform LightObject1 {
    mat4 model;
    mat4 data;
} sphereLight[10];

layout(binding = 9) uniform LightObject2 {
    mat4 model;
    mat4 data;
} spotLight[10];

layout(binding = 10) uniform LightObject3 {
    mat4 model;
    mat4 data;
} sunLight[5];


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

vec3 getSphereLight(vec3 normal){

    vec3 color = vec3(0.);
    mat4 light;

    // iterate through ights, break out early if possible
    for (int i = 0; i < 10; i++){
        
        light = sphereLight[i].data;
        if (light[0][0]<1) {
            break;
        }

        // check if in range of illumination
        float limit = light[1].w;        
        vec3 lightPos = vec3(sphereLight[i].model * vec4(0.0,0.0,0.0,1.0));
        float d = length(position - lightPos);

        if (1 - pow(d/limit, 4.0) <= 0) {
            continue;
        }

        // get light specs
        vec3 tint = light[0].yzw;
        float shadow = light[1].x;
        float radius = light[1].y;
        float power = light[1].z;

        // determine whether the normal hits the light.
        // part of the code from https://kylehalladay.com/blog/tutorial/math/2013/12/24/Ray-Sphere-Intersection.html
        vec3 l = lightPos-position;
        float lengthOfN = dot(l, normalize(normal));
        float len = sqrt((lengthOfN*lengthOfN) - (length(l)*length(l)));
        bool intersect = ((lengthOfN >= 0.0) && (len <= radius));

        if (intersect){
            color += tint*(power/(4*3.1415*d*d));
        }
        else{
            color += tint*(power/(4*3.1415*d*d))*dot(normalize(l), normalize(normal));
        }
    }
    return color;
}

void main() {
    vec3 normalColor = (gammaEncode(texture(normalMap, surfaceNormal).xyz) - 0.5) * 2;
    vec3 newNormal = vec3(
        (normalColor.x * tang.x) + (normalColor.y * bitang.x) + (normalColor.z * surfaceNormal.x), 
        (normalColor.x * tang.y) + (normalColor.y * bitang.y) +(normalColor.z * surfaceNormal.y),
        (normalColor.x * tang.z) + (normalColor.y * bitang.z) +(normalColor.z * surfaceNormal.z));


    vec3 I = position - cameraPosition;
    vec3 R = reflect(normalize(I), normalize(newNormal));

    vec3 baseColor = rgbe2rgb(texture(cubeMapTexture, R));

    outColor = toneMapReinhard(baseColor + getSphereLight(R));

}

// sources: 
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
