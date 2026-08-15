#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Custom lighting uniforms
uniform vec3 ambientColor;  // Muted ambient tone
uniform float brightness;   // Shading level (< 1.0 for ambient shadows)
uniform float warmth;       // Subtle color temperature

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    if (texel.a < 0.05) discard;

    // Base color from texture and vertex color
    vec3 col = texel.rgb * colDiffuse.rgb * fragColor.rgb;
    
    // Subtle warmth tint (no neon yellowing)
    col.r *= (1.0 + (warmth - 1.0) * 0.12);
    col.g *= (1.0 + (warmth - 1.0) * 0.05);
    col.b *= (1.0 - (warmth - 1.0) * 0.10);

    // Apply ambient room lighting and shadow shading
    col = col * ambientColor * brightness;

    finalColor = vec4(clamp(col, 0.0, 1.0), texel.a * colDiffuse.a * fragColor.a);
}
