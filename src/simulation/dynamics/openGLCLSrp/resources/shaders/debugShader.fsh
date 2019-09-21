#version 410

// Interpolated values from the geometry shader
//in GS_OUT {
//    vec3 position_worldSpace;
//    vec3 normalDir_cameraSpace;
//    vec2 UV;
//    vec3 eyeDir_cameraSpace;
//    vec3 lightDir_cameraSpace;
//} fs_in;

in VS_OUT_F {
    vec3 position_worldSpace;
    vec3 position_modelSpace;
    vec3 normalDir_cameraSpace;
    vec2 UV;
    vec3 eyeDir_cameraSpace;
    vec3 lightDir_cameraSpace;
} fs_in;

//// Constant values
uniform sampler2D gShadowMap;
uniform dvec3 lightPosition_worldSpace;
uniform dvec3 lightIntensity;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform float shininess;

// Output values for each fragment
out vec4 fragColor;

void main()
{
//    float tmp = texture(gShadowMap, fs_in.UV).x;
//    vec4 color = vec4(tmp,tmp,tmp,0.5);
    vec4 color = vec4(1.0,0.5,0.5,0.5);
    // Normal of the computed fragment in camera space
//    vec3 normal = normalize(fs_in.normal_cameraSpace);
    // Direction of the light (From the fragment to the light)
//    vec3 lightDir = normalize(fs_in.lightDir_cameraSpace);
    // Cosine of the angle between the normal and the light direction
    // clamped above 0
    float cosTheta = clamp(dot(fs_in.normalDir_cameraSpace, fs_in.lightDir_cameraSpace), 0.0, 1.0);
//    cosTheta = dot(fs_in.normalDir_cameraSpace, fs_in.lightDir_cameraSpace);
//    cosTheta = dot(fs_in.normalDir_cameraSpace, vec3(1.0,1.0,1.0));
    vec4 cosThetaVec = vec4(cosTheta, cosTheta, cosTheta, 1.0);
    // Eye vector (toward the camera) in camera space
//    vec3 eyeDir = normalize(fs_in.eyeDir_cameraSpace);
    // Direction in which the the triangle reflects the light
    vec3 r = reflect(-fs_in.lightDir_cameraSpace, fs_in.normalDir_cameraSpace);
    // Cosine of the angle between the eye and reflect vectors
    float cosAlpha = clamp(dot(fs_in.eyeDir_cameraSpace, r), 0, 1);
    float powCosAlpha = pow(cosAlpha, shininess);
    vec4 cosAlphaVec = vec4(powCosAlpha, powCosAlpha, powCosAlpha, 1.0);
    
//    vec4 textureColor = texture(textureId, fs_in.UV);
    
    
    fragColor = ambientColor * color
    + diffuseColor * color * vec4(lightIntensity, 1.0) * cosThetaVec
    + specularColor * vec4(lightIntensity, 1.0) * cosAlphaVec;
    fragColor = vec4(0.0,0.0,0.0, 1.0);
}
