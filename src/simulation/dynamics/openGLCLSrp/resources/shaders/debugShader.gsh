#version 410

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Geometry Shaders are recieved as arrays of the base type
in VS_OUT {
    dvec3 position_worldSpace;
    dvec3 position_modelSpace;
    dvec3 normalDir_cameraSpace;
    dvec2 UV;
    dvec3 eyeDir_cameraSpace;
    dvec3 lightDir_cameraSpace;
} gs_in[];

uniform double solarFlux;
uniform dvec3 sHat_B;
uniform dmat4 modelMatrix;
uniform double rhoD;
uniform double rhoS;
//uniform sampler2D gShadowMap;

// Geometry Shaders output a vector for each vertex
out GS_OUT {
    vec3 position_worldSpace;
    vec3 normalDir_cameraSpace;
    vec2 UV;
    vec3 eyeDir_cameraSpace;
    vec3 lightDir_cameraSpace;
} gs_out;

out dvec3 forceSrp;
out dvec3 torqueSrp;
out dvec3 cg;                    // centroid
out double area;
out double cosTheta;

void main() {
    double c = 299792458.0;  // [m/s] speed of light
    double rho_d = 0.2;
    double rho_s = 0.2;
    double rhoA = 1 - rho_s - rho_d;
    dvec3 V[3];
//    float tmp = texture(gShadowMap, vec2(1.0,1.0)).x;
    for (int i = 0; i < 3; i++)
    {
        if (i == 0) {
            dvec3 edge1 = gs_in[1].position_modelSpace - gs_in[0].position_modelSpace;
            dvec3 edge2 = gs_in[2].position_modelSpace - gs_in[0].position_modelSpace;
            dvec3 faceNormal = cross(edge1,edge2);
            double normalMag = abs(sqrt(faceNormal[0]*faceNormal[0] + faceNormal[1]*faceNormal[1] + faceNormal[2]*faceNormal[2]));
            dvec3 nHat_B = dvec3(faceNormal[0]/normalMag,faceNormal[1]/normalMag,faceNormal[2]/normalMag);
            //dvec3 nHat_B = normalize(faceNormal);
            cosTheta = dot(nHat_B, sHat_B);
            double P_sun = solarFlux/c;                  // [N/m^2] solar radiation pressure
            
            if (cosTheta > 0.0 && cosTheta <= 1.0) {
                //area = 0.5*length(faceNormal); //crazy bug but length() returns negative value traced to sqrt() result!!!
                area = 0.5*normalMag;
                V[0] = gs_in[0].position_modelSpace.xyz;
                V[1] = gs_in[1].position_modelSpace.xyz;
                V[2] = gs_in[2].position_modelSpace.xyz;
                cg = (V[0] + V[1] + V[2])/3.0;

                forceSrp = -P_sun*area*cosTheta*(rhoA*sHat_B + 2.0*rho_s*cosTheta*nHat_B + rho_d*(sHat_B + (2.0/3.0)*nHat_B));
                torqueSrp = cross(cg,forceSrp);
            } else {
                area = 0.0;
                forceSrp = dvec3(0.0,0.0,0.0);
                torqueSrp = dvec3(0.0,0.0,0.0);
            }
        } else {
            forceSrp = dvec3(0.0,0.0,0.0);
            torqueSrp = dvec3(0.0,0.0,0.0);
            cg = dvec3(0.0,0.0,0.0);
            area = 0.0;
            cosTheta = 0.0;
        }
        // Pass through geometry shader per vertex values
        gs_out.position_worldSpace = vec3(gs_in[i].position_worldSpace);
        gs_out.normalDir_cameraSpace = vec3(gs_in[i].normalDir_cameraSpace);
        gs_out.UV = vec2(gs_in[i].UV);
        gs_out.eyeDir_cameraSpace = vec3(gs_in[i].eyeDir_cameraSpace);
        gs_out.lightDir_cameraSpace = vec3(gs_in[i].lightDir_cameraSpace);
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }

    EndPrimitive();
}
