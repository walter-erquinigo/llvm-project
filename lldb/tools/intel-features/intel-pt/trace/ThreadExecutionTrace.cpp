#include "ThreadExecutionTrace.h"

using namespace intelpt_private;

ThreadExecutionTrace::ThreadExecutionTrace(Decoder *decoder): m_decoder_ptr(decoder) {}

bool ThreadExecutionTrace:: CanDoReverse() {
    return true;
}
