#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_INSTR_LOG_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_INSTR_LOG_H

#include "../trace/PTDecoder.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

class ProcessorTraceShowInstrLog : public ProcessorTraceCommand {
public:
  ProcessorTraceShowInstrLog(std::shared_ptr<ptdecoder::PTDecoder> &pt_decoder);

  ~ProcessorTraceShowInstrLog();

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                         lldb::SBCommandReturnObject &result) override;

  const char *GetCommandName() override;

  const char *GetHelp() override;

  const char *GetSyntax() override;

private:
  std::shared_ptr<ptdecoder::PTDecoder> pt_decoder_sp;
  const uint32_t m_default_count = 10;
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_SHOW_INSTR_LOG_H
