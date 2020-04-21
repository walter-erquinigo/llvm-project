#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_GO_TO_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_GO_TO_H

#include "../trace/PTManager.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

class ProcessorTraceGoTo : public ProcessorTraceCommand {
public:
  ProcessorTraceGoTo(std::shared_ptr<intelpt::PTManager> &pt_decoder);

  ~ProcessorTraceGoTo();

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                         lldb::SBCommandReturnObject &result) override;

  const char *GetCommandName() override;

  const char *GetHelp() override;

  const char *GetSyntax() override;

private:
  std::shared_ptr<intelpt::PTManager> pt_decoder_sp;
  const uint32_t m_default_count = 10;
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_GO_TO_H
