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
    return (c1*f)+(c2*(1.0-f));
}

float mixVal(float v1, float v2, float f){
    return (v1*f)+(v2*(1.0-f));
}

float thunder(){ // a silly thunder modeling function, very much hardcoded.
    float f = mod(cloud.data[0].w, 200);
    
    if (f<150){
        return 0;
    }
    f -= 150;
    if (f<5){
        return 0.5+ f/2;
    }else if (f<10){
        return 0.5+(10-f)/2;
    }
    f -= 10;
    if (f<5){
        return 0.5+ f/2;
    }else if (f<10){
        return 0.5+(10-f)/2;
    }
    return 0;
}


vec2 intersectCube(vec3 pos, vec3 ray, vec3 bbmin, vec3 bbmax){
    ray = normalize(ray);
    float tmin = 1000;
    float tmax = 0;

    vec3 p1 = (bbmin - pos) /ray;
    vec3 p2 = (bbmax - pos) /ray;

    vec3 r = vec3(0);

    if (pos.y<=bbmax.y && pos.y >= bbmin.y && pos.z<=bbmax.z && pos.z >= bbmin.z && pos.x<=bbmax.x && pos.x >= bbmin.x){
        tmin = 0;
    }

    if (!isnan(p1.x) && p1.x >=0){
        r = pos + (p1.x * ray);
        if (r.y<=bbmax.y && r.y >= bbmin.y && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p1.x);
            tmax = max(tmax, p1.x);
        }
    }

    if (!isnan(p2.x) && p2.x >=0){
        r = pos + (p2.x * ray);
        if (r.y<=bbmax.y && r.y >= bbmin.y && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p2.x);
            tmax = max(tmax, p2.x);
        }
    }

    if (!isnan(p1.y) && p1.y >=0){
        r = pos + (p1.y * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p1.y);
            tmax = max(tmax, p1.y);
        }
    }

    if (!isnan(p2.y) && p2.y>=0){
        r = pos + (p2.y * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.z<=bbmax.z && r.z >= bbmin.z){
            tmin = min(tmin, p2.y);
            tmax = max(tmax, p2.y);
        }
    }

    if (!isnan(p1.z) && p1.z >=0){
        r = pos + (p1.z * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.y<=bbmax.y && r.y >= bbmin.y){
            tmin = min(tmin, p1.z);
            tmax = max(tmax, p1.z);
        }
    }

    if (!isnan(p2.z) &&p2.z >=0){
        r = pos + (p2.z * ray);
        if (r.x<=bbmax.x && r.x >= bbmin.x && r.y<=bbmax.y && r.y >= bbmin.y){
            tmin = min(tmin, p2.z);
            tmax = max(tmax, p2.z);
        }
    }

    return vec2(tmin, tmax);

}


float clip(float f, float minf, float maxf){
    if (f <= minf){
        return minf+0.0001;
    }
    else if (f >= maxf){
        return maxf-0.0001;
    }
    return f;
}

vec2 clip(vec2 v,  float minf, float maxf ){
    return vec2(clip(v.x, minf, maxf), clip(v.y, minf, maxf));
}

vec4 sampleField(vec2 sampleAt, float layer){
    vec4 n1 = gammaEncode(texture(cloudField[int(floor(layer))], sampleAt));
    vec4 n2 = gammaEncode(texture(cloudField[int(ceil(layer))], sampleAt));
    return mixColor(n2, n1, layer-floor(layer));
}

vec4 sampleModel(vec2 sampleAt, float layer){
    vec4 n1 = gammaEncode(texture(cloudModel[int(floor(layer))], sampleAt));
    vec4 n2 = gammaEncode(texture(cloudModel[int(ceil(layer))], sampleAt));
    return mixColor(n2, n1, layer-floor(layer));
}

vec4 sampleNoise(vec2 sampleAt, float layer){
    sampleAt.x += (cloud.data[0].w/800); // adding wind scroll
    sampleAt.x = clip(sampleAt.x, 0.0,1.0);
    vec4 n1 = gammaEncode(texture(cloudNoise[int(floor(layer))], sampleAt));
    vec4 n2 = gammaEncode(texture(cloudNoise[int(ceil(layer))], sampleAt));
    return mixColor(n2, n1, layer-floor(layer));
}


