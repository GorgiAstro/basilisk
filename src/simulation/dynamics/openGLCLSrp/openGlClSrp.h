/*
 ISC License
 
 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder
 
 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
 
 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
 */

#ifndef OPENGLCL_SRP_H
#define OPENGLCL_SRP_H

#define USE_OPENCL 1

#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"

#include "_GeneralModuleFiles/sys_model.h"
#include "../_GeneralModuleFiles/dynamicEffector.h"
#include "../_GeneralModuleFiles/stateData.h"
#include "../_GeneralModuleFiles/dynParamManager.h"
#include "simMessages/spicePlanetStateSimMsg.h"
#include "simMessages/scPlusStatesSimMsg.h"
#include "simMessages/eclipseSimMsg.h"
#include "texMap.h"
#include "openCLUtil.h"

typedef struct {
    std::string assetDir; //!< -- Absolute path to the directory containing the OpenCL kernel directory
    std::string modelFilePath; //!< -- Absolute file path for the OBJ mesh model file
    std::string mtlBasePath; //!< -- File path, relative to the modelFilePath for the mesh model OBJ materials file
    std::string frameBufferOutputPath;
    bool saveFrameBuffer;
    bool showNormalMap;
    unsigned int resolution;
    unsigned int windowSize;
    bool outputDebugInfo; //!< -- If set to true OpenCL kernels will be set to output print info
    unsigned int deviceIndex;
} TexMapSettings;

//! @brief Radiation pressure dynamics class used to compute
//  SRP effects on body
class OpenGLCLSrp: public SysModel, public DynamicEffector {
public:
    OpenGLCLSrp();
    OpenGLCLSrp(std::string modelFilePath);
    ~OpenGLCLSrp();

    void SelfInit();
    void CrossInit();
    void UpdateState(uint64_t CurrentSimNanos);
    void Reset(uint64_t CurrentSimNanos);
    void linkInStates(DynParamManager& statesIn);
    void readMessages();
    void computeForceTorque(double integTime);
    void showNormalMap(bool showNormalMap);

    void setBuildOpts(std::string opts);
    void setAssetPath(std::string path);
    void setModelFilePath(std::string modelFilePath);
    void setNodeTransform(std::string name, Eigen::Matrix3d dcm_B, Eigen::Vector3d r_BM_B);
    void setNodeTransform(std::string name, Eigen::Matrix4d transform);
    void setResolution(unsigned int resolution);
    void setDebugWindowSize(unsigned int size);
    void setEclipseMessageName(std::string eclipseMsgName);
    void setFrameBufferOutputPath(std::string filePath);
    void setShouldSaveFrameBuffer(bool shouldSave);
    void shouldOutputKernelDebugInfo(bool shouldOutput); //!< -- If set to true OpenCL kernels will be set to output print info
    void setDeviceIndex(unsigned int index);
    double getAvgRenderTime();
    
public:
    std::string sunEphmInMsgName; //!< -- sun ephemeris message name
    std::string stateInMsgName; //!< -- spacecraft state message name
    
    // TODO: remove temporary hack
    float rho_s_gk;
    float rho_d_gk;
    float rho_s_sp;
    float rho_d_sp;
    
private:
    void updateSunVectors();
    std::string generateFileName();

private:
    std::shared_ptr<TexMapSRP> texMapCompute; // This is a shared_ptr because swig doesn't support unique_ptr
    int64_t sunEphmInMsgId; //!< -- Message ID for incoming sun ephemeris data
    SpicePlanetStateSimMsg sunEphmInBuffer; //!< -- Buffer for incoming ephemeris message data
    int64_t stateInMsgId; //!< -- Message ID for incoming SC state data
    bool stateRead; //!< -- Indicates a succesful read of incoming SC state message data
    SCPlusStatesSimMsg stateInBuffer; //!< -- Buffer for incoming state message data
    std::unordered_map<std::string, Eigen::Matrix4d> nodeDCMs; //!< -- Map of named homogeneous transformation matrices corresponding to each submesh
    double prevTime;
    double previousIntegTime;
    std::uint32_t renderCount;
    double totalComputeTime;
    TexMapSettings texMapSettings;
    std::string eclipseInMsgName;
    std::uint32_t eclipseInMsgId;
    EclipseSimMsg eclipseMsgBuffer;
    Eigen::Vector3d sHat_B;
    Eigen::Vector3d sHat_N;
    Eigen::Vector3d s_N;
    double avgRenderTime;
};

/*! @} */

#endif
