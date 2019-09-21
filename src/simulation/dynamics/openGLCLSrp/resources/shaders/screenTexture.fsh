#version 410 core

in vec2 UV;

//out vec3 color;
layout(location = 0) out vec4 color;

uniform sampler2D renderedTexture;

void main(){
    
    vec4 tmp_color = texture(renderedTexture, UV);
    color = vec4(tmp_color.x, tmp_color.y, tmp_color.z, 1.0);
}
