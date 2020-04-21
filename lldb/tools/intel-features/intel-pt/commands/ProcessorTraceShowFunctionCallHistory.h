#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_FUNCTION_CALL_HISTORY_LOG_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_FUNCTION_CALL_HISTORY_LOG_H

#include "../trace/PTManager.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

class ProcessorTraceShowFunctionCallHistory : public ProcessorTraceCommand {
public:
  ProcessorTraceShowFunctionCallHistory(
      std::shared_ptr<intelpt::PTManager> &pt_decoder);

  ~ProcessorTraceShowFunctionCallHistory();

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;

  const char *GetCommandName() override;

  const char *GetHelp() override;

  const char *GetSyntax() override;

private:
  std::shared_ptr<intelpt::PTManager> pt_decoder_sp;
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_FUNCTION_CALL_HISTORY_LOG_H
