#include <cstring>

#include "CommandUtils.h"
#include "ProcessorTraceStart.h"
#include "lldb/API/SBProcess.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBStructuredData.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBThread.h"

ProcessorTraceStart::ProcessorTraceStart(
    std::shared_ptr<ptdecoder::PTDecoder> &pt_decoder)
    : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

ProcessorTraceStart::~ProcessorTraceStart() {}

bool ProcessorTraceStart::DoExecute(lldb::SBDebugger debugger, char **command,
                                    lldb::SBCommandReturnObject &result) {
  lldb::SBProcess process;
  lldb::SBThread thread;
  if (!GetProcess(debugger, result, process))
    return false;

  // Default initialize API's arguments
  lldb::SBTraceOptions lldb_SBTraceOptions;
  uint32_t trace_buffer_size = m_default_trace_buff_size;
  lldb::tid_t thread_id;

  // Parse Command line options
  bool thread_argument_provided = false;
  if (command) {
    for (uint32_t i = 0; command[i]; i++) {
      if (!strcmp(command[i], "-b")) {
        if (!ParseCommandOption(command, result, i, "-b", trace_buffer_size))
          return false;
      } else {
        thread_argument_provided = true;
        if (!ParseCommandArgThread(command, result, process, i, thread_id))
          return false;
      }
    }
  }

  if (!thread_argument_provided) {
    thread = process.GetSelectedThread();
    if (!thread.IsValid()) {
      result.Printf("error: invalid current selected thread\n");
      result.SetStatus(lldb::eReturnStatusFailed);
      return false;
    }
    thread_id = thread.GetThreadID();
  }

  if (trace_buffer_size > m_max_trace_buff_size)
    trace_buffer_size = m_max_trace_buff_size;

  // Set API's arguments with parsed values
  lldb_SBTraceOptions.setType(lldb::TraceType::eTraceTypeProcessorTrace);
  lldb_SBTraceOptions.setTraceBufferSize(trace_buffer_size);
  lldb_SBTraceOptions.setMetaDataBufferSize(0);
  lldb_SBTraceOptions.setThreadID(thread_id);
  lldb::SBStream sb_stream;
  sb_stream.Printf("{\"trace-tech\":\"intel-pt\"}");
  lldb::SBStructuredData custom_params;
  lldb::SBError error = custom_params.SetFromJSON(sb_stream);
  if (!error.Success()) {
    result.Printf("error: %s\n", error.GetCString());
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }
  lldb_SBTraceOptions.setTraceParams(custom_params);

  // Start trace
  pt_decoder_sp->StartProcessorTrace(process, lldb_SBTraceOptions, error);
  if (!error.Success()) {
    result.Printf("error: %s\n", error.GetCString());
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }
  result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
  return true;
}

const char *ProcessorTraceStart::GetCommandName() { return "start"; }

const char *ProcessorTraceStart::GetHelp() {
  return "start Intel(R) Processor Trace on a "
         "specific thread or on the whole process";
}

const char *ProcessorTraceStart::GetSyntax() {
  return "processor-trace start  <cmd-options>\n\n"
         "\rcmd-options Usage:\n"
         "\r  processor-trace start [-b <buffer-size>] [<thread-index>]\n\n"
         "\t\b-b <buffer-size>\n"
         "\t    size of the trace buffer to store the trace data. If not "
         "specified then a default value will be taken\n\n"
         "\t\b<thread-index>\n"
         "\t    thread index of the thread. If no threads are specified, "
         "currently selected thread is taken.\n"
         "\t    Use the thread-index 'all' to start tracing the whole "
         "process\n";
}
