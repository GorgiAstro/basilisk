//
//  mesh.cpp
//  Simulation
//
//  Created by Patrick Kenneally on 8/4/15.
//
//

#include "mesh.h"
#include "materialInfo.h"

Mesh::Mesh() : name("")
    , indexOffset(0)
    , indexCount(0)
    , vertexOffset(0)
    , vertexCount(0)
    , primitiveType(0)
    , textureFile("")
    , material(new MaterialInfo())
{
    
}

Mesh::~Mesh()
{
    
}
