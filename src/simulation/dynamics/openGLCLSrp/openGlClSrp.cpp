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
#include "openGlClSrp.h"
#include <fstream>
#include <chrono>
#include <iostream>
#include <unistd.h>

#include "architecture/messaging/system_messaging.h"
#include "utilities/astroConstants.h"
#include "utilities/avsEigenSupport.h"
#include "utilities/avsEigenMRP.h"
#include "cl.hpp"
#include "materialInfo.h"
#include "mesh.h"
#include "EigenGLMUtils.h"
#include "utilities/rigidBodyKinematics.h"
#include "simMessages/eclipseSimMsg.h"

#define GetCurrentDir getcwd

#pragma mark -
#pragma mark Private - Constants

//#define CHECK_GL_ERROR assert(glGetError() == 0)
//#pragma OPENCL EXTENSION cl_khr_fp64 : enable
//#define CHECK_GL_ERROR assert(gl::GetError() == 0)


void OnError(int error, const char* description)
{
    std::cout << description << "\n";
}

template<typename T>
std::vector<T> flatten(const std::vector<glm::dvec3> &orig)
{
    std::vector<T> ret;
    for(const auto &v: orig)
    {
        ret.push_back(v[0]);
        ret.push_back(v[1]);
        ret.push_back(v[2]);
    }
    return ret;
}

/*! This method is used to create any messages published by this module.
 @return void
 */
std::string GetCurrentWorkingDir( void ) {
    char buff[FILENAME_MAX];
    GetCurrentDir( buff, FILENAME_MAX );
    std::string current_working_dir(buff);
    return current_working_dir;
}

OpenGLCLSrp::OpenGLCLSrp()
    : sunEphmInMsgName("sun_planet_data")
    , stateInMsgName("inertial_state_output")
    , sunEphmInMsgId(-1)
    , stateInMsgId(-1)
    , eclipseInMsgId(-1)
    , stateRead(false)
    , prevTime(0.0)
    , avgRenderTime(0.0)
    , renderCount(0)
{
    this->texMapCompute = nullptr;
    std::cout << GetCurrentWorkingDir() << std::endl;
    CallCounts = 0;
    
    return;
}

OpenGLCLSrp::OpenGLCLSrp(std::string modelFilePath)
    : OpenGLCLSrp()
{
    this->texMapSettings.modelFilePath = modelFilePath;
    this->texMapSettings.mtlBasePath = modelFilePath.substr(0, modelFilePath.find_last_of("\\/"));
}

OpenGLCLSrp::~OpenGLCLSrp()
{
    return;
}

void OpenGLCLSrp::Reset(uint64_t CurrentSimNanos)
{
    if (this->texMapCompute != nullptr)
    {
        this->texMapCompute.reset();
    }
    
    this->texMapCompute = std::make_shared<TexMapSRP>(10, 10, 10);
    this->texMapCompute->setAssetDir(this->texMapSettings.assetDir);
    this->texMapCompute->setShouldSaveFrameBuffer(this->texMapSettings.saveFrameBuffer);
    this->texMapCompute->setResolution(this->texMapSettings.resolution);
    this->texMapCompute->setDebugWindowSize(this->texMapSettings.windowSize);
    this->texMapCompute->setOutputFilePath(this->texMapSettings.frameBufferOutputPath);
    this->texMapCompute->setShouldSaveFrameBuffer(this->texMapSettings.saveFrameBuffer);
    this->texMapCompute->showNormalMap = this->texMapSettings.showNormalMap;
    this->texMapCompute->modelFileName = this->texMapSettings.modelFilePath;
    
    uint32_t res = 0;
    res = this->texMapCompute->initialize();
    
//    this->m_texMapCompute->updateMaterial("MLK_GK.001", this->rho_s_gk, this->rho_d_gk);
//    this->m_texMapCompute->updateMaterial("solar_cells", this->rho_s_sp, this->rho_d_sp);
    
    this->prevTime = 0.0;
    this->totalComputeTime = 0.0;
    this->renderCount = 0;
    this->avgRenderTime = 0;
    this->eclipseMsgBuffer.shadowFactor = 1.0;
    this->previousIntegTime = 0.0;
}

void OpenGLCLSrp::SelfInit()
{
}

/*! This method is used to subscribe to systems message from this module.
 It sets the message ID based on what it finds for the input string.  If the
 message is not successfully linked, it will warn the user.
 @return void
 */
