#include <cinttypes>
#include <cstring>
#include <sstream>

#include "CommandUtils.h"
#include "ProcessorTraceStepOver.h"
#include "lldb/API/SBThread.h"

ProcessorTraceStepOver::ProcessorTraceStepOver(
    std::shared_ptr<intelpt::PTManager> &pt_decoder)
    : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

ProcessorTraceStepOver::~ProcessorTraceStepOver() {}

bool ProcessorTraceStepOver::DoExecute(
    lldb::SBDebugger debugger, char **command,
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

  if (!thread_trace.StepOver()) {
    result.AppendMessage("error: beginning of trace reached");
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  std::ostringstream source_command;
  source_command << "source list -a " << thread_trace.GetCurrentInstruction().GetInsnAddress();
  debugger.GetCommandInterpreter().HandleCommand(
      source_command.str().c_str(), result);

  result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
  return true;
}

const char *ProcessorTraceStepOver::GetCommandName() {
  return "step-over";
}

const char *ProcessorTraceStepOver::GetHelp() {
  return "Move the trace position over to the next source-level position.";
}

const char *ProcessorTraceStepOver::GetSyntax() {
  return "processor-trace step-over <cmd-options>\n\n"
         "\rcmd-options Usage:\n"
         "\r  processor-trace step-over -t [<thread-index>]\n\n"
         "\t\b-t <thread-index>\n"
         "\t    thread index of the thread. If no threads are specified, "
         "the currently selected thread is taken.\n";
}

const char *ProcessorTraceStepOver::GetAlias() { return "ptn"; }
