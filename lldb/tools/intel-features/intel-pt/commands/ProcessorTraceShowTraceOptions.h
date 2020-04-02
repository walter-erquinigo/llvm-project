#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_TRACE_OPTIONS_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_TRACE_OPTIONS_H

#include "../trace/PTDecoder.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

class ProcessorTraceShowTraceOptions : public ProcessorTraceCommand {
public:
  ProcessorTraceShowTraceOptions(std::shared_ptr<ptdecoder::PTDecoder> &pt_decoder);

  ~ProcessorTraceShowTraceOptions();

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;

  const char *GetCommandName() override;

  const char *GetHelp() override;

  const char *GetSyntax() override;

private:
  std::shared_ptr<ptdecoder::PTDecoder> pt_decoder_sp;
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_TRACE_OPTIONS_H