void OpenGLCLSrp::CrossInit()
{
    //! - Find the message ID associated with the ephmInMsgID string.
    //! - Warn the user if the message is not successfully linked.
    this->sunEphmInMsgId = SystemMessaging::GetInstance()->subscribeToMessage(this->sunEphmInMsgName, sizeof(SpicePlanetStateSimMsg), this->moduleID);
    
    if(this->sunEphmInMsgId < 0)
    {
        std::cerr << "WARNING: Did not find a valid message with name: ";
        std::cerr << this->sunEphmInMsgName << "  :" << __FILE__ << std::endl;
    }
    
    this->stateInMsgId = SystemMessaging::GetInstance()->subscribeToMessage(this->stateInMsgName, sizeof(SCPlusStatesSimMsg), this->moduleID);
    
    if(this->stateInMsgId < 0)
    {
        std::cerr << "WARNING: Did not find a valid message with name: ";
        std::cerr << this->stateInMsgId << "  :" << __FILE__ << std::endl;
    }
    
    if (this->eclipseInMsgName != "")
    {
        this->eclipseInMsgId = SystemMessaging::GetInstance()->subscribeToMessage(this->eclipseInMsgName, sizeof(EclipseSimMsg), this->moduleID);
    }
    
}

void OpenGLCLSrp::UpdateState(uint64_t CurrentSimNanos)
{
    std::unordered_map<std::string, glm::dmat4> tmpDcms;
    double n = 0.00099623297206;
    
    for ( auto it = this->nodeDCMs.begin(); it != this->nodeDCMs.end(); ++it )
    {
        tmpDcms[it->first] = eigenToGlmDcm(it->second);

        if (it->first == "sa_back" || it->first == "sa_front" || it->first == "sa_support")
        {
            double dt = CurrentSimNanos/1E9 - this->prevTime/1E9;
            double rot = dt*n;
            Eigen::Matrix3d m_1;
            Eigen::Matrix3d m_2;
            m_1 = it->second.block<3,3>(0,0);
            m_2 = Eigen::AngleAxisd(-rot, Eigen::Vector3d::UnitZ());
            it->second.block<3,3>(0,0) = m_2*m_1;
            tmpDcms[it->first] = eigenToGlmDcm(it->second);
        }
    }
    this->prevTime = CurrentSimNanos;

    this->texMapCompute->setNodeDcms(tmpDcms);
    this->readMessages();
}

/*! This method is used to read the incoming ephmeris and
 spacecraft state messages. The data is stored in the associated
 buffer structure.
 @return void
 */
void OpenGLCLSrp::readMessages()
{
    this->stateRead = false;
    SingleMessageHeader localHeader;
    memset(&localHeader, 0x0, sizeof(localHeader));
    
    bool sunMsgRead = false;
    if(this->sunEphmInMsgId >= 0)
    {
        memset(&this->sunEphmInBuffer, 0x0, sizeof(SpicePlanetStateSimMsg));
        sunMsgRead = SystemMessaging::GetInstance()->ReadMessage(this->sunEphmInMsgId, &localHeader, sizeof(SpicePlanetStateSimMsg), reinterpret_cast<uint8_t*> (&this->sunEphmInBuffer));
    }
    
    memset(&localHeader, 0x0, sizeof(localHeader));
    bool stateMsgRead = false;
    if(this->stateInMsgId >= 0)
    {
        memset(&this->stateInBuffer, 0x0, sizeof(SCPlusStatesSimMsg));
        stateMsgRead = SystemMessaging::GetInstance()->ReadMessage(this->stateInMsgId, &localHeader, sizeof(SCPlusStatesSimMsg), reinterpret_cast<uint8_t*> (&this->stateInBuffer));
    }
    
    if(this->eclipseInMsgName != "")
    {
        memset(&localHeader, 0x0, sizeof(localHeader));
        memset(&this->eclipseMsgBuffer, 0x0, sizeof(EclipseSimMsg));
        SystemMessaging::GetInstance()->ReadMessage(this->eclipseInMsgId, &localHeader, sizeof(EclipseSimMsg), reinterpret_cast<uint8_t*> (&this->eclipseMsgBuffer));
    }
    // We've successfully received s/c state and sun state data
    this->stateRead = sunMsgRead && stateMsgRead;
}

/*! This method retrieves pointers to parameters/data stored
 in the dynamic parameter manager
 @return void
 @param statesIn Dynamic parameter manager
 */
void OpenGLCLSrp::linkInStates(DynParamManager& statesIn)
{
}

/*! This method computes the dynamic effect due to solar raidation pressure.
 It is an inherited method from the DynamicEffector class and
 is designed to be called by the simulation dynamics engine.
 @return void
 @param integTime Current simulation integration time
 */
