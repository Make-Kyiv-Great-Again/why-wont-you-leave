#version 100
precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

const float intensity = 1.5;
const float blurSize = 2.0 / 512.0; // Adjust spread of the glow

void main()
{
    vec4 texColor = texture2D(texture0, fragTexCoord);
    vec4 glowColor = vec4(0.0);
    
    float weightSum = 0.0;

    // Apply a weighted blur to create a smooth circular glow
    for(int i = -5; i <= 5; i++) {
        for(int j = -5; j <= 5; j++) {
            vec2 offset = vec2(float(i), float(j)) * blurSize;
            
            // Calculate distance for a soft, fall-off weight
            float dist = length(vec2(float(i), float(j)));
            float weight = max(0.0, (5.0 - dist) / 5.0);
            
            glowColor += texture2D(texture0, fragTexCoord + offset) * weight;
            weightSum += weight;
        }
    }
    
    glowColor = glowColor / weightSum;
    
    // Combine base image with the smooth glow
    gl_FragColor = (texColor + glowColor * intensity) * colDiffuse * fragColor;
}
