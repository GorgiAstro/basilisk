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

#include "texMap.h"
#include <OpenGL/CGLCurrent.h>
#include "openGLUtil.h"
#include <iostream>
#include <fstream>
#include <numeric>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <random>
#include <memory>
#include <algorithm>
#include <unistd.h>
#include "shaderProgram.h"
#include "texture.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
////#include "glm/ext.hpp"
#include "glm/gtx/norm.hpp"
#include "lodepng.h"
#include "utilities.h"
#include "EigenGLMUtils.h"
#include "mesh.h"
#include "util.h"

const unsigned int kMaxGroupSize = 64;
const unsigned int kMaxGroupCount = 64;

static void glfw_error_callback(int error, const char* desc)
{
    fputs(desc,stderr);
}

// Constructor
TexMapSRP::TexMapSRP(unsigned int width, unsigned int height, unsigned int fboDim)
{
    winWidth = width;
    winHeight = height;
    fboWidth = fboDim;
    fboHeight = fboDim;
    fbo = 0;
    this->glParams.shadowMap = 0;
    near = 0.0;
    far = 1.0;
    pixelArea = 0.0;
    showNormalMap = false;
    force[0] = 0.0;
    force[1] = 0.0;
    force[2] = 0.0;
    force[3] = 0.0;
    
    this->shaderManager = nullptr;
}