void OpenGLCLSrp::computeForceTorque(double integTime)
{
    if (!this->stateRead)
    {
        this->forceExternal_N.setZero();
        this->forceExternal_B.setZero();
        this->torqueExternalPntB_B.setZero();
        return;
    }
    if (this->previousIntegTime == integTime) return;

    std::cout << "integTime: " << integTime << std::endl;
    this->updateSunVectors();
    this->texMapCompute->sHat_B = this->sHat_B;
    this->texMapCompute->sHat_N = this->sHat_N;
    this->texMapCompute->sigma_BN = Eigen::Vector3d(this->stateInBuffer.sigma_BN[0],
                                                    this->stateInBuffer.sigma_BN[1],
                                                    this->stateInBuffer.sigma_BN[2]);
    
    auto start = std::chrono::high_resolution_clock::now();
    this->texMapCompute->compute();
    Eigen::Vector3d normalizedForcePerArea = this->texMapCompute->getForce();
    Eigen::Vector3d normalizedTorquePerArea = this->texMapCompute->getTorque();
    
    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms(stop - start);
    if (this->renderCount > 1)
    {
        this->totalComputeTime += fp_ms.count();
    }

    this->renderCount++;
    this->avgRenderTime = this->totalComputeTime/this->renderCount;

    double R_s = this->s_N.norm();

    this->forceExternal_B = this->eclipseMsgBuffer.shadowFactor*normalizedForcePerArea*(SOLAR_FLUX_EARTH*(AU*1000*AU*1000)/(SPEED_LIGHT*R_s*R_s)); // for regular bsk sims
    // TODO: remove but only by pat
    //    this->forceExternal_B = normalizedForcePerArea; // for Orex

    this->torqueExternalPntB_B = this->eclipseMsgBuffer.shadowFactor*normalizedTorquePerArea*(SOLAR_FLUX_EARTH*AU*1000*AU*1000/(SPEED_LIGHT*R_s*R_s));
//    std::cout << "normalizedForcePerArea: " << normalizedForcePerArea[0] << " " << normalizedForcePerArea[1] << " " << normalizedForcePerArea[2] << std::endl;
}

void OpenGLCLSrp::setEclipseMessageName(std::string eclipseMsgName)
{
    this->eclipseInMsgName = eclipseMsgName;
}

void OpenGLCLSrp::setAssetPath(std::string path)
{
    this->texMapSettings.assetDir = path;
}

void OpenGLCLSrp::setModelFilePath(std::string modelFilePath)
{
    this->texMapSettings.modelFilePath = modelFilePath;
    this->texMapSettings.mtlBasePath = modelFilePath.substr(0, modelFilePath.find_last_of("\\/"));
}

void OpenGLCLSrp::setNodeTransform(std::string name, Eigen::Matrix3d dcm_B, Eigen::Vector3d r_BM_B)
{
    this->nodeDCMs[name] = Eigen::Matrix4d::Zero();
    this->nodeDCMs[name].block<3,3>(0,0) = dcm_B;
    this->nodeDCMs[name].col(3) << Eigen::Vector4d(r_BM_B[0], r_BM_B[1], r_BM_B[2], 1.0);
}

void OpenGLCLSrp::setNodeTransform(std::string name, Eigen::Matrix4d transform)
{
    this->nodeDCMs[name] = transform;
}

void OpenGLCLSrp::setResolution(unsigned int resolution)
{
    this->texMapSettings.resolution = resolution;
}

void OpenGLCLSrp::setDebugWindowSize(unsigned int size)
{
    this->texMapSettings.windowSize = size;
}

void OpenGLCLSrp::showNormalMap(bool showNormalMap)
{
    this->texMapSettings.showNormalMap = showNormalMap;
}

void OpenGLCLSrp::setBuildOpts(std::string opts)
{
    //this->renderer->m_buildopts
}

void OpenGLCLSrp::shouldOutputKernelDebugInfo(bool shouldOutput)
{
    this->texMapSettings.outputDebugInfo = shouldOutput;
}

double OpenGLCLSrp::getAvgRenderTime()
{
    return this->avgRenderTime;
}

void OpenGLCLSrp::updateSunVectors()
{
    Eigen::Vector3d r_N(this->stateInBuffer.r_BN_N);
    Eigen::Vector3d sun_r_N(this->sunEphmInBuffer.PositionVector);
    this->s_N = sun_r_N - r_N;
    // TODO: there's surely a more Eigen correct way to do the following
    this->sHat_N = Eigen::Vector3d(this->s_N[0]/this->s_N.norm(), this->s_N[1]/this->s_N.norm(), this->s_N[2]/this->s_N.norm());
    
    Eigen::MRPd sigma_BN(this->stateInBuffer.sigma_BN);
    Eigen::Matrix3d dcm_BN = sigma_BN.toRotationMatrix().transpose();
    this->sHat_B = dcm_BN*this->sHat_N;
    
    return;
}

void OpenGLCLSrp::setFrameBufferOutputPath(std::string filePath)
{
    this->texMapSettings.frameBufferOutputPath = filePath;
}

void OpenGLCLSrp::setShouldSaveFrameBuffer(bool shouldSave)
{
    this->texMapSettings.saveFrameBuffer = shouldSave;
}

void OpenGLCLSrp::setDeviceIndex(unsigned int index)
{
    this->texMapSettings.deviceIndex = index;
}
