#version 450

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
layout (binding = 13) uniform sampler2D sunShadow[5];

layout (binding = 14) uniform sampler2D cloudModel[64];
layout (binding = 15) uniform sampler2D cloudField[64];
layout (binding = 16) uniform sampler2D cloudNoise[64];

layout(binding = 17) uniform cloudObject {
    mat4 data;
} cloud;

layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 cameraPosition;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

layout(location = 0) out vec4 outColor;

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

vec4 mixColor(vec4 c1, vec4 c2, float f){
    return (c1*f)+(c2*(1-f));
}

float mixVal(float v1, float v2, float f){
    return (v1*f)+(v2*(1-f));
}

vec2 intersectCube(vec3 pos, vec3 ray, vec3 bbmin, vec3 bbmax){
    float tmin = 1000;
    float tmax = 0;

    vec3 p1 = (bbmin - pos) /ray;
    vec3 p2 = (bbmax - pos) /ray;

    vec3 r = vec3(0);

    if (pos.y<=bbmax.y && pos.y >= bbmin.y && pos.z<=bbmax.z && pos.z >= bbmin.z && pos.x<=bbmax.x && pos.x >= bbmin.x){
        tmin = 0;
    }

    if (p1.x >=0){
        r = pos + (p1.x * ray);
        if (r.y<=bbmax.y && r.y >= bbmin.y && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p1.x);
            tmax = max(tmax, p1.x);
        }
    }

    if (p2.x >=0){
        r = pos + (p2.x * ray);
        if (r.y<=bbmax.y && r.y >= bbmin.y && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p2.x);
            tmax = max(tmax, p2.x);
        }
    }

    if (p1.y >=0){
        r = pos + (p1.y * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p1.y);
            tmax = max(tmax, p1.y);
        }
    }

    if (p2.y >=0){
        r = pos + (p2.y * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p2.y);
            tmax = max(tmax, p2.y);
        }
    }

    if (p1.z >=0){
        r = pos + (p1.z * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.y<=bbmax.y && r.y >= bbmin.y){
            tmin = min(tmin, p1.z);
            tmax = max(tmax, p1.z);
        }
    }

    if (p2.z >=0){
        r = pos + (p2.z * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.y<=bbmax.y && r.y >= bbmin.y){
            tmin = min(tmin, p2.z);
            tmax = max(tmax, p2.z);
        }
    }

    return vec2(tmin,tmax);

}

void main() {
    // get vector from viewing angle to cloud
    vec3 ray = normalize(position - cameraPosition);

    vec3 bbmin = cloud.data[0].xyz;
    vec3 bbmax = cloud.data[1].xyz;
    vec3 grid = cloud.data[2].xyz;
    float size = cloud.data[3].x;
    vec3 bbsize = grid * size;

    vec3 cloudColor = cloud.data[3].yzw;
    vec4 skyColor = vec4(0, 0.3, 0.6, 1);
    vec3 lightDir = -sunLight[0].data[2].xyz; // a normalized light dir vector
    vec3 lightColor = sunLight[0].data[0].yzw;

    vec2 marchrange = intersectCube(cameraPosition, ray, bbmin, bbmax);

    float sn = 0;
    if (marchrange.x > marchrange.y){
        outColor = skyColor;
    } else {// ray march
        float inc = marchrange.x;
        float stepSize = 0.2;//max(abs(marchrange.x - marchrange.y)/10, 0.0001);
        float stepSizeLight = 0.2;

        float light = 0;
        float trans = 1;
        float alpha = 0;

        while (inc <= marchrange.y){ // we are in the cloud's bounding box now. 
            vec3 newPos = cameraPosition + (ray * inc);
            float layer = (newPos.y-bbmin.y)/bbsize.y * 63;

            // light marching
            vec2 marchLight = intersectCube(newPos, lightDir, bbmin, bbmax);
            float transLight = 1;
            float incLight = 0.1;

            while (incLight <= marchLight.y){ // a ray towards the light
                vec3 newPosLight = newPos + (lightDir * incLight);
                vec2 sampleLightat = vec2((newPosLight.x-bbmin.x)/bbsize.x, (newPosLight.z-bbmin.z)/bbsize.z);
                float lightlayer = (newPosLight.y-bbmin.y)/bbsize.y * 63;
                vec4 l1 = gammaEncode(texture(cloudModel[int(floor(lightlayer))], sampleLightat));
                vec4 l2 = gammaEncode(texture(cloudModel[int(ceil(lightlayer))], sampleLightat));

                vec4 l = mixColor(l2, l1, lightlayer-floor(lightlayer));
                transLight *= (1-l.x); // the light's transmittannce is weakened by the density of cloud at a location
                if (transLight < 0.01){
                    break;
                }
                incLight += stepSizeLight;
            }

            vec2 sampleat = vec2((newPos.x -bbmin.x)/bbsize.x, (newPos.z-bbmin.z)/bbsize.z);
            vec4 c1 = gammaEncode(texture(cloudModel[int(floor(layer))], sampleat));
            vec4 c2 = gammaEncode(texture(cloudModel[int(ceil(layer))], sampleat));
            vec4 c = mixColor(c2,c1,layer-floor(layer));

            sampleat.x += (cloud.data[0].w/400);
            vec4 n1 = gammaEncode(texture(cloudNoise[int(floor(layer))], sampleat));
            vec4 n2 = gammaEncode(texture(cloudNoise[int(ceil(layer))], sampleat));
            vec4 n = mixColor(n2, n1,layer-floor(layer));

            // blending noise, first blending frequency 
            float noiseAtt = mixVal(mixVal(n.x, n.z, c.y), mixVal(n.y, n.w, c.y), c.z); // 0-1 range with 0=wispy and 1=billowy

            alpha += c.x * noiseAtt; // sample density
            light += (trans * transLight) * c.x * noiseAtt; 
            // the total light at the current location is adjusted by the current 
            // transmittance (how much is the current location visible) and the total
            // light that arrives here. hence adding
            trans *= (1-(c.x * noiseAtt));
            inc += stepSize;
            sn++;
        }
        outColor = vec4((light * lightColor) + cloudColor, alpha);
    }
   
    
}

