/*
 ISC License

 Copyright (c) 2016, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

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
/*
    FSW MODULE Template
 
 */

/* modify the path to reflect the new module names */
#include "sunlineEphem.h"
#include <string.h>
#include <math.h>




/*
 Pull in support files from other modules.  Be sure to use the absolute path relative to Basilisk directory.
 */
#include "simulation/utilities/linearAlgebra.h"
#include "simulation/utilities/rigidBodyKinematics.h"
//#include "simulation/utilities/astroConstants.h"


/*! This method initializes the ConfigData for this module.
 It checks to ensure that the inputs are sane and then creates the
 output message
 @return void
 @param ConfigData The configuration data associated with this module
 */
void SelfInit_sunlineEphem(sunlineEphemConfig *ConfigData, uint64_t moduleID)
{
    
    /*! Begin method steps */
    /*! - Create output message for module */
    
    ConfigData->navStateOutMsgId = CreateNewMessage(ConfigData->navStateOutMsgName,
                                                    sizeof(NavAttIntMsg), "NavAttIntMsg", moduleID);
}

/*! This method performs the second stage of initialization for this module.
 It's primary function is to link the input messages that were created elsewhere.
 @return void
 @param ConfigData The configuration data associated with this module
 */
void CrossInit_sunlineEphem(sunlineEphemConfig *ConfigData, uint64_t moduleID)
{
    
    /*! Begin method steps */
    
    /*! -- Find the message ID for the sun direction */
    ConfigData->sunPositionInMsgId = subscribeToMessage(ConfigData->sunPositionInMsgName,
                                                        sizeof(EphemerisIntMsg), moduleID);
    
    /*! -- Find the messgae ID for the spacecraft direction */
    ConfigData->scPositionInMsgId = subscribeToMessage(ConfigData->scPositionInMsgName,
                                                       sizeof(NavTransIntMsg), moduleID);
    
    ConfigData->scAttitudeInMsgId = subscribeToMessage(ConfigData->scAttitudeInMsgName,
                                                       sizeof(NavAttIntMsg), moduleID);

}

/*! This method performs a complete reset of the module.  Local module variables that retain
 time varying states between function calls are reset to their default values.
 @return void
 @param ConfigData The configuration data associated with the module
 */
void Reset_sunlineEphem(sunlineEphemConfig *ConfigData, uint64_t callTime, uint64_t moduleID)
{
    memset(&(ConfigData->outputSunline), 0x0, sizeof(NavAttIntMsg));
}

/*! Add a description of what this main Update() routine does for this module
 @return void
 @param ConfigData The configuration data associated with the module
 @param callTime The clock time at which the function was called (nanoseconds)
 */
void Update_sunlineEphem(sunlineEphemConfig *ConfigData, uint64_t callTime, uint64_t moduleID)
{
    uint64_t            clockTime;
    uint32_t            readSize;
    double              rDiff_N[3];
    double              rDiffUnit_N[3];
    double              rDiff_B[3];/*!< [unit] variable description */
    double              dcm_BN[3][3];
    

    /*! Begin method steps*/
    /*! - Read the input messages */
    ReadMessage(ConfigData->sunPositionInMsgId, &clockTime, &readSize,
                sizeof(EphemerisIntMsg), (void*) &(ConfigData->sunEphemBuffer), moduleID);
    
    ReadMessage(ConfigData->scPositionInMsgId, &clockTime, &readSize,
                sizeof(NavTransIntMsg), (void*) &(ConfigData->scTransBuffer), moduleID);
    
    ReadMessage(ConfigData->scAttitudeInMsgId, &clockTime, &readSize,
                sizeof(NavAttIntMsg), (void*) &(ConfigData->scAttBuffer), moduleID);

    /* Calculate Sunline Heading*/
    v3Subtract(ConfigData->scTransBuffer.r_BN_N, ConfigData->sunEphemBuffer.r_BdyZero_N, rDiff_N);
    v3Normalize(rDiff_N, rDiffUnit_N);
    MRP2C(ConfigData->scAttBuffer.sigma_BN, dcm_BN);
    
    m33MultV3(dcm_BN, rDiffUnit_N, rDiff_B);
    
    /*store the output message*/
    v3Copy(rDiff_B, ConfigData->outputSunline.vehSunPntBdy);
    
    WriteMessage(ConfigData->navStateOutMsgId, callTime, sizeof(NavAttIntMsg),
                 &(ConfigData->outputSunline), moduleID);


    return;
}