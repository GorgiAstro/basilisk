//
//  boundingBox.cpp
//  SimUtilities
//
//  Created by Patrick Kenneally on 12/31/17.
//

#include <math.h>
#include <iostream>
#include "boundingBox.h"
#include "glm/gtx/norm.hpp"

// This bounding box starts with a minimum size. This is particularly useful for gemoetry such as
// an axis aligned flate plate non-manifold mesh (has no depth, simply planes) and therefore would have
// a zero volume bounding box. We need bounding boxes to have volume to compute the size of the
// region they take up.
BoundingBox::BoundingBox()
{
    this->max = glm::dvec3(0.001);
    this->min = glm::dvec3(-0.001);
}

BoundingBox::BoundingBox(std::vector<glm::dvec3> vertices) : BoundingBox()
{
    for(int idx = 0; idx < vertices.size(); idx++) {
        this->max.x = fmax(this->max.x, vertices.at(idx)[0]);
        this->min.x = fmin(this->min.x, vertices.at(idx)[0]);
        this->max.y = fmax(this->max.y, vertices.at(idx)[1]);
        this->min.y = fmin(this->min.y, vertices.at(idx)[1]);
        this->max.z = fmax(this->max.z, vertices.at(idx)[2]);
        this->min.z = fmin(this->min.z, vertices.at(idx)[2]);
    }
    return;
}

BoundingBox::~BoundingBox()
{
    
}

double BoundingBox::getDepth(){
    return fabs(max.z - min.z);
}
double BoundingBox::getWidth(){
    return fabs(max.x - min.x);
}
double BoundingBox::getHeight(){
    return fabs(max.y - min.y);
}

int BoundingBox::getRayGridWidth(){
    // assumes s/c model is in meters and we want centimeter
    // so we multiple by 100
    return (int)ceil(fabs(max.x - min.x)*100);
}

int BoundingBox::getRayGridHeight(){
    return (int)ceil(fabs(max.y - min.y)*100);
}

double BoundingBox::getDiagonalLength()
{
    glm::dvec3 tmp;
    tmp[0] = this->min.x - this->max.x;
    tmp[1] = this->min.y - this->max.y;
    tmp[2] = this->min.z - this->max.z;
    return glm::l2Norm(tmp);
}

glm::dvec3 BoundingBox::getCenter()
{
    glm::dvec3 center;
    center[0] = (this->min.x + this->max.x)/2;
    center[1] = (this->min.y + this->max.y)/2;
    center[2] = (this->min.z + this->max.z)/2;
    return center;
}

std::vector<glm::ivec4> BoundingBox::getBBoxQuadMeshIndices() {
    std::vector<glm::ivec4> indices;
    indices.resize(6);
    indices[0] = glm::ivec4(0, 1, 2, 3);
    indices[1] = glm::ivec4(4, 5, 6, 7);
    indices[2] = glm::ivec4(2, 1, 6, 5);
    indices[3] = glm::ivec4(2, 5, 4, 3);
    indices[4] = glm::ivec4(3, 4, 7, 0);
    indices[5] = glm::ivec4(1, 0, 7, 6);
    return indices;
}

std::vector<glm::dvec4> BoundingBox::getVec4BBoxQuadMeshVertices(){
    std::vector<glm::dvec4> boundVertices;
    boundVertices.resize(8);
    boundVertices[0] = glm::vec4(this->min, 1.0);
    boundVertices[1] = glm::vec4(max.x, min.y, min.z, 1.0);
    boundVertices[2] = glm::vec4(max.x, max.y, min.z, 1.0);
    boundVertices[3] = glm::vec4(min.x, max.y, min.z, 1.0);
    boundVertices[4] = glm::vec4(min.x, max.y, max.z, 1.0);
    boundVertices[5] = glm::vec4(this->max, 1.0);
    boundVertices[6] = glm::vec4(max.x, min.y, max.z, 1.0);
    boundVertices[7] = glm::vec4(min.x, min.y, max.z, 1.0);
    return boundVertices;
}

std::vector<glm::dvec3> BoundingBox::getVec3BBoxQuadMeshVertices(){
    std::vector<glm::dvec3> boundVertices;
    boundVertices.resize(8);
    boundVertices[0] = this->min;
    boundVertices[1] = glm::vec3(max.x, min.y, min.z);
    boundVertices[2] = glm::vec3(max.x, max.y, min.z);
    boundVertices[3] = glm::vec3(min.x, max.y, min.z);
    boundVertices[4] = glm::vec3(min.x, max.y, max.z);
    boundVertices[5] = this->max;
    boundVertices[6] = glm::vec3(max.x, min.y, max.z);
    boundVertices[7] = glm::vec3(min.x, min.y, max.z);
    return boundVertices;
}

double BoundingBox::getMaxX()
{
    return this->max.x;
}

double BoundingBox::getMinX()
{
    return this->min.x;
}

double BoundingBox::getMaxY()
{
    return this->max.y;
}

double BoundingBox::getMinY()
{
    return this->min.y;
}

double BoundingBox::getMaxZ()
{
    return this->max.z;
}

double BoundingBox::getMinZ()
{
    return this->min.z;
}

// This transform assumes the bound box was created with body frame mesh verticies
// But the transform matrix may define a different coordinate frame with an origin
// offset from the body frame. As such we need to remove the offset, do the rotation,
// then add back the offset.
void BoundingBox::transform(glm::dmat4 transform)
{
    glm::dvec3 tmpMin = glm::dvec3(this->min.x - transform[3][0], this->min.y - transform[3][1], this->min.z - transform[3][2]);
    glm::dvec3 tmpMax = glm::dvec3(this->max.x - transform[3][0], this->max.y - transform[3][1], this->max.z - transform[3][2]);
    
    glm::dmat3 dcm = glm::dmat3(transform);
    tmpMin = dcm*tmpMin;
    tmpMax = dcm*tmpMax;
    
    this->min = glm::dvec3(tmpMin.x + transform[3][0], tmpMin.y + transform[3][1], tmpMin.z + transform[3][2]);
    this->max = glm::dvec3(tmpMax.x + transform[3][0], tmpMax.y + transform[3][1], tmpMax.z + transform[3][2]);
}
