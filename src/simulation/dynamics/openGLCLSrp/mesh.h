//
//  mesh.h
//  Simulation
//
//  Created by Patrick Kenneally on 8/4/15.
//
//

#ifndef __Simulation__mesh__
#define __Simulation__mesh__

#include <stdio.h>
#include <string>
#include "glm/glm.hpp"
#include "materialInfo.h"

class Mesh
{
public:
    Mesh();
    ~Mesh();
    
    std::string name;
    unsigned int indexOffset;
    unsigned int indexCount;
    unsigned int vertexOffset;
    unsigned int vertexCount;
    unsigned int primitiveType;
    std::string textureFile;
    std::shared_ptr<MaterialInfo> material;
    glm::dmat4 dcmBM; // homogenous transform from mesh frame to body frame

protected:
private:
};
#endif /* defined(__Simulation__mesh__) */
