/*
 ISC License

 Copyright (c) 2016-2018, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

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
%module headingSuKF
%{
   #include "headingSuKF.h"
   #include "../_GeneralModuleFiles/ukfUtilities.h"
%}

%include "swig_conly_data.i"
%constant void Update_headingSuKF(void*, uint64_t, uint64_t);
%ignore Update_headingSuKF;
%constant void SelfInit_headingSuKF(void*, uint64_t);
%ignore SelfInit_headingSuKF;
%constant void CrossInit_headingSuKF(void*, uint64_t);
%ignore CrossInit_headingSuKF;
%constant void Reset_headingSuKF(void*, uint64_t, uint64_t);
%ignore Reset_headingSuKF;
GEN_SIZEOF(HeadingFilterFswMsg);
GEN_SIZEOF(VehicleConfigFswMsg);
GEN_SIZEOF(OpnavFswMsg);
GEN_SIZEOF(HeadingSuKFConfig);
%include "headingSuKF.h"
%include "../_GeneralModuleFiles/ukfUtilities.h"
%include "../../fswMessages/vehicleConfigFswMsg.h"
%include "../../fswMessages/headingFilterFswMsg.h"
%include "../../fswMessages/opnavFswMsg.h"

%pythoncode %{
import sys
protectAllClasses(sys.modules[__name__])
%}
