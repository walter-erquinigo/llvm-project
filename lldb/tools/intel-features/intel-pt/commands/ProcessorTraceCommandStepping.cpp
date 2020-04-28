#include <cinttypes>
#include <cstring>
#include <sstream>

#include "CommandUtils.h"
#include "ProcessorTraceCommandStepping.h"
#include "lldb/API/SBThread.h"

ProcessorTraceCommandStepping::ProcessorTraceCommandStepping(
    std::shared_ptr<intelpt::PTManager> &pt_decoder,
    PTSteppingKind stepping_kind)
    : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder),
      m_stepping_kind(stepping_kind) {}

bool ProcessorTraceCommandStepping::SupportAutoRepeat() {
  return true;
}

bool ProcessorTraceCommandStepping::DoExecute(
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

  bool did_move = false;
  switch (m_stepping_kind) {
  case eStepInst:
    did_move = thread_trace.StepInst();
    break;
  case eReverseStepInst:
    did_move = thread_trace.ReverseStepInst();
    break;
  case eStepOver:
    did_move = thread_trace.StepOver();
    break;
  case eReverseStepOver:
    did_move = thread_trace.ReverseStepOver();
    break;
  case eContinue:
    did_move = thread_trace.Continue();
    break;
  case eReverseContinue:
    did_move = thread_trace.ReverseContinue();
    break;
  }

  if (!did_move) {
    result.AppendMessage("error: end of trace reached");
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  std::ostringstream source_command;
  source_command << "source list -a "
                 << thread_trace.GetCurrentInstruction().GetInsnAddress();
  debugger.GetCommandInterpreter().HandleCommand(source_command.str().c_str(),
                                                 result);

  result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
  return true;
}
