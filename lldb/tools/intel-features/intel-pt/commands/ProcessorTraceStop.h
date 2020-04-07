#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STOP_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STOP_H

#include "../trace/PTManager.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

class ProcessorTraceStop : public ProcessorTraceCommand {
public:
  ProcessorTraceStop(std::shared_ptr<ptdecoder::PTManager> &pt_decoder);

  ~ProcessorTraceStop();

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                         lldb::SBCommandReturnObject &result) override;

  const char *GetCommandName() override;

  const char *GetHelp() override;

  const char *GetSyntax() override;

private:
  std::shared_ptr<ptdecoder::PTManager> pt_decoder_sp;
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STOP_H
