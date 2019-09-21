//
//  MaterialInfo.cpp
//  Simulation
//
//  Created by Patrick Kenneally on 8/4/15.
//
//

#include "materialInfo.h"


MaterialInfo::MaterialInfo()
{

}

MaterialInfo::MaterialInfo(glm::vec4 color)
    : ambientColor(color)
    , diffuseColor(glm::vec4(0, 0, 0, 0))
    , specularColor(glm::vec4(0, 0, 0, 0))
    , shininess(0.0)
    , rhoD(0.0)
    , rhoS(0.0)
{}

MaterialInfo::~MaterialInfo(){}
