#include <cinttypes>

#include "CommandUtils.h"
#include "ProcessorTraceShowTraceOptions.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBThread.h"

ProcessorTraceShowTraceOptions::ProcessorTraceShowTraceOptions(
    std::shared_ptr<ptdecoder::PTDecoder> &pt_decoder)
    : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

ProcessorTraceShowTraceOptions::~ProcessorTraceShowTraceOptions() {}

bool ProcessorTraceShowTraceOptions::DoExecute(lldb::SBDebugger debugger, char **command,
                                   lldb::SBCommandReturnObject &result) {
  lldb::SBProcess process;
  lldb::SBThread thread;
  if (!GetProcess(debugger, result, process))
    return false;

  lldb::tid_t thread_id;

  // Parse command line options
  bool thread_argument_provided = false;
  if (command) {
    for (uint32_t i = 0; command[i]; i++) {
      thread_argument_provided = true;
      if (!ParseCommandArgThread(command, result, process, i, thread_id))
        return false;
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

  size_t loop_count = 1;
  bool entire_process_tracing = false;
  if (thread_id == LLDB_INVALID_THREAD_ID) {
    entire_process_tracing = true;
    loop_count = process.GetNumThreads();
  }

  // Get trace information
  lldb::SBError error;
  lldb::SBCommandReturnObject res;
  for (size_t i = 0; i < loop_count; i++) {
    error.Clear();
    res.Clear();

    if (entire_process_tracing)
      thread = process.GetThreadAtIndex(i);
    else
      thread = process.GetThreadByID(thread_id);
    thread_id = thread.GetThreadID();

    ptdecoder::PTTraceOptions options;
    pt_decoder_sp->GetProcessorTraceInfo(process, thread_id, options, error);
    if (!error.Success()) {
      res.Printf("thread #%" PRIu32 ": tid=%" PRIu64 ", error: %s",
                 thread.GetIndexID(), thread_id, error.GetCString());
      result.AppendMessage(res.GetOutput());
      continue;
    }

    lldb::SBStructuredData data = options.GetTraceParams(error);
    if (!error.Success()) {
      res.Printf("thread #%" PRIu32 ": tid=%" PRIu64 ", error: %s",
                 thread.GetIndexID(), thread_id, error.GetCString());
      result.AppendMessage(res.GetOutput());
      continue;
    }

    lldb::SBStream s;
    error = data.GetAsJSON(s);
    if (!error.Success()) {
      res.Printf("thread #%" PRIu32 ": tid=%" PRIu64 ", error: %s",
                 thread.GetIndexID(), thread_id, error.GetCString());
      result.AppendMessage(res.GetOutput());
      continue;
    }

    res.Printf("thread #%" PRIu32 ": tid=%" PRIu64
               ", trace buffer size=%" PRIu64 ", meta buffer size=%" PRIu64
               ", trace type=%" PRIu32 ", custom trace params=%s",
               thread.GetIndexID(), thread_id, options.GetTraceBufferSize(),
               options.GetMetaDataBufferSize(), options.GetType(), s.GetData());
    result.AppendMessage(res.GetOutput());
  }
  result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
  return true;
}

const char *ProcessorTraceShowTraceOptions::GetCommandName() {
  return "show-trace-options";
}

const char *ProcessorTraceShowTraceOptions::GetHelp() {
  return "display all the information regarding Intel(R) Processor Trace for a "
         "specific thread or for the whole process.\n"
         "The information contains trace buffer size and configuration options"
         " of Intel(R) Processor Trace.";
}

const char *ProcessorTraceShowTraceOptions::GetSyntax() {
  return "processor-trace show-trace-options <cmd-options>\n\n"
         "\rcmd-options Usage:\n"
         "\r  processor-trace show-trace-options [<thread-index>]\n\n"
         "\t\b<thread-index>\n"
         "\t    thread index of the thread. If no threads are specified, "
         "currently selected thread is taken.\n"
         "\t    Use the thread-index 'all' to display information for all "
         "threads "
         "of the process\n";
}