TexMapSRP::~TexMapSRP()
{
    if (this->fbo != 0) {
        gl::DeleteFramebuffers(1, &this->fbo);
    }
    
    if (this->glParams.shadowMap != 0) {
        gl::DeleteTextures(1, &this->glParams.shadowMap);
    }
    
    this->clParams.outputBufForce = nullptr;
    this->clParams.outputBufTorque = nullptr;
    this->clParams.sharedBuf = nullptr;
    this->clParams.sHat_B_Buf = nullptr;
    gl::DeleteTextures(1, &this->glParams.shadowMap);
    gl::DeleteTextures(1, &this->glParams.normalMap);
    gl::DeleteTextures(1, &this->glParams.pixelPosMap);
    gl::DeleteFramebuffers(1, &this->fbo);
    
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

bool TexMapSRP::initialize()
{
    // this order is important
    // first create a GL window and context
    this->initializeOpenGl();
    // now we set up the frame buffers
    this->initializeFbo();
    // now that we have a GL context and the framebuffers, we can create a CL context which will share the frame buffers
    this->initializeOpenCl();
    // create the shader manager before we start wanting to reference/access shader progrems
    this->shaderManager = std::unique_ptr<ShaderManager>(new ShaderManager(this->assetDir));
    //std::make_unique<ShaderManager>(this->assetDir);
    // with these two set up we can go ahead configuring vertex buffers and shaders programs
    this->initializeMeshModel();
    //        glfwGetFramebufferSize(this->window, &this->fboWidth, &this->fboHeight);
    if (this->showNormalMap) {
        this->initializeFullScreenQuad();
    }
    // this->initializeTestPointsBuffer();
    // this->glParams.texTestId = loadDDS("/Users/patrick/Documents/SoftwareDevelopment/Basilisk_Fork/src/simulation/dynamics/openGLCLSrp/resources/textures/uvtemplate.DDS");
    return true;
}

void TexMapSRP::initializeOpenCl()
{
    cl_int errCode;
    
    try {
        cl::Platform lPlatform = getPlatform();
        // Select the default platform and create a context using this platform and the GPU
#ifdef __APPLE__
        CGLContextObj kCGLContext = CGLGetCurrentContext();
        CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
        cl_context_properties cps[] =
        {
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
            0
        };
#endif
#ifdef OS_LNX
        cl_context_properties cps[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetGLXContext(window),
            CL_GLX_DISPLAY_KHR, (cl_context_properties)glfwGetX11Display(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)lPlatform(),
            0
        };
#endif
#ifdef OS_WIN
        cl_context_properties cps[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetWGLContext(window),
            CL_WGL_HDC_KHR, (cl_context_properties)GetDC(glfwGetWin32Window(window)),
            CL_CONTEXT_PLATFORM, (cl_context_properties)lPlatform(),
            0
        };
#endif
        
        std::vector<cl::Device> devices;
        lPlatform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        for (unsigned d=0; d < devices.size(); ++d) {
            if (checkExtnAvailability(devices[d], CL_GL_SHARING_EXT)) {
                this->clParams.device = devices[d];
            }
            
        }
        std::cout << "Device Vendor " << this->clParams.device.getInfo<CL_DEVICE_VENDOR>() << std::endl;
        
        this->clParams.context = cl::Context(this->clParams.device, cps, NULL, NULL, &errCode);
        std::cout << "context " << errCode << std::endl;
        
        // Create a command queue and use the first device
        this->clParams.queue = cl::CommandQueue(this->clParams.context, this->clParams.device,  NULL, &errCode);
        std::cout << "CommandQueue " << errCode << std::endl;
        
        this->clParams.program = getProgram(this->clParams.context, this->assetDir+"cl_kernels/normalsSrpParallelReduce.cl", errCode);
        std::cout << "getProgram " << errCode << std::endl;
        
        this->clParams.program.build(std::vector<cl::Device>(1, this->clParams.device));
        this->clParams.kernel = cl::Kernel(this->clParams.program, "normalsSrpParallelReduceFloat");
        this->clParams.normalMap = cl::ImageGL(this->clParams.context, CL_MEM_READ_ONLY, gl::TEXTURE_2D, 0, this->glParams.normalMap, &errCode);
        this->clParams.pixelPosMap = cl::ImageGL(this->clParams.context, CL_MEM_READ_ONLY, gl::TEXTURE_2D, 0, this->glParams.pixelPosMap, &errCode);
        this->clParams.materialMap = cl::ImageGL(this->clParams.context, CL_MEM_READ_ONLY, gl::TEXTURE_2D, 0, this->glParams.materialMap, &errCode);
        
        //        this->params.sharedBuf = cl::Buffer(cl_context,
        //                                        CL_MEthis->READ_WRITE,
        //                                        this->fboWidth*this->fboHeight*4*sizeof(float),
        //                                        nullptr,
        //                                        &errCode);
        
        this->clParams.outputBufForce = cl::Buffer(this->clParams.context,
                                                 CL_MEM_READ_WRITE,
                                                 this->fboWidth*this->fboHeight*4*sizeof(float),
                                                 nullptr,
                                                 &errCode);
        
        this->clParams.outputBufTorque = cl::Buffer(this->clParams.context,
                                                  CL_MEM_READ_WRITE,
                                                  this->fboWidth*this->fboHeight*4*sizeof(float),
                                                  nullptr,
                                                  &errCode);
        
        this->clParams.sHat_B_Buf = cl::Buffer(this->clParams.context,
                                             CL_MEM_READ_WRITE,
                                             3*sizeof(float),
                                             nullptr,
                                             &errCode);
        
    } catch(cl::Error error) {
        std::cout << error.what() << "(" << error.err() << ")" << std::endl;
        std::string val = this->clParams.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(this->clParams.device);
        std::cout<<"Log:\n"<<val<<std::endl;
    }
    return;
}


void TexMapSRP::initializeOpenGl()
{
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit())
        return;
    
    //        glfwWindowHint(GLFW_RESIZABLE, gl::TRUE_);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, gl::TRUE_);
#endif
    
    this->window = glfwCreateWindow(this->winWidth, this->winHeight, "Random particles", NULL, NULL);
    if (!this->window) {
        glfwTerminate();
        return;
    }
    
    glfwMakeContextCurrent(this->window);
    glload::LoadTest test = glload::LoadFunctions();
    if(!test)
    {
        glfwTerminate();
        return;
    }
    std::cout << "#### OpenGL Version ####" << std::endl;
    std::cout << "GLFW reported version string: " << glfwGetVersionString() << std::endl;
    const GLubyte* renderer = gl::GetString(gl::RENDERER);
    const GLubyte* version = gl::GetString(gl::VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported " << version << std::endl;
    std::cout << "########################" << std::endl;
    
    gl::Enable(gl::DEPTH_TEST);
    gl::ClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    // Accept fragment if it closer to the camera than the former one
    gl::DepthFunc(gl::LESS);
    // Cull triangles which normal is not towards the camera
    gl::Enable(gl::CULL_FACE);
    
    return;
}

void TexMapSRP::initializeMeshModel()
{
    // init spacecraft mesh model
    this->model = Geometry(this->modelFileName);
    this->model.initializeWithShaderIdPtr(this->shaderManager->getProgramPointer("simpleNormals")); CHECK_GL_ERROR;
}

void TexMapSRP::initializeTestPointsBuffer()
{
    this->glParams.prgTestPoints = std::make_shared<ShaderProgram>(this->assetDir, "testPoints");
    gl::GenVertexArrays(1, &this->glParams.vaoTestPoints);
    gl::BindVertexArray(this->glParams.vaoTestPoints);
    static const GLfloat pointsVertexBufferData[] = {
        128, 136, 0.0,
        173, 103, 0.0,
        249, 51, 0.0,
        340, 103, 0.0,
        303, 188, 0.0,
        342, 221, 0.0,
        208, 320, 0.0,
        294, 287, 0.0,
        333, 320, 0.0,
        178, 188, 0.0,
        216, 221, 0.0,
        169, 287, 0.0,
        115, 151, 0.0,
        117, 269, 0.0,
        390, 419, 0.0,
        392, 327, 0.0,
        399, 320, 0.0,
        381, 250, 0.0,
        406, 287, 0.0,
        395, 478, 0.0,
        373, 349, 0.0,
        379, 342, 0.0,
        388, 242, 0.0,
        384, 427, 0.0,
        399, 235, 0.0,
        397, 235, 0.0,
        401, 412, 0.0,
        399, 412, 0.0,
        389, 183, 0.0,
        405, 346, 0.0
    };
    
    gl::GenBuffers(1, &this->glParams.vboTestPoints);
    gl::BindBuffer(gl::ARRAY_BUFFER, this->glParams.vboTestPoints);
    gl::BufferData(gl::ARRAY_BUFFER, sizeof(pointsVertexBufferData), pointsVertexBufferData, gl::STATIC_DRAW);
    gl::BindBuffer(gl::ARRAY_BUFFER, 0);
    gl::BindVertexArray(0);
}

void TexMapSRP::initializeFullScreenQuad()
{
    this->glParams.prgScreenQuad = std::make_shared<ShaderProgram>(this->assetDir, "screenTexture");
    gl::GenVertexArrays(1, &this->glParams.vaoScreenQuad);
    gl::BindVertexArray(this->glParams.vaoScreenQuad);
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };
    
    gl::GenBuffers(1, &this->glParams.vboScreenQuad);
    gl::BindBuffer(gl::ARRAY_BUFFER, this->glParams.vboScreenQuad);
    gl::BufferData(gl::ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, gl::STATIC_DRAW);
    gl::BindBuffer(gl::ARRAY_BUFFER, 0);
    gl::BindVertexArray(0);
}

