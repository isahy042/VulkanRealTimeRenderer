#version 450

layout(location = 0) in vec3 surfaceNormal;

layout(location = 0) out vec4 outColor;

void main() {
    // Normalize the surface normal
    vec3 normalizedNormal = normalize(surfaceNormal);

    // Map the components of the normalized normal to the range [0, 1]
    vec3 color = (normalizedNormal + 1.0) * 0.5;

    // Assign grayscale color (average of RGB components)
    float grayscale = (color.r + color.g + color.b) / 3.0;

    // Output the grayscale color
    fragColor = vec4(grayscale, grayscale, grayscale, 1.0);
}
