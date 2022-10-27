/*
 ISC License

 Copyright (c) 2022, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

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

#ifndef _PLATFORM_ROTATION_
#define _PLATFORM_ROTATION_

#include <stdint.h>
#include "architecture/utilities/bskLogging.h"
#include "cMsgCInterface/AttRefMsg_C.h"
#include "cMsgCInterface/NavAttMsg_C.h"


/*! @brief Top level structure for the sub-module routines. */
typedef struct {

    /* declare module IO interfaces */
    NavAttMsg_C attNavInMsg;                      //!< input msg measured attitude
    AttRefMsg_C attRefOutMsg;                     //!< sample output message

    BSKLogger *bskLogger;                         //!< BSK Logging

}platformRotationConfig;

#ifdef __cplusplus
extern "C" {
#endif

    void SelfInit_platformRotation(platformRotationConfig *configData, int64_t moduleID);
    void Reset_platformRotation(platformRotationConfig *configData, uint64_t callTime, int64_t moduleID);
    void Update_platformRotation(platformRotationConfig *configData, uint64_t callTime, int64_t moduleID);

#ifdef __cplusplus
}
#endif


#endif