void TexMapSRP::initializeFbo()
{
    GLint maxSize = 0;
    gl::GetIntegerv(gl::MAX_RENDERBUFFER_SIZE, &maxSize);
    if (this->fboHeight > maxSize) this->fboHeight = maxSize;
    if (this->fboWidth > maxSize) this->fboWidth = maxSize;
    
    // Create the FBO
    gl::GenFramebuffers(1, &this->fbo);
    gl::BindFramebuffer(gl::FRAMEBUFFER, this->fbo);
    
    // Create the depth buffer
    gl::GenTextures(1, &this->glParams.shadowMap);
    gl::BindTexture(gl::TEXTURE_2D, this->glParams.shadowMap);
    /* @TODO look at using the method shown in https://www.codeproject.com/Articles/685281/OpenGL-OpenCL-Interoperability-A-Case-Study-Using
     to get higher (24 or 32 bit) depth component. Right now open_cl image
     format only supports DEPTH_COMPONENT16 */
    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::DEPTH_COMPONENT16, fboWidth, fboHeight, 0, gl::DEPTH_COMPONENT, gl::FLOAT, NULL);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE);
    gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, gl::TEXTURE_2D, this->glParams.shadowMap, 0);
    
    GLenum status = gl::CheckFramebufferStatus(gl::FRAMEBUFFER);
    
    if (status != gl::FRAMEBUFFER_COMPLETE) {
        printf("FB error for shadow map, status: 0x%x\n", status);
    }
    gl::BindTexture(gl::TEXTURE_2D, 0); CHECK_GL_ERROR;
    
    gl::GenTextures(1, &this->glParams.normalMap); CHECK_GL_ERROR;
    gl::BindTexture(gl::TEXTURE_2D, this->glParams.normalMap); CHECK_GL_ERROR;
    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, fboWidth, fboHeight, 0, gl::RGBA, gl::FLOAT, NULL); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE); CHECK_GL_ERROR;
    gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0, gl::TEXTURE_2D, this->glParams.normalMap, 0); CHECK_GL_ERROR;
    
    status = gl::CheckFramebufferStatus(gl::FRAMEBUFFER);
    
    if (status != gl::FRAMEBUFFER_COMPLETE) {
        printf("FB error for normal map, status: 0x%x\n", status);
    }
    gl::BindTexture(gl::TEXTURE_2D, 0); CHECK_GL_ERROR;
    
    gl::GenTextures(1, &this->glParams.pixelPosMap); CHECK_GL_ERROR;
    gl::BindTexture(gl::TEXTURE_2D, this->glParams.pixelPosMap); CHECK_GL_ERROR;
    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, fboWidth, fboHeight, 0, gl::RGBA, gl::FLOAT, NULL); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE); CHECK_GL_ERROR;
    gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT1, gl::TEXTURE_2D, this->glParams.pixelPosMap, 0); CHECK_GL_ERROR;
    
    status = gl::CheckFramebufferStatus(gl::FRAMEBUFFER); CHECK_GL_ERROR;
    
    if (status != gl::FRAMEBUFFER_COMPLETE) {
        printf("FB error for pixel position map, status: 0x%x\n", status);
    }
    gl::BindTexture(gl::TEXTURE_2D, 0); CHECK_GL_ERROR;
    
    gl::GenTextures(1, &this->glParams.materialMap); CHECK_GL_ERROR;
    gl::BindTexture(gl::TEXTURE_2D, this->glParams.materialMap); CHECK_GL_ERROR;
    gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, fboWidth, fboHeight, 0, gl::RGBA, gl::FLOAT, NULL); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE); CHECK_GL_ERROR;
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE); CHECK_GL_ERROR;
    gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT2, gl::TEXTURE_2D, this->glParams.materialMap, 0); CHECK_GL_ERROR;
    
    status = gl::CheckFramebufferStatus(gl::FRAMEBUFFER); CHECK_GL_ERROR;
    
    if (status != gl::FRAMEBUFFER_COMPLETE) {
        printf("FB error for material map, status: 0x%x\n", status);
    }
    
    gl::BindFramebuffer(gl::FRAMEBUFFER, 0); CHECK_GL_ERROR;
    gl::BindTexture(gl::TEXTURE_2D, 0); CHECK_GL_ERROR;
}

