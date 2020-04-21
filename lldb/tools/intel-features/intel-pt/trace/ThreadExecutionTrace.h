#pragma once

#include "lldb/API/SBThread.h"
#include "Decoder.h"

namespace intelpt_private {

class ThreadExecutionTrace: public lldb::SBExecutionTracePluginInterface {
 public:
  ThreadExecutionTrace(Decoder *decoder);

  bool CanDoReverse();

 private:
  Decoder *m_decoder_ptr;
};

}
