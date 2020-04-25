#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STEP_OVER_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STEP_OVER_H

#include "../trace/PTManager.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

class ProcessorTraceStepOver : public ProcessorTraceCommand {
public:
  ProcessorTraceStepOver(
      std::shared_ptr<intelpt::PTManager> &pt_decoder);

  ~ProcessorTraceStepOver();

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;

  const char *GetCommandName() override;

  const char *GetHelp() override;

  const char *GetSyntax() override;

  const char *GetAlias() override;

private:
  std::shared_ptr<intelpt::PTManager> pt_decoder_sp;
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STEP_OVER_H