void main() {
    float stepsize = 2;
    // get vector from viewing angle to cloud
    float epsi = 0.001;
    vec3 ray = normalize(position - cameraPosition);

    vec3 bbmin = cloud.data[0].xyz;
    vec3 bbmax = cloud.data[1].xyz;
    vec3 grid = cloud.data[2].xyz;
    float size = cloud.data[3].x;
    vec3 bbsize = grid * size;
    float thunderMode = cloud.data[1].w;

    vec3 cloudColor = cloud.data[3].yzw;
    vec4 skyColor = vec4(0.3, 0.3, 0.6, 1);
    vec3 lightDir = -normalize(sunLight[0].data[2].xyz); // a normalized light dir vector
    vec3 lightColor = sunLight[0].data[0].yzw;
    vec3 innerColor = sphereLight[0].data[0].yzw;
    bool debug = false;
    vec2 marchrange = intersectCube(cameraPosition, ray, bbmin, bbmax);
    outColor = vec4(0,0,0,1);

    float distanceFromPixel = length(position - cameraPosition);

    if (marchrange.x > marchrange.y){
        outColor = skyColor;
    } else {// ray march
        float inc = marchrange.x;
        float stepSize = size*stepsize;//max(abs(marchrange.x - marchrange.y)/10, 0.0001);
        float stepSizeLight = size*stepsize;

        float light = 0;
        float light2 = 0;
        float trans = 1;
        float alpha = 0;

        bool secondaryLight = false;

        float cloudDistance = 10;
        float adaptiveStepSize = stepSize;

        if (marchrange.y > 1000 || marchrange.y < 0 || isnan(marchrange.y)){
            debug = true;
        }
        if (marchrange.x > 1000 || marchrange.x < 0 || isnan(marchrange.x)){
            debug = true;
        }

        while (inc < marchrange.y && inc < distanceFromPixel){ // we are in the cloud's bounding box now. 
            vec3 newPos = cameraPosition + (ray * inc);
            float layer = clip((newPos.y-bbmin.y)/bbsize.y * 63, 0.0, 63.0);

            // light marching
            vec2 marchLight = intersectCube(newPos, lightDir, bbmin, bbmax);

            if (marchLight.y > 1000 || marchLight.y < 0 || isnan(marchLight.y)){
                debug = true;
            }
            if (marchLight.x > 1000 || marchLight.x < 0 || isnan(marchLight.x)){
                debug = true;
            }

            float transLight = 1;
            float trans2Light = 1;
            float incLight = 0.05;

            //check if box intersect returs inifnity
             while (incLight < marchLight.y && trans > epsi){ // a ray towards the light
                vec3 newPosLight = newPos + (lightDir * incLight);
                vec2 sampleLightat = vec2((newPosLight.x-bbmin.x)/bbsize.x, (newPosLight.z-bbmin.z)/bbsize.z);
                sampleLightat = clip(sampleLightat, 0.0, 1.0);
                float lightlayer = clip((newPosLight.y-bbmin.y)/bbsize.y * 63, 0.0, 63.0);

                vec4 l = sampleModel(sampleLightat, lightlayer); // get density
                
                transLight *= (1-l.x); // the light's transmittance is weakened by the density of cloud at a location

                if (transLight < 0.01){
                    break;
                }
                incLight += stepSizeLight;
            }

            float thunderIndex = 1.0;
            float wind =  mod(cloud.data[0].w, 200);
            if (wind >= 150 && wind <= 160)
            {
                thunderIndex = 0;
            }
            else if (wind >= 160 && wind <= 170){
                thunderIndex = 7;
            }

            // if has thunder, currently has thunder, trans>epsi
            if (thunderMode > 0 && thunderIndex != 1.0 && trans > epsi){ // attempting to render secondary light.  
                secondaryLight = true;
                vec3 lightPos = vec3(0,thunderIndex,0);
                vec3 light2Dir = normalize(lightPos-newPos);
                float march2Light = length(lightPos-newPos);

                if (march2Light> 1000 || march2Light < 0 || isnan(march2Light)){
                    debug = true;
                }

                incLight = 0.05;
                // shoot a ray into the middle?
                while (incLight < march2Light && trans > epsi){ // a ray towards the light
                    vec3 newPosLight = newPos + (light2Dir * incLight);
                    vec2 sampleLightat = vec2((newPosLight.x-bbmin.x)/bbsize.x, (newPosLight.z-bbmin.z)/bbsize.z);
                    sampleLightat = clip(sampleLightat, 0.0, 1.0);
                    float lightlayer = clip((newPosLight.y-bbmin.y)/bbsize.y * 63, 0.0, 63.0);
                    vec4 l = sampleModel(sampleLightat, lightlayer); // get density
                    
                    trans2Light *= (1-l.x); // the light's transmittance is weakened by the density of cloud at a location
                    if (trans2Light < 0.01){
                        break;
                    }
                    incLight += stepSizeLight;
                }
            } 

           // layer = clip(layer, 30.0, 32.0);

            vec2 sampleat = vec2((newPos.x -bbmin.x)/bbsize.x, (newPos.z-bbmin.z)/bbsize.z);
            sampleat = clip(sampleat,0.0,1.0);
            vec4 c = sampleModel(sampleat, layer);
            vec4 n = sampleNoise(sampleat, layer);
            // blending noise, first blending frequency 
            float noiseAtt = mixVal(mixVal(n.x, n.z, c.y), mixVal(n.y, n.w, c.y), c.z); // 0-1 range with 0=wispy and 1=billowy

            if (c.x*noiseAtt > epsi){
                
                alpha += c.x * noiseAtt; // sample density
                // TOGGLE  LIGHT
                if (trans > 0.01 && transLight > 0.01){
                    light += (trans * transLight) * c.x * noiseAtt; 
                }
                light2 += (trans * trans2Light) * c.x * noiseAtt;
                
                // the total light at the current location is adjusted by the current 
                // transmittance (how much is the current location visible) and the total
                // light that arrives here. hence adding
                trans *= (1-(c.x * noiseAtt));

                // TOGGLE adaptive step size
                //adaptiveStepSize *= 1.01;// max(size , max(sqrt(inc), epsi) * 0.08);
                cloudDistance = (sampleField(sampleat, layer).x * 0.2 - 0.02);//4352 - 256); // decoding sdf data
                cloudDistance *= bbsize.x;
                stepSize = adaptiveStepSize;//max(cloudDistance, adaptiveStepSize) ;//+ jitter;

            }
            inc += stepSize;
        }

        vec3 finalColor = (light * lightColor) + cloudColor; // no thunder

        if (thunderMode== 1.0){// thunder mode 1
            float f = mod(cloud.data[0].w, 200);

            finalColor *= min(1, max(0.5, (225-f)/150));
             
            if (secondaryLight){
                light2 *= thunder();
                finalColor += (light2 * innerColor);
            }
        }

        outColor = vec4(finalColor,alpha);

        if (debug){
            outColor = vec4(1,0,0,1);
        }

    }
   
    
}

