//
//  EigenGLMUtils.hpp
//  AlgorithmMessaging
//
//  Created by Patrick Kenneally on 12/18/18.
//

#ifndef EigenGLMUtils_hpp
#define EigenGLMUtils_hpp

#include <stdio.h>
#include <Eigen/Dense>
#include "glm/glm.hpp"

inline glm::dmat3 eigenToGlmDcm(Eigen::Matrix3d dcm)
{
    glm::dmat3 newDcm;
    newDcm[0][0] = dcm.row(0)[0];
    newDcm[1][0] = dcm.row(0)[1];
    newDcm[2][0] = dcm.row(0)[2];
    newDcm[0][1] = dcm.row(1)[0];
    newDcm[1][1] = dcm.row(1)[1];
    newDcm[2][1] = dcm.row(1)[2];
    newDcm[0][2] = dcm.row(2)[0];
    newDcm[1][2] = dcm.row(2)[1];
    newDcm[2][2] = dcm.row(2)[2];
    
    return newDcm;
}

inline glm::dmat4 eigenToGlmDcm(Eigen::Matrix4d dcm)
{
    glm::dmat4 newDcm;
    newDcm[0][0] = dcm.row(0)[0];
    newDcm[1][0] = dcm.row(0)[1];
    newDcm[2][0] = dcm.row(0)[2];
    newDcm[3][0] = dcm.row(0)[3];
    
    newDcm[0][1] = dcm.row(1)[0];
    newDcm[1][1] = dcm.row(1)[1];
    newDcm[2][1] = dcm.row(1)[2];
    newDcm[3][1] = dcm.row(1)[3];
    
    newDcm[0][2] = dcm.row(2)[0];
    newDcm[1][2] = dcm.row(2)[1];
    newDcm[2][2] = dcm.row(2)[2];
    newDcm[3][2] = dcm.row(2)[3];
    
    newDcm[0][3] = dcm.row(3)[0];
    newDcm[1][3] = dcm.row(3)[1];
    newDcm[2][3] = dcm.row(3)[2];
    newDcm[3][3] = dcm.row(3)[3];
    return newDcm;
}
#endif /* EigenGLMUtils_hpp */
