%module openGlClSrp
%{
   #include "openGlClSrp.h"
%}

%include "swig_common_model.i"

%include "../_GeneralModuleFiles/dynamicEffector.h"
%include "../_GeneralModuleFiles/stateData.h"
%include "sys_model.h"
%include "openGlClSrp.h"

%include "../../simMessages/spicePlanetStateSimMsg.h"
%include "../../simMessages/scPlusStatesSimMsg.h"
GEN_SIZEOF(SpicePlanetStateSimMsg);
GEN_SIZEOF(SCPlusStatesSimMsg);

%pythoncode %{
import sys
protectAllClasses(sys.modules[__name__])
%}