void TexMapSRP::bindFboForWriting()
{
    gl::BindFramebuffer(gl::DRAW_FRAMEBUFFER, this->fbo);
}

void TexMapSRP::bindFboForReading(GLenum textureUnit)
{
    gl::ActiveTexture(textureUnit);
    gl::BindTexture(gl::TEXTURE_2D, this->glParams.shadowMap);
}

void TexMapSRP::compute()
{
    this->computeGL();
    this->computeCL();
    // swap front and back buffers
    glfwSwapBuffers(this->window);
    // poll for events
    if (this->showNormalMap) glfwPollEvents();
}

Eigen::Vector3d TexMapSRP::getForce()
{
    return Eigen::Vector3d(this->force[0], this->force[1], this->force[2]);
}

Eigen::Vector3d TexMapSRP::getTorque()
{
    return Eigen::Vector3d(this->torque[0], this->torque[1], this->torque[2]);
}

void TexMapSRP::computeGL()
{
    BoundingBox box_loose_S = this->looseSunFrameBoundingBox(&this->model);
    BoundingBox box_B = this->model.getBoundingBox();
    
    //double maxLength = box_B.getDiagonalLength();
    this->model.setDcmSB(glm::dmat4(eigenToGlmDcm(this->getSB_dcm())));
    
    this->far = abs(box_loose_S.getMaxX() - box_loose_S.getMinX());
    
    // We want to look at the center of the sun frame bounding box not
    // 0,0,0 which would be the origin of the body frame.
    glm::dvec3 originBbox_S = box_loose_S.getCenter();
    //    this->cameraMat = glm::lookAt(glm::dvec3(maxLength/2, originBbox_S[1], -originBbox_S[2]), originBbox_S, glm::dvec3(0.0,1.0,0.0));
    //glm::dvec3 targetOffset = glm::dvec3(0.0, 0.0, 0.0);
    //glm::dvec3 eyeOffset = glm::dvec3(0.0, 0.0, 0.0);
    
    this->cameraMat = glm::lookAt(glm::dvec3(box_loose_S.getMaxX(), originBbox_S[1], originBbox_S[2]), originBbox_S, glm::dvec3(0.0,1.0,0.0));
    
    // sun bounding box is generated where for the sun frame sHat_B is the first unit vector making up the frame
    // maxing the X direction towards the sun. Therefore Y and Z form the projection plane coord basis.
    //double maxZ = std::max(abs(box_loose_S.getMinZ() - originBbox_S[2]), abs(box_loose_S.getMaxZ() - originBbox_S[2]));
    //double maxY = std::max(abs(box_loose_S.getMinY() - originBbox_S[1]), abs(box_loose_S.getMaxY() - originBbox_S[1]));
    double maxDim = std::max(abs(box_loose_S.getMinZ()) + abs(box_loose_S.getMaxZ()), abs(box_loose_S.getMinY()) + abs(box_loose_S.getMaxY()));
    this->projMat = glm::ortho(-maxDim/2, maxDim/2, -maxDim/2, maxDim/2, this->near, this->far);
    //    this->projMat = glm::ortho(-maxDim/2, maxDim/2, -maxDim/2, maxDim/2, this->near, this->far);
    //    this->projMat = glm::ortho(-maxLength/2, maxLength/2, -maxLength/2, maxLength/2, this->near, maxLength);
    //    this->pixelArea = maxLength/this->fboWidth*maxLength/this->fboWidth;
    this->pixelArea = maxDim/this->fboWidth*maxDim/this->fboWidth;
    //    std::cout << "this->pixelArea " << this->pixelArea << std::endl;
    //    this->maxDim = maxLength;
    
    gl::BindFramebuffer(gl::FRAMEBUFFER, this->fbo);
    gl::Viewport(0, 0, this->fboWidth, this->fboHeight);
    gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
    GLenum drawBuffers[] = {gl::COLOR_ATTACHMENT0, gl::COLOR_ATTACHMENT1, gl::COLOR_ATTACHMENT2};
    gl::DrawBuffers(3, drawBuffers);
    //    this->model.setAttitude(glm::dvec3(this->sigma_BN.x(), this->sigma_BN.y(), this->sigma_BN.z()));
    this->model.draw(this->cameraMat, this->projMat, 1.0);
    
    // we iterate through the array addressing top to bottom for the png save,
    // because glReadPixels writes out the data to image.data from bottom to top.
    if (this->shouldSaveFrameBuffer){
        this->saveFrameBuffer();
    }
    
    gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
    
    if (this->showNormalMap) {
        gl::Viewport(0, 0, this->fboWidth, this->fboHeight);
        gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
        // Bind vertex array object for quad
        gl::BindVertexArray(this->glParams.vaoScreenQuad);
        // Use Quad shader
        this->glParams.prgScreenQuad->enable();
        // Bind our texture in Texture Unit 0
        gl::ActiveTexture(gl::TEXTURE0);
        gl::BindTexture(gl::TEXTURE_2D, this->glParams.normalMap);
        //            gl::BindTexture(gl::TEXTURE_2D, this->glParams.texTestId);
        GLuint id = this->glParams.prgScreenQuad->getShaderProgId();
        GLuint texLoc = gl::GetUniformLocation(id, "renderedTexture");
        gl::Uniform1i(texLoc, 0);
        
        // 1rst attribute buffer : vertices
        gl::EnableVertexAttribArray(0);
        gl::BindBuffer(gl::ARRAY_BUFFER, this->glParams.vboScreenQuad);
        gl::VertexAttribPointer(
                                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                                3,                  // size
                                gl::FLOAT,           // type
                                gl::FALSE_,           // normalized?
                                0,                  // stride
                                (void*)0            // array buffer offset
                                );
        
        // Draw the triangles !
        gl::DrawArrays(gl::TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
        
        gl::BindTexture(gl::TEXTURE_2D, 0);
        gl::BindBuffer(gl::ARRAY_BUFFER, 0);
        gl::DisableVertexAttribArray(0);
        gl::BindVertexArray(0);
    }
    
    gl::Finish();
}

void TexMapSRP::computeCL()
{
    cl::Event ev;
    try {
        this->force[0] = 0.0; this->force[1] = 0.0; this->force[2] = 0.0; this->force[3] = 0.0;
        this->torque[0] = 0.0; this->torque[1] = 0.0; this->torque[2] = 0.0; this->torque[3] = 0.0;
        
        Eigen::Vector3f tmp = Eigen::Vector3f(this->sHat_B[0], this->sHat_B[1], this->sHat_B[2]);
        this->clParams.queue.enqueueWriteBuffer(this->clParams.sHat_B_Buf, CL_TRUE, 0, 3*sizeof(float), static_cast<void*>(tmp.data()), NULL, NULL);
        
        std::vector<cl::Memory> objs;
        objs.clear();
        objs.push_back(this->clParams.normalMap);
        objs.push_back(this->clParams.pixelPosMap);
        objs.push_back(this->clParams.materialMap);
        
        // flush opengl commands and wait for object acquisition
        cl_int res = this->clParams.queue.enqueueAcquireGLObjects(&objs, NULL, &ev);
        ev.wait();
        if (res != CL_SUCCESS) {
            std::cout << "Failed acquiring GL object: " << res << std::endl;
            exit(247);
        }
        
        cl::NDRange localSize(kMaxGroupSize);
        unsigned int textureSize = this->fboWidth * this->fboHeight;
        // For my hardware we have max groups of 64 and max group size of 64 (GPU prefers 64).
        cl::NDRange globalSize = ((textureSize + 63) / 64) * 64;
        if (globalSize[0] > kMaxGroupSize * kMaxGroupCount)
        {
            globalSize = kMaxGroupSize * kMaxGroupCount;
        }
        
        unsigned int sharedBufSize = kMaxGroupSize*sizeof(float)*4;
        
        // 64*64 is set as the limit as that's the maximum number of groups (16) x work items (64) we will launch
        // 16 and 64 are the numbers from the GPU I have for "Max compute units" and
        // "Preferred work group size multiple" from clinfo output.
        //            if (sharedBufSize > 64*64) sharedBufSize = sizeof(double)*64*16*4;
        //            cl::NDRange globalSize = ((tmpNumPrimitivesInMesh + 15) / 16) * 16;
        // std::cout << this->pixelArea << std::endl;
        // set kernel arguments
        int argc = 0;
        this->clParams.kernel.setArg(argc++, this->clParams.normalMap);
        this->clParams.kernel.setArg(argc++, this->clParams.pixelPosMap);
        this->clParams.kernel.setArg(argc++, this->clParams.materialMap);
        this->clParams.kernel.setArg(argc++, sharedBufSize, nullptr);
        this->clParams.kernel.setArg(argc++, sharedBufSize, nullptr);
        this->clParams.kernel.setArg(argc++, this->clParams.outputBufForce);
        this->clParams.kernel.setArg(argc++, this->clParams.outputBufTorque);
        this->clParams.kernel.setArg(argc++, this->clParams.sHat_B_Buf);
        this->clParams.kernel.setArg(argc++, (float)this->pixelArea);
        this->clParams.kernel.setArg(argc++, this->fboWidth);
        this->clParams.kernel.setArg(argc++, this->fboHeight);
        this->clParams.kernel.setArg(argc++, textureSize);
        
        this->clParams.queue.enqueueNDRangeKernel(this->clParams.kernel,
                                                cl::NullRange,
                                                globalSize,
                                                localSize);
        
        unsigned int numComputeUnitResults = globalSize[0]/localSize[0]*4;
        
        std::vector<float> tmpForce = std::vector<float>(numComputeUnitResults);
        std::vector<float> tmpTorque = std::vector<float>(numComputeUnitResults);
        
        this->clParams.queue.enqueueReadBuffer(this->clParams.outputBufForce, CL_TRUE, 0, numComputeUnitResults*sizeof(float), static_cast<void*>(tmpForce.data()));
        this->clParams.queue.enqueueReadBuffer(this->clParams.outputBufTorque, CL_TRUE, 0, numComputeUnitResults*sizeof(float), static_cast<void*>(tmpTorque.data()));
        
        for (int i = 0; i < numComputeUnitResults; i = i + 4)
        {
            this->force[0] += tmpForce[i+0];
            this->force[1] += tmpForce[i+1];
            this->force[2] += tmpForce[i+2];
            this->force[3] += tmpForce[i+3];
            
            this->torque[0] += tmpTorque[i+0];
            this->torque[1] += tmpTorque[i+1];
            this->torque[2] += tmpTorque[i+2];
            this->torque[3] += tmpTorque[i+3];
        }
        
        // release opengl objects
        res = this->clParams.queue.enqueueReleaseGLObjects(&objs, NULL, &ev);
        ev.wait();
        if (res != CL_SUCCESS) {
            std::cout << "Failed releasing GL object: " << res << std::endl;
            exit(247);
        }
        this->clParams.queue.finish();
        
    } catch(cl::Error err) {
        std::cout << err.what() << "(" << err.err() << ")" << std::endl;
    }
}

BoundingBox TexMapSRP::looseSunFrameBoundingBox(Geometry* model)
{
    Eigen::Matrix3d dcm_SB = this->getSB_dcm();
    // Map vertices into sun frame
    std::vector<glm::dvec3> vertices_B = model->getBoundingBox().getVec3BBoxQuadMeshVertices();
    
    std::vector<glm::dvec3> vertices_S;
    
    glm::dmat3 glmDcm_SB = eigenToGlmDcm(dcm_SB);
    
    for(int idx = 0; idx < vertices_B.size(); idx++) {
        vertices_S.push_back(glmDcm_SB*vertices_B[idx]);
    }
    
    BoundingBox box_S = BoundingBox(vertices_S);
    return box_S;
}

Eigen::Matrix3d TexMapSRP::getSB_dcm()
{
    Eigen::Vector3d randUnitVec_B(0.0, 1.0, 0.0);
    if (std::fabs(randUnitVec_B.dot(this->sHat_B)) > 0.95)
    {
        randUnitVec_B = Eigen::Vector3d(1.0, 0.0, 0.0);
    }
    // Body to sun (bounding box) frame
    Eigen::Matrix3d dcm_SB;
    Eigen::Vector3d sHat_2 = this->sHat_B.cross(randUnitVec_B).normalized();
    Eigen::Vector3d sHat_3 = this->sHat_B.cross(sHat_2).normalized();
    
    // GLM matricies are column major so we lay out the basis vectors manually
    dcm_SB.row(0) = this->sHat_B;
    dcm_SB.row(1) = sHat_2;
    dcm_SB.row(2) = sHat_3;
    
    return dcm_SB;
}

Eigen::Matrix3d TexMapSRP::getNS_dcm()
{
    Eigen::Vector3d randUnitVec_N(0.0, 1.0, 0.0);
    if (std::fabs(randUnitVec_N.dot(this->sHat_N)) > 0.95)
    {
        randUnitVec_N = Eigen::Vector3d(1.0, 0.0, 0.0);
    }
    // Sun to inertial DCM
    Eigen::Matrix3d dcm_NS;
    Eigen::Vector3d sHat_2 = this->sHat_N.cross(randUnitVec_N).normalized();
    Eigen::Vector3d sHat_3 = this->sHat_N.cross(sHat_2).normalized();
    
    dcm_NS.row(0) = this->sHat_N;
    dcm_NS.row(1) = sHat_2;
    dcm_NS.row(2) = sHat_3;
    
    return dcm_NS;
}

void TexMapSRP::setNodeDcms(std::unordered_map<std::string, glm::dmat4> dcms)
{
    for ( auto it = dcms.begin(); it != dcms.end(); ++it )
    {
        this->model.setNodeDcm(it->first, it->second);
    }
}

void TexMapSRP::setResolution(unsigned int resolution)
{
    this->fboHeight = resolution;
    this->fboWidth = resolution;
}

void TexMapSRP::setDebugWindowSize(unsigned int size)
{
    this->winWidth = size;
    this->winHeight = size;
}

double TexMapSRP::getPixelArea()
{
    return this->pixelArea;
}

void TexMapSRP::saveTextureImage(int width, int height, std::vector<unsigned char> tex_data)
{
    long imageSize = 4 * width * height;
    int xa= width % 256;
    int xb= (width-xa)/256;int ya= height % 256;
    int yb= (height-ya)/256; //assemble the header
    char header[18] = {0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,32,0};
    // write header and data to file
    std::fstream File("/Users/patrick/Documents/SoftwareDevelopment/basilisk/screenshot.tga", std::ios::out | std::ios::binary);
    File.write (reinterpret_cast<char *>(header), sizeof (char)*18);
    File.write (reinterpret_cast<char *>(tex_data.data()), sizeof (char)*imageSize);
    File.close();
    return;
}

void TexMapSRP::updateMaterial(std::string name, float rho_s, float rho_d)
{
    this->model.updateMaterial(name, rho_s, rho_d);
}

void TexMapSRP::setShouldSaveFrameBuffer(bool shouldSave)
{
    this->shouldSaveFrameBuffer = shouldSave;
}

void TexMapSRP::setOutputFilePath(std::string outputFilePath)
{
    this->outputFilePath = outputFilePath;
}

void TexMapSRP::setAssetDir(std::string path)
{
    this->assetDir = path;
}

void TexMapSRP::saveFrameBuffer()
{
    // Save screen shot for debugging
    std::vector<float> image;
    image.resize(this->fboWidth * this->fboHeight);
    std::vector<unsigned char> imagePNG;
    imagePNG.resize(this->fboWidth * this->fboHeight);
    gl::ReadPixels(0, 0, this->fboWidth, this->fboHeight, gl::DEPTH_COMPONENT, gl::FLOAT, image.data());// split x and y sizes into bytes
    for(int y = this->fboHeight - 1; y >= 0; --y){
        for(int x = 0; x < this->fboWidth; x++)
        {
            // flip the y index for imagePNG so we write into the array from the start of the array
            // in ascedning order. If we didn't flip we'd end up writing in descending order and not
            // flipping the image values vertically at all.
            imagePNG[this->fboWidth * (-1)*(y - this->fboHeight+1) + x + 0] = (unsigned)(image[this->fboWidth * y + x]*255.0);
        }
    }
    
    unsigned error = lodepng::encode(this->outputFilePath +  "/depth_fbo.png", imagePNG, this->fboWidth, this->fboHeight, LCT_GREY, 8);
    if (error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
    
    image.clear();
    image.resize(4*this->fboWidth * this->fboHeight);
    imagePNG.clear();
    imagePNG.resize(4*this->fboWidth * this->fboHeight);
    gl::PixelStorei(gl::PACK_ALIGNMENT, 4);
    gl::ReadPixels(0, 0, this->fboWidth, this->fboHeight, gl::RGBA, gl::FLOAT, image.data()); CHECK_GL_ERROR;
    for(int y = this->fboHeight - 1; y >= 0; --y){
        for(int x = 0; x < 4*this->fboWidth; x = x + 4)
        {
            imagePNG[4*this->fboWidth * (-1)*(y - this->fboHeight+1) + x + 0] = (unsigned)(image[4*this->fboWidth * y + x + 0]*255.0);
            imagePNG[4*this->fboWidth * (-1)*(y - this->fboHeight+1) + x + 1] = (unsigned)(image[4*this->fboWidth * y + x + 1]*255.0);
            imagePNG[4*this->fboWidth * (-1)*(y - this->fboHeight+1) + x + 2] = (unsigned)(image[4*this->fboWidth * y + x + 2]*255.0);
            imagePNG[4*this->fboWidth * (-1)*(y - this->fboHeight+1) + x + 3] = (unsigned)(image[4*this->fboWidth * y + x + 3]*255.0);
        }
    }
    
    error = lodepng::encode(this->outputFilePath +  "/color_fbo.png", imagePNG, this->fboWidth, this->fboHeight, LCT_RGBA);
    if (error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}
