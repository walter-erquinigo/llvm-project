#include <cinttypes>
#include <cstring>
#include <sstream>

#include "CommandUtils.h"
#include "ProcessorTraceReverseStepInst.h"
#include "lldb/API/SBThread.h"

ProcessorTraceReverseStepInst::ProcessorTraceReverseStepInst(
    std::shared_ptr<intelpt::PTManager> &pt_decoder)
    : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

ProcessorTraceReverseStepInst::~ProcessorTraceReverseStepInst() {}

bool ProcessorTraceReverseStepInst::DoExecute(lldb::SBDebugger debugger, char **command,
                                   lldb::SBCommandReturnObject &result) {
  lldb::SBProcess process;
  lldb::SBThread thread;
  if (!GetProcess(debugger, result, process))
    return false;

  // Default initialize API's arguments
  lldb::tid_t thread_id;

  // Parse command line options
  bool thread_argument_provided = false;
  if (command) {
    for (uint32_t i = 0; command[i]; i++) {
      if (!strcmp(command[i], "-t")) {
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

  lldb::SBError error;
  intelpt::PTThreadTrace thread_trace =
      pt_decoder_sp->GetThreadTrace(process, thread_id, error);
  if (!error.Success()) {
    result.AppendMessage(error.GetCString());
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  if (!thread_trace.ReverseStepInst()) {
    result.AppendMessage("error: beginning of trace reached");
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  std::ostringstream backtrace_command;
  backtrace_command << "processor-trace backtrace " << thread_id;
  debugger.GetCommandInterpreter().HandleCommand(
      backtrace_command.str().c_str(), result);

  result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
  return true;
}

const char *ProcessorTraceReverseStepInst::GetCommandName() { return "reverse-step-inst"; }

const char *ProcessorTraceReverseStepInst::GetHelp() {
  return "Move the trace position to the previous instruction.";
}

const char *ProcessorTraceReverseStepInst::GetSyntax() {
  return "processor-trace reverse-step-inst <cmd-options>\n\n"
         "\rcmd-options Usage:\n"
         "\r  processor-trace reverse-step-in -t [<thread-index>]\n\n"
         "\t\b-t <thread-index>\n"
         "\t    thread index of the thread. If no threads are specified, "
         "the currently selected thread is taken.\n";
}

const char *ProcessorTraceReverseStepInst::GetAlias() { return "ptrsi"; }
