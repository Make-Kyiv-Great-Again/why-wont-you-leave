#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    if (texel.a < 0.05) discard;

    // Convert to desaturated shadow with 20% original color retention
    float gray = dot(texel.rgb, vec3(0.299, 0.587, 0.114));
    
    // Softer dark shadow tone (20% less intensely dark)
    vec3 darkShadow = vec3(gray * 0.44, gray * 0.32, gray * 0.38);
    vec3 originalMuted = texel.rgb * 0.48;
    
    // Blend 80% dark shadow + 20% original muted texture
    vec3 baseCol = mix(originalMuted, darkShadow, 0.80);
    
    // Subtle pulsing shadow
    float pulse = 0.88 + 0.12 * sin(time * 3.0);
    
    // Subtle soft scanline effect
    float scanline = 0.92 + 0.08 * sin(fragTexCoord.y * 100.0 + time * 5.0);
    
    vec3 result = baseCol * pulse * scanline;

    finalColor = vec4(clamp(result, 0.0, 1.0), texel.a * colDiffuse.a * fragColor.a);
}
