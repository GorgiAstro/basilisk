#version 330 core

in vec2 TexCoord0;

smooth in vec3 vNormal;
smooth in vec3 vWorldPos;
in mat4 ProjectionMatrix;

uniform sampler2D uDepthTex;
uniform float uWidth;
uniform float uHeight;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec2 res = gl_FragCoord.xy / vec2(uWidth, uHeight);
    FragColor = texture(uDepthTex, res);
}
