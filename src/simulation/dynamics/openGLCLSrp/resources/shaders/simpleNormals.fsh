#version 410
uniform sampler2D g_Texture;

layout(location = 0) out vec4 normal;
layout(location = 1) out vec4 pixelPosition;
layout(location = 2) out vec4 material;

in VS_OUT_F {
    vec3 position_worldSpace;
    vec3 position_modelSpace;
    vec4 postion_projSpace;
    vec3 position_modelSpace_transformed;
    vec3 normalDir_cameraSpace;
    vec3 normalDir_sunSpace;
    vec2 UV;
    vec3 eyeDir_cameraSpace;
    vec3 lightDir_cameraSpace;
    float rho_s;
    float rho_d;
} fs_in;

//out vec4 fragColor;

void main()
{
    
//    fragColor = texture(g_Texture, fs_in.UV);
//    fragColor = vec4(fs_in.UV.x, fs_in.UV.y, 0.0, 1.0);
//    fragColor = vec4(0.5, 0.0, 1.0, 1.0);
    
    normal = vec4(fs_in.normalDir_sunSpace, length(fs_in.normalDir_sunSpace));
    pixelPosition = vec4(fs_in.position_modelSpace_transformed.xyz, length(fs_in.postion_projSpace.xyz));
    material = vec4(fs_in.rho_s, fs_in.rho_d, 1.0 - fs_in.rho_s - fs_in.rho_d, 1.0);
    
//    normal = vec4(fs_in.rho_s, fs_in.rho_d, 1.f - fs_in.rho_s - fs_in.rho_d, 1.f);
    
//    normal = vec4(1.0, 2.0, 3.0, 1.0);
//    pixelPosition = vec4(5.0, 6.0, 7.0, 1.0);
//    material = vec4(8.0, 9.0, 10.0, 1.0);
    
//    fragColor = vec4(fs_in.normalDir_cameraSpace.x, fs_in.normalDir_cameraSpace.y, fs_in.normalDir_cameraSpace.z, 1.0);

}
