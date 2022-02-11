/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#version 410
// Input vertex data, different for all executions of this shader
layout(location = 0) in dvec3 vertexPosition_modelSpace;
layout(location = 2) in dvec3 vertexNormal_modelSpace;
layout(location = 4) in dvec2 vertexUV;

//// Constant values
uniform dmat4 modelMatrix; // NB dcm
uniform mat4 nodeMatrix; // homogenous transformation matrix from body frame to notional node frame
uniform dmat4 sunModelMatrix; // SB dcm
uniform dmat4 projectionMatrix;
uniform dmat4 viewMatrix;
uniform dmat3 normalMatrix;
uniform dvec3 lightPosition_worldSpace;
uniform dvec3 sHat_B;

layout (std140) uniform nodeTransforms
{
    mat4 matrix[4];     // 16              // 0  (column 0)
    // 16              // 16  (column 1)
    // 16              // 32  (column 2)
    // 16              // 48  (column 3) * 4
};

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
    // Rotate mesh with respect to its own coordinate frame origin
    dvec3 tmpPos_nodeRel = dvec3(vertexPosition_modelSpace - dvec3(nodeMatrix[3]));
    dvec3 vertexPosition_modelSpace_rotated = dmat3x3(nodeMatrix)*tmpPos_nodeRel;
    dvec4 tmpPos_BodyRel = dvec4(vertexPosition_modelSpace_rotated + dvec3(nodeMatrix[3]), 1.0);
    // Output position of the vertex in clip space
    gl_Position = vec4(projectionMatrix * viewMatrix * sunModelMatrix * tmpPos_BodyRel);

    ///// From here down only needed if wanting to debug with fragment shader ///////
    
    
//    // Position of the vertex in world space
//    vs_out.position_worldSpace = dvec4(modelMatrix * dvec4(vertexPosition_modelSpace, 1.0)).xyz;
//    // Vector from vertex to camera in camera space
//    // In camera space, the camera is at the origin
//    dvec3 vertexPosition_cameraSpace = dvec4(viewMatrix * modelMatrix * dvec4(vertexPosition_modelSpace, 1.0)).xyz;
//    vs_out.eyeDir_cameraSpace = normalize(vec3(0, 0, 0) - vertexPosition_cameraSpace);
//
//    // Vector from vertex to the light in camera space
//    dvec3 lightPosition_cameraSpace = dvec4(viewMatrix * dvec4(lightPosition_worldSpace, 1.0)).xyz;
//    vs_out.lightDir_cameraSpace = normalize(lightPosition_cameraSpace - vertexPosition_cameraSpace);
//
//    // Normal of the vertex in camera space
//    vs_out.normalDir_cameraSpace = normalize(normalMatrix * vertexNormal_modelSpace);
//
//    // UV of the vertex
//    vs_out.UV = vertexUV;
//    vs_out.position_modelSpace = vertexPosition_modelSpace;
//
//    vs_out_f.position_worldSpace = vec3(vs_out.position_worldSpace);
//    vs_out_f.position_modelSpace = vec3(vs_out.position_modelSpace);
//    vs_out_f.normalDir_cameraSpace = vec3(vs_out.normalDir_cameraSpace);
//    vs_out_f.UV = vec2(vs_out.UV);
//    vs_out_f.eyeDir_cameraSpace = vec3(vs_out.eyeDir_cameraSpace);
//    vs_out_f.lightDir_cameraSpace = vec3(vs_out.lightDir_cameraSpace);
}

