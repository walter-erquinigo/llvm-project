%module lldbIntelFeatures

%{
#include "lldb/lldb-public.h"
#include "intel-pt/trace/PTManager.h"
using namespace intelpt;
%}

/* Undefine GCC keyword to make Swig happy when processing glibc's stdint.h */
#define __extension__

/* Combined python typemap for all features */
%include "python-typemaps.txt"

/* Feature specific python interface files*/
%include "../intel-pt/interface/PTManager.i"
