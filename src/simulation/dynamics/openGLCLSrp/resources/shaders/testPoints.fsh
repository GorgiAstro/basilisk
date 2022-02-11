#version 410
uniform sampler2D g_Texture;

in VS_OUT_F {
    vec4 position_clip;
    vec3 position_worldSpace;
    vec3 position_modelSpace;
    vec3 normalDir_cameraSpace;
    vec2 UV;
    vec3 eyeDir_cameraSpace;
    vec3 lightDir_cameraSpace;
} fs_in;

out vec4 fragColor;

void main()
{
    
    //    fragColor = texture(g_Texture, fs_in.UV);
    //    fragColor = vec4(fs_in.UV.x, fs_in.UV.y, 0.0, 1.0);
    fragColor = vec4(0.8, 0.0, 0.0, 1.0);
    //    fragColor = vec4(fs_in.normalDir_cameraSpace.x, fs_in.normalDir_cameraSpace.y, fs_in.normalDir_cameraSpace.z, 1.0);
    
}
