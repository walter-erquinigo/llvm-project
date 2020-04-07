#include "ProcessorTraceStop.h"
#include "lldb/API/SBThread.h"
#include "CommandUtils.h"

  ProcessorTraceStop::ProcessorTraceStop(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

  ProcessorTraceStop::~ProcessorTraceStop() {}

  bool ProcessorTraceStop::DoExecute(lldb::SBDebugger debugger, char **command,
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

    // Stop trace
    lldb::SBError error;
    pt_decoder_sp->StopProcessorTrace(process, error, thread_id);
    if (!error.Success()) {
      result.Printf("error: %s\n", error.GetCString());
      result.SetStatus(lldb::eReturnStatusFailed);
      return false;
    }
    result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
    return true;
  }

const char *ProcessorTraceStop::GetCommandName() {
  return "stop";
}

  const char *ProcessorTraceStop::GetHelp() {
    return "stop Intel(R) Processor Trace on a specific thread or on whole process";
  }

  const char *ProcessorTraceStop::GetSyntax() {
    return "processor-trace stop  <cmd-options>\n\n"
      "\rcmd-options Usage:\n"
      "\r  processor-trace stop [<thread-index>]\n\n"
      "\t\b<thread-index>\n"
      "\t    thread index of the thread. If no threads are specified, "
      "currently selected thread is taken.\n"
      "\t    Use the thread-index 'all' to stop tracing the whole process\n";
  }
