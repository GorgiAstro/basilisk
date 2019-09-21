//
//  boundingBox.hpp
//  SimUtilities
//
//  Created by Patrick Kenneally on 12/31/17.
//

#ifndef boundingBox_h
#define boundingBox_h

#include <stdio.h>
#include <vector>
#include "glm/glm.hpp"


class BoundingBox {

public:
    BoundingBox();
    BoundingBox(std::vector<glm::dvec3> vertices);
    ~BoundingBox();
    std::vector<glm::dvec3> getBoundsVertices();
    double getDepth();
    double getWidth();
    double getHeight();
    double getMaxX();
    double getMinX();
    double getMaxY();
    double getMinY();
    double getMaxZ();
    double getMinZ();
    int getRayGridWidth();
    int getRayGridHeight();
    glm::dvec3 getCenter();
    std::vector<glm::dvec4> getVec4BBoxQuadMeshVertices();
    std::vector<glm::dvec3> getVec3BBoxQuadMeshVertices();
    std::vector<glm::ivec4> getBBoxQuadMeshIndices();
    void transform(glm::dmat4 transform);
    double getDiagonalLength();

private:
    glm::dvec3 max;
    glm::dvec3 min;
};


#endif /* boundingBox_h */
