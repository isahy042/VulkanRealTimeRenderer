#version 450

//layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;

layout (binding = 1) uniform samplerCube specularTexture;
layout (binding = 2) uniform samplerCube albedoTexture;
layout (binding = 3) uniform samplerCube unusedTexture1;
layout (binding = 4) uniform samplerCube unusedTexture2;
layout (binding = 5) uniform samplerCube normalMap;
layout (binding = 6) uniform samplerCube displacementMap;
layout (binding = 7) uniform sampler2D texSampler;

layout(binding = 8) uniform LightObject1 {
    mat4 model;
    mat4 data;
    mat4 proj;
    mat4 view;
} sphereLight[10];

layout(binding = 9) uniform LightObject2 {
    mat4 model;
    mat4 data;
    mat4 proj;
    mat4 view;
} spotLight[10];

layout(binding = 10) uniform LightObject3 {
    mat4 model;
    mat4 data;
    mat4 proj;
    mat4 view;
} sunLight[1];

layout (binding = 11) uniform sampler2D sphereShadow[10];
layout (binding = 12) uniform sampler2D spotShadow[10];
layout (binding = 13) uniform sampler2D sunShadow[1];

layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 cameraPosition;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

layout(location = 0) out vec4 outColor;

vec3 rgbe2rgb(vec4 rgbe) {
    // Extract exponent
    if (rgbe.x + rgbe.y + rgbe.z + rgbe.w == 0) return vec3(0.0);

    rgbe = rgbe * 255.0;
    float exponent = rgbe.w;
    // Convert RGBE to linear RGB' using the specified formula
    return exp2(exponent - 128.0) * ((rgbe.xyz + 0.5)/256);
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

vec4 gammaEncode(vec4 color) {
    return vec4(gammaEncode(color.x), gammaEncode(color.y), gammaEncode(color.z), gammaEncode(color.w));
}

// https://scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/shading-multiple-lights.html was very helpful
vec3 getSphereLight(vec3 normal){

    vec3 color = vec3(0.);
    mat4 light;

    // iterate through lights, break out early if possible
    for (int i = 0; i < 10; i++){
        
        light = sphereLight[i].data;
        if (light[0][0]<1) {
            break;
        } else if (light[0][0]>1){
            continue;
        }

        // check if in range of illumination
        float limit = light[1].w;        
        vec3 lightPos = vec3(sphereLight[i].model * vec4(0.0,0.0,0.0,1.0));
        float d = length(position - lightPos);
        float cutoff = 1 - pow(d/limit, 4.0);
        if (cutoff <= 0) {
            continue;
        }

        // get light specs
        vec3 tint = light[0].yzw;
        float shadow = light[1].x;
        float radius = light[1].y;
        float power = light[1].z;

        // determine whether the normal hits the light.
        // logic of the code (for this and all sphere intersection in shader light processing)
        // from https://kylehalladay.com/blog/tutorial/math/2013/12/24/Ray-Sphere-Intersection.htm
        // AND PART OF IT WAS WRONG :(
        vec3 l = lightPos-position;
        float lengthOfN = dot(l, normalize(normal));
        float len = sqrt((length(l)*length(l)) - (lengthOfN*lengthOfN));
        bool intersect = ((lengthOfN >= 0.0) && (len <= radius) && dot(normalize(normal), normalize(l)) > 0);

        if (intersect){
            color += tint*(power/(4*3.1415*d*d))*cutoff;
        }
        else if ((lengthOfN >= 0.0)){
            color += tint*(power/(4*3.1415*d*d))*dot(normalize(l), normalize(normal))*cutoff;
        }
    }
    return color;
}

vec3 getSpotLight(vec3 normal){
    vec3 color = vec3(0.);
    mat4 light;
    // iterate through lights, break out early if possible
    for (int i = 0; i < 10; i++){
        
        light = spotLight[i].data;
        if (light[0][0]!=1.0) {
            break;
        }

        // check if in range of illumination
        float limit = light[1].w;        
        vec3 lightPos = vec3(spotLight[i].model * vec4(0.0,0.0,0.0,1.0));
        float d = length(position - lightPos);
        float cutoff = 1 - pow(d/limit, 4.0);
        if (cutoff <= 0) {
            continue;
        }

        // get light specs
        vec3 tint = light[0].yzw;
        float shadow = light[1].x;
        float radius = light[1].y;
        float power = light[1].z;
        float fov = light[2].x;
        float blend = light[2].y;
        vec3 lightDirection = light[3].xyz;

        // determine the fov.
        vec3 l = lightPos - position;
        vec3 pointOnSphere = lightPos - (normalize(l) * radius);
        // get fov with the closest point on the sphere
        float currentFov = acos(dot(-normalize(pointOnSphere-position), normalize(lightDirection)));
        
        vec4 lightSpaceLocation = spotLight[i].proj * spotLight[i].view * vec4(position, 1.0);
        lightSpaceLocation /= lightSpaceLocation.w;
        vec2 sampleDepthAt = vec2(lightSpaceLocation.x + 1, lightSpaceLocation.y + 1) / 2;

        float depth = gammaEncode(texture(spotShadow[i], sampleDepthAt)).x;
        // PCF
        float neighborDepth = 4.0;
        float shadowIncrec = 1/shadow;
        float shadowOffset = 0.01;
        float currentDepth = length(l)/limit;
        if (dot(normalize(l), normalize(normal))<0){// 
             continue;
        }

        if (gammaEncode(texture(spotShadow[i], vec2(sampleDepthAt.x + shadowIncrec, sampleDepthAt.y))).x + shadowOffset < currentDepth) {neighborDepth --;}
        if (gammaEncode(texture(spotShadow[i], vec2(sampleDepthAt.x - shadowIncrec, sampleDepthAt.y))).x + shadowOffset < currentDepth) {neighborDepth --;}
        if (gammaEncode(texture(spotShadow[i], vec2(sampleDepthAt.x, sampleDepthAt.y + shadowIncrec))).x + shadowOffset < currentDepth) {neighborDepth --;}
        if (gammaEncode(texture(spotShadow[i], vec2(sampleDepthAt.x, sampleDepthAt.y - shadowIncrec))).x + shadowOffset < currentDepth) {neighborDepth --;}
        cutoff *= neighborDepth/4.0;

        // we will hit the object. assign color.
        if (fov/2 >= currentFov){
            // update the cutoff adjustment factor identical to sphere light
            float lengthOfN = dot(l, normalize(normal));
            float len = sqrt((length(l)*length(l)) - (lengthOfN*lengthOfN));
            bool intersect = ((lengthOfN >= 0.0) && (len <= radius));
            if (!intersect){
                cutoff *= dot(normalize(l), normalize(normal));
            }

            // if fully illuminated 
            // colors are scaled both by whether the normal points straight to the light, and where it is in the FOV.
            if (fov * (1-blend)/2 >= currentFov){
                color += tint*(power/(4*3.1415*d*d))*cutoff;
            } else{
                float blendingFactor = (currentFov - (fov * (1-blend)/2))/((fov/2)-(fov * (1-blend)/2));
                color += tint*(power/(4*3.1415*d*d))*cutoff*(1-blendingFactor);
            }
        }
    }
    return color;
}

vec3 getSunLight(vec3 normal){
    vec3 color = vec3(0.);
    mat4 light;
    // iterate through lights, break out early if possible
    for (int i = 0; i < 1; i++){
        
        light = sunLight[i].data;
        if (light[0][0]<1) {
            break;
        } else if (light[0][0]>1){
            continue;
        }

        // we assume sun is infinity away
        // get light specs
        vec3 tint = light[0].yzw;
        float shadow = light[1].x;
        float angle = light[1].y;
        float strength = light[1].z;
        vec3 lightDirection = light[2].xyz;

        // determine the fov.
        // if (acos(dot(normalize(normal), -normalize(lightDirection)))<=angle){
        //     color += strength * tint;
        // }
        float ndotl = dot(normalize(normal), -normalize(lightDirection));
        if (ndotl > 0){
            color += strength * tint * ndotl;
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

    vec3 albedo = rgbe2rgb(texture(albedoTexture, newNormal));
    vec3 c = rgbe2rgb(texture(specularTexture, newNormal)) * albedo;


    outColor = vec4(toneMapReinhard(c+(albedo*(getSphereLight(newNormal)+getSpotLight(newNormal)+getSunLight(newNormal)))), 1);
}

