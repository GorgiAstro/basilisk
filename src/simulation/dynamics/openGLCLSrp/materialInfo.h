//
//  MaterialInfo.h
//  Simulation
//
//  Created by Patrick Kenneally on 8/4/15.
//
//

#ifndef __Simulation__MaterialInfo__
#define __Simulation__MaterialInfo__

#include <stdio.h>
#include "glm/glm.hpp"
#include <string>

class MaterialInfo
{
public:
    std::string name;
    glm::vec4 ambientColor;
    glm::vec4 diffuseColor;
    glm::vec4 specularColor;
    double shininess;
    float rhoD;
    float rhoS;
    
    MaterialInfo();
    MaterialInfo(glm::vec4 color);
    ~MaterialInfo();
    
protected:
private:
};

#endif /* defined(__Simulation__MaterialInfo__) */
