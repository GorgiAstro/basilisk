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
//#pragma once

#ifndef _tex_map_SRP_h
#define _tex_map_SRP_h

#include <glload/gl_4_1.hpp>
#include <glload/gl_load.hpp>
#include <chrono>

//#ifdef __APPLE__
//#define GLFW_EXPOSE_NATIVE_X11
//#define GLFW_EXPOSE_NATIVE_GLX
//#endif

//#ifdef OS_WIN
//#define GLFW_EXPOSE_NATIVE_WIN32
//#define GLFW_EXPOSE_NATIVE_WGL
//#endif
//
//#ifdef OS_LNX
//#define GLFW_EXPOSE_NATIVE_X11
//#define GLFW_EXPOSE_NATIVE_GLX
//#endif

#include <memory>
#include <Eigen/Dense>
#include <unordered_map>
#include "utilities/avsEigenSupport.h"
#include "utilities/avsEigenMRP.h"
#include <glfw3.h>
//#include <glfw3native.h>
#include "geometry.h"
#include "openCLUtil.h"
#include "glm/glm.hpp"
#include "shaderManager.h"

typedef struct {
    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
    cl::Kernel kernel;
    cl::size_t<3> dims;
    
    cl::BufferGL vertexBuf;
    cl::BufferGL indexBuf;
    cl::BufferGL normalBuf;
    cl::BufferGL uvsBuf;
    cl::BufferGL uboBuf;
    cl::ImageGL depthMap;
    cl::ImageGL normalMap;
    cl::ImageGL pixelPosMap;
    cl::ImageGL materialMap;
    
    cl::Buffer centroidsBuf;
    cl::Buffer areasBuf;
    cl::Buffer outputBufForce;
    cl::Buffer outputBufTorque;
    cl::Buffer sharedBuf;
    cl::Buffer sHat_B_Buf;
    cl::Buffer transform;
    cl::Buffer projMat;
    cl::Buffer cameraMat;
    cl::Buffer SB;
} CLParams;

typedef struct {
    std::shared_ptr<ShaderProgram> prg;
    std::shared_ptr<ShaderProgram> prgScreenQuad;
    std::shared_ptr<ShaderProgram> prgTestPoints;
    GLuint vaoScreenQuad;
    GLuint vaoTestPoints;
    GLuint vboScreenQuad;
    GLuint vboTestPoints;
    GLuint shadowMap;
    GLuint normalMap;
    GLuint pixelPosMap;
    GLuint materialMap;
    GLuint texTestId;
} GLParams;

class TexMapSRP
{
public:
    TexMapSRP(unsigned int winWidth, unsigned int winHeight, unsigned int fboDim);
    ~TexMapSRP();
    
    void compute();
    void setNodeDcms(std::unordered_map<std::string, glm::dmat4> dcms);
    void setResolution(unsigned int resolution);
    void setDebugWindowSize(unsigned int size);
    double getPixelArea();
    Eigen::Vector3d getForce();
    Eigen::Vector3d getTorque();
    void updateMaterial(std::string name, float rho_s, float rho_d);
    void setShouldSaveFrameBuffer(bool shouldSave);
    void setOutputFilePath(std::string outputFilePath);
    void setAssetDir(std::string path);
    bool initialize();
    
public:
    Eigen::Vector3d sHat_B;
    Eigen::Vector3d sHat_N;
    Eigen::MRPd sigma_BN;
    float depth;
    std::string shaderName;
    bool showNormalMap;
    double maxDim;
    std::string modelFileName;
    
private:
    CLParams clParams;
    GLParams glParams;
    GLFWwindow* window;
    std::unique_ptr<ShaderManager> shaderManager;
    int winWidth;
    int winHeight;
    int fboWidth;
    int fboHeight;
    Geometry model;
    GLuint fbo;
    glm::dmat4 projMat;
    glm::dmat4 cameraMat;
    double near;
    double far;
    double force[4];
    double torque[4];
    double pixelArea;
    bool shouldSaveFrameBuffer;
    std::string outputFilePath;
    std::string assetDir;
    
private:
    void initializeOpenGl();
    void initializeOpenCl();
    void initializeMeshModel();
    void initializeFbo();
    void initializeFullScreenQuad();
    void initializeTestPointsBuffer();
    void computeGL();
    void computeCL();
    void parallelReduce();
    void bindFboForWriting();
    void bindFboForReading(GLenum textureUnit);
    void saveFrameBuffer();
    void saveTextureImage(int width, int height, std::vector<unsigned char> tex_data);
    Eigen::Matrix3d getSB_dcm();
    Eigen::Matrix3d getNS_dcm();
    BoundingBox looseSunFrameBoundingBox(Geometry* model);
    void launchKernelForNode(Geometry::Node node);
    void bufferNodeData(Geometry::Node *node);
};

#endif
