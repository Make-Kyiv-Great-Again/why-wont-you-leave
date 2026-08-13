#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float progress;
uniform vec2 resolution;

out vec4 finalColor;

void main() {
    float p = progress < 0.5 ? progress : 1.0 - progress;
    float maxPixelSize = max(resolution.x / 20.0, 32.0);
    float pixelSize = mix(1.0, maxPixelSize, p * 2.0);
    
    vec2 dx = vec2(pixelSize / resolution.x, pixelSize / resolution.y);
    vec2 coord = dx * floor(fragTexCoord / dx) + (dx / 2.0);
    
    finalColor = texture(texture0, coord) * fragColor;
}
