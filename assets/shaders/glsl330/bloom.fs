#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

const float intensity = 1.5;
const float blurSize = 2.0 / 512.0; // Adjust spread of the glow

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);
    vec4 glowColor = vec4(0.0);
    
    int radius = 5;
    float weightSum = 0.0;

    // Apply a weighted blur to create a smooth circular glow
    for(int i = -radius; i <= radius; i++) {
        for(int j = -radius; j <= radius; j++) {
            vec2 offset = vec2(float(i), float(j)) * blurSize;
            
            // Calculate distance for a soft, fall-off weight
            float dist = length(vec2(float(i), float(j)));
            float weight = max(0.0, (float(radius) - dist) / float(radius));
            
            glowColor += texture(texture0, fragTexCoord + offset) * weight;
            weightSum += weight;
        }
    }
    
    glowColor = glowColor / weightSum;
    
    // Combine base image with the smooth glow
    finalColor = (texColor + glowColor * intensity) * colDiffuse * fragColor;
}
