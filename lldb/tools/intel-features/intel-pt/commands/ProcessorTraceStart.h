#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_START_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_START_H

#include "../trace/PTDecoder.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

class ProcessorTraceStart : public ProcessorTraceCommand {
public:
  ProcessorTraceStart(std::shared_ptr<ptdecoder::PTDecoder> &pt_decoder);

  ~ProcessorTraceStart();

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;

  const char *GetCommandName() override;

  const char *GetHelp() override;

  const char *GetSyntax() override;

private:
  std::shared_ptr<ptdecoder::PTDecoder> pt_decoder_sp;
  const uint32_t m_max_trace_buff_size = 0x3fff;
  const uint32_t m_default_trace_buff_size = 4096;
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_START_H
