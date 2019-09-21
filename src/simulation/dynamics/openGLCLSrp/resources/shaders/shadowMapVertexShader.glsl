#version 410
                                                                                    
layout(location = 0) in dvec3 Position;
layout(location = 2) in dvec3 Normal;
layout(location = 4) in dvec2 TexCoord;
                                                                                    
uniform mat4 gWVP;

out vec2 TexCoordOut;

void main()
{                                                                                   
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoordOut = vec2(TexCoord);
}
