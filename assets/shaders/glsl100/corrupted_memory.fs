#version 100
precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;

void main()
{
    vec4 texel = texture2D(texture0, fragTexCoord);
    if (texel.a < 0.05) discard;

    float gray = dot(texel.rgb, vec3(0.299, 0.587, 0.114));
    vec3 darkShadow = vec3(gray * 0.44, gray * 0.32, gray * 0.38);
    vec3 originalMuted = texel.rgb * 0.48;
    vec3 baseCol = mix(originalMuted, darkShadow, 0.80);
    
    float pulse = 0.88 + 0.12 * sin(time * 3.0);
    float scanline = 0.92 + 0.08 * sin(fragTexCoord.y * 100.0 + time * 5.0);
    vec3 result = baseCol * pulse * scanline;

    gl_FragColor = vec4(clamp(result, 0.0, 1.0), texel.a * colDiffuse.a * fragColor.a);
}
