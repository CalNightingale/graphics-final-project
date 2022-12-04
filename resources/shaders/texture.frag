#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 texLoc;
// Task 8: Add a sampler2D uniform
uniform sampler2D tex;
// Task 29: Add a bool on whether or not to filter the texture
uniform bool invert;
uniform bool blur;

uniform int width;
uniform int height;

out vec4 fragColor;

void main()
{
    fragColor = vec4(1);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(tex, texLoc);
    //fragColor = vec4(texLoc[0], texLoc[1], 1, 1); // uncomment for testing; should show a 2D gradient

    // Handle blurring
    if (blur) {
        float uIncrement = 1.0 / width;
        float vIncrement = 1.0 / height;
        vec4 colorAccumulator = vec4(0);
        for (int x = -2; x < 3; x++) {
            for (int y = -2; y < 3; y++) {
                colorAccumulator += texture(tex, vec2(texLoc[0] + uIncrement*x, texLoc[1] + vIncrement*y));
            }
        }
        fragColor = (1.0/25) * colorAccumulator;
    }
    // Handle inversion
    if (invert) {
        fragColor = 1 - fragColor;
    }
}
