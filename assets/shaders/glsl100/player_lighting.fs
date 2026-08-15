#version 100
precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 ambientColor;
uniform float brightness;
uniform float warmth;

void main()
{
    vec4 texel = texture2D(texture0, fragTexCoord);
    if (texel.a < 0.05) discard;

    vec3 col = texel.rgb * colDiffuse.rgb * fragColor.rgb;
    
    col.r *= (1.0 + (warmth - 1.0) * 0.12);
    col.g *= (1.0 + (warmth - 1.0) * 0.05);
    col.b *= (1.0 - (warmth - 1.0) * 0.10);

    col = col * ambientColor * brightness;

    gl_FragColor = vec4(clamp(col, 0.0, 1.0), texel.a * colDiffuse.a * fragColor.a);
}
