#version 450

//layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 cameraTrans;
} ubo;
layout (binding = 1) uniform samplerCube albedoTexture;
layout (binding = 2) uniform samplerCube roughnessTexture;
layout (binding = 3) uniform samplerCube metalnessTexture;
layout (binding = 4) uniform samplerCube mipmapTexture;
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
} sunLight[5];

layout (binding = 11) uniform sampler2D sphereShadow[10];
layout (binding = 12) uniform sampler2D spotShadow[10];
layout (binding = 13) uniform sampler2D sunShadow[5];

layout(location = 0) in vec3 surfaceNormal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 cameraPosition;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

layout(location = 0) out vec3 outColor;


// Apply Reinhard tone mapping operator
vec3 toneMapReinhard(vec3 linearRGB) {
    return linearRGB;
    float exposure = 1.0;
    return linearRGB / (linearRGB + vec3(1)) * exposure;
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

vec3 rgbe2rgb(vec4 rgbe) {
    // Extract exponent
    if (rgbe.x + rgbe.y + rgbe.z + rgbe.w == 0) return vec3(0.0);

    rgbe = rgbe * 255;
    int exponent = 128;//int(floor(rgbe.w));
    // Convert RGBE to linear RGB' using the specified formula
    return exp2(exponent - 128) * ((rgbe.xyz+ 0.5)/256);
}

// functions for calculating BRDF
//https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#appendix-b-brdf-implementation

float mixBRDF(float brdf1, float brdf2, float p){
    return (p*brdf1) + ((1-p)*brdf2);
}

vec3 mixColor(vec3 c1, vec3 c2, float p){
    return (p*c1) +((1-p)*c2);
}

// dielectric reflect, metallic reflect and attenuate, diffuse attenuate
vec3 getSphereLight(vec3 normal, vec3 reflected, vec3 albedo, float roughness,float metalness, vec2 envBRDF){
    vec3 color = vec3(0.);;
    vec3 attenuation = vec3(0.);
    vec3 specColor = vec3(0.);

    mat4 light;
    float alpha = roughness* roughness;

    // iterate through lights, break out early if possible
    for (int i = 0; i < 10; i++){

        // // check if in range of illumination
        // float limit = light[1].w;        
        // vec3 lightPos = vec3(sphereLight[i].model * vec4(0.0,0.0,0.0,1.0));
        // float d = length(position - lightPos);
        // float cutoff = 1 - pow(d/limit, 4.0);

        // vec3 l = lightPos-position;

        // if (cutoff <= 0 || dot(normalize(l), normalize(normal)) < 0) { // or, if in shadow
        //     continue;
        // }

        // // get light specs
        // vec3 tint = light[0].yzw;
        // float shadow = light[1].x;
        // float radius = light[1].y;
        // float power = light[1].z;

        // // determine whether the normal hits the light.
        // // lambertian portion
        
        // float lengthOfN = dot(l, normalize(normal));
        // float len = sqrt((length(l)*length(l)) - (lengthOfN*lengthOfN));
        // bool intersect = ((lengthOfN >= 0.0) && (len <= radius));
        // if (intersect){
        //     attenuation += tint * (power/(4*3.1415*d*d)) * cutoff * dot(normalize(l), normalize(normal))* (1/(3.1415926 * alpha * alpha));
        // }

        // // specular portion
        // // get closest point on sphere.
        // vec3 pointOnSphere = lightPos - (normalize(l) * radius);
        // // dot with the closest point on the sphere
        // float NoV = clamp(dot(normalize(pointOnSphere-position), reflected), -1.0, 0.99);
        // if (NoV<0) { continue;}
        // float specular = gammaEncode(texture(texSampler, vec2(roughness, NoV)).x);
        // specColor += specular * power * tint;

        light = sphereLight[i].data;
        if (light[0][0]<1) {
            break;
        }

        // check if in range of illumination
        float limit = light[1].w;        
        vec3 lightPos = vec3(sphereLight[i].model * vec4(0.0,0.0,0.0,1.0));
        float d = length(position - lightPos);
        float cutoff = 1 - pow(d/limit, 4.0);

        vec3 l = lightPos-position;

        if (cutoff <= 0 || dot(normalize(l), normalize(normal)) < 0) { // too far away or facing the other side
            continue;
        }

        // get light specs
        vec3 tint = light[0].yzw;
        float shadow = light[1].x;
        float radius = light[1].y;
        float power = light[1].z;

        // determine whether the normal hits the light.
        // lambertian portion
        float lengthOfN = dot(l, normalize(normal));
        float len = sqrt((length(l)*length(l)) - (lengthOfN*lengthOfN));
        bool intersect = ((lengthOfN >= 0.0) && (len <= radius) && dot(normalize(normal), normalize(l)) > 0);
        if (intersect){
            attenuation = tint*(power/(4*3.1415*d*d))*cutoff;
        }
        else if ((lengthOfN >= 0.0)){
            attenuation = tint*(power/(4*3.1415*d*d))*dot(normalize(l), normalize(normal))*cutoff;
        }

        // specular portion
        if (dot(normalize(l), normalize(reflected)) >= 0){
        // get closest point on sphere.
        vec3 centerToRay = dot(l, reflected)*reflected - l;
        vec3 closestPoint = l + centerToRay * clamp(radius/length(centerToRay),0.0,1.0);
        float phis = atan(radius/d);
        float phir = acos(dot(normalize(reflected),normalize(l)));

        float norm = alpha / clamp(alpha + radius/(2*d),0.01,1.0) ;
        norm = 1/norm;
        //norm = norm * norm;
        //return vec3(phis/3.1415,0,0);
        if (phir<phis){
            specColor = tint * norm ;///* (power+2) / (2*3.1415926);
        }else{
            specColor = tint *  norm *  pow(cos(phir-phis), power) / (2*3.1415926);//(power+2) *
        }
        }
       attenuation *= albedo;
 
       // mix attenuation and specular color based on metalness
        color += mixColor(attenuation*albedo, specColor, roughness);
       }
    return color;
}




vec3 getSpotLight(vec3 normal, vec3 reflected, vec3 albedo,float roughness){
    vec3 attenuation = vec3(0.);
    vec3 specColor = vec3(0.);

    mat4 light;
    float alpha = roughness * roughness;

    // iterate through lights, break out early if possible
    for (int i = 0; i < 10; i++){
        light = spotLight[i].data;
        if (light[0][0]<1) {
            break;
        }

        // check if in range of illumination
        float limit = light[1].w;        
        vec3 lightPos = vec3(spotLight[i].model * vec4(0.0,0.0,0.0,1.0));
        float d = length(position - lightPos);
        if (d-limit >=0) {
            continue;
        }
        float cutoff = 1 - pow(d/limit, 4.0);

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
        vec2 sampleDepthAt = vec2(lightSpaceLocation.x + 1, 1 - lightSpaceLocation.y) / 2;
        float depth = gammaEncode(texture(spotShadow[i], sampleDepthAt)).x; // this may need to be gamma encoded?

        if (length(l)/limit > depth + 0.05 || dot(normalize(-lightDirection), normalize(normal))<0 || dot(normalize(l), normalize(normal)) < 0){// pass if in shadow or opposite side of light
            continue;
        }

        // we will hit the object. assign color.
        if (fov/2 >= currentFov){
            cutoff *= 40 * abs(dot(normalize(normal), -normalize(lightDirection)));//(1/(3.1415926 * alpha * alpha)); // * abs(dot(normalize(l), normalize(reflected)))
            if (fov * (1-blend)/2 >= currentFov){
                attenuation += tint*(power/(4*3.1415*d*d))*cutoff;
            } else{
                float blendingFactor = (currentFov - (fov * (1-blend)/2))/((fov/2)-(fov * (1-blend)/2));
                attenuation += tint*(power/(4*3.1415*d*d))*cutoff*(1-blendingFactor);
            }


            // specular case
            // dot with the closest point on the sphere
            float lengthOfN = abs(dot(l, normalize( reflected)));
            float len = sqrt((length(l)*length(l)) - (lengthOfN*lengthOfN));
            bool intersect = ((lengthOfN >= 0.0) && (len <= radius));
            if (!intersect) { continue;}
            float NoV =  clamp(dot(normalize(pointOnSphere-position), reflected), 0.01, 0.99);
            float specular = gammaEncode(texture(texSampler, vec2(roughness, NoV)).x);
            specColor +=  power * tint * (1/(3.1415*alpha*alpha));
        }



    }
    return (attenuation*albedo) + specColor;
}

vec3 getSunLight(vec3 normal, vec3 reflected, vec3 albedo,float alpha){
    vec3 color = vec3(0.);
    mat4 light;
    // iterate through lights, break out early if possible
    for (int i = 0; i < 10; i++){
        
        light = sunLight[i].data;
        if (light[0][0]<1) {
            break;
        }

        // we assume sun is infinity away
        // get light specs
        vec3 tint = light[0].yzw;
        float shadow = light[1].x;
        float angle = light[1].y;
        float strength = light[1].z;
        vec3 lightDirection = light[2].xyz;

        // TODO: change this.
        if (acos(dot(normalize(normal), -normalize(lightDirection)))<=angle/2){
            color += strength * tint;
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

    float roughness = gammaEncode(texture(roughnessTexture, newNormal).xyz).x;
    float metalness = gammaEncode(texture(metalnessTexture, newNormal).xyz).x;
    vec3 albedo =  rgbe2rgb(texture(albedoTexture, newNormal));

    vec3 I = position - cameraPosition;

    float NoV = clamp(dot(normalize(newNormal), normalize(-I)), 0.01, 0.99); //cosine theta
    vec3 R = reflect(normalize(I), normalize(newNormal));

    vec3 prefilteredColor = rgbe2rgb(textureLod(mipmapTexture, R, roughness/0.2));

    vec2 EnvBRDF = gammaEncode(texture(texSampler, vec2(roughness,  NoV))).xy;//*gammaEncode(texture(texSampler, vec2(0,  NoV)).xyz).xy;
    vec3 c = (vec3(1) * EnvBRDF.x) + EnvBRDF.y;

    vec3 metal_brdf = prefilteredColor * c;

    vec3 dielectric_brdf = mixColor(albedo/3.14159, prefilteredColor,
       1- ((0.04 * EnvBRDF.x) + EnvBRDF.y));

    outColor = toneMapReinhard(mixColor(metal_brdf, dielectric_brdf, metalness) + getSphereLight(newNormal, R, albedo, roughness, metalness, EnvBRDF));//+ getSphereLight(newNormal, R, albedo, roughness) +
   // getSpotLight(newNormal, R, albedo, roughness) + getSunLight(newNormal, R, albedo, roughness));
}
