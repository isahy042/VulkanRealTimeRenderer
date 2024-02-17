#version 450

layout(location = 0) in vec3 surfaceNormal;

layout(location = 0) out vec4 outColor;

void main() {
    // Normalize the surface normal
    vec3 normalizedNormal = normalize(surfaceNormal);

    // Map the components of the normalized normal to the range [0, 1]
    vec3 color = (normalizedNormal + 1.0) * 0.5;

    vec3 light = mix(vec3(0,0,0), vec3(1,1,1),
		dot(normalizedNormal, vec3(0,0,1)) * 0.5 + 0.5);
	outColor = vec4(light * color, 1.0);
}
