#version 410

// Input vertex data, different for all executions of this shader
layout(location = 0) in dvec3 vertexPosition_modelSpace;
layout(location = 2) in dvec3 vertexNormal_modelSpace;
layout(location = 4) in dvec2 vertexUV;

//// Constant values
uniform dmat4 modelMatrix;
uniform dmat4 projectionMatrix;
uniform dmat4 viewMatrix;
uniform dmat3 normalMatrix;
uniform dvec3 lightPosition_worldSpace;
uniform dvec3 sHat_B;

// Output data to be interpolated for each fragment
out VS_OUT {
    dvec3 position_worldSpace;
    dvec3 position_modelSpace;
    dvec3 normalDir_cameraSpace;
    dvec2 UV;
    dvec3 eyeDir_cameraSpace;
    dvec3 lightDir_cameraSpace;
} vs_out;

out VS_OUT_F {
    vec3 position_worldSpace;
    vec3 position_modelSpace;
    vec3 normalDir_cameraSpace;
    vec2 UV;
    vec3 eyeDir_cameraSpace;
    vec3 lightDir_cameraSpace;
} vs_out_f;

void main()
{
    // Output position of the vertex in clip space
    gl_Position = vec4(projectionMatrix * viewMatrix * modelMatrix * dvec4(vertexPosition_modelSpace, 1.0));
    
    // Position of the vertex in world space
    vs_out.position_worldSpace = dvec4(modelMatrix * dvec4(vertexPosition_modelSpace, 1.0)).xyz;
    
    // Vector from vertex to camera in camera space
    // In camera space, the camera is at the origin
    dvec3 vertexPosition_cameraSpace = dvec4(viewMatrix * modelMatrix * dvec4(vertexPosition_modelSpace, 1.0)).xyz;
    vs_out.eyeDir_cameraSpace = normalize(vec3(0, 0, 0) - vertexPosition_cameraSpace);

    // Vector from vertex to the light in camera space
    dvec3 lightPosition_cameraSpace = dvec4(viewMatrix * dvec4(lightPosition_worldSpace, 1.0)).xyz;
    vs_out.lightDir_cameraSpace = normalize(lightPosition_cameraSpace - vertexPosition_cameraSpace);

    // Normal of the vertex in camera space
    vs_out.normalDir_cameraSpace = normalize(normalMatrix * vertexNormal_modelSpace);

    // UV of the vertex
    vs_out.UV = vertexUV;
    vs_out.position_modelSpace = vertexPosition_modelSpace;
    
    vs_out_f.position_worldSpace = vec3(vs_out.position_worldSpace);
    vs_out_f.position_modelSpace = vec3(vs_out.position_modelSpace);
    vs_out_f.normalDir_cameraSpace = vec3(vs_out.normalDir_cameraSpace);
    vs_out_f.UV = vec2(vs_out.UV);
    vs_out_f.eyeDir_cameraSpace = vec3(vs_out.eyeDir_cameraSpace);
    vs_out_f.lightDir_cameraSpace = vec3(vs_out.lightDir_cameraSpace);
}
