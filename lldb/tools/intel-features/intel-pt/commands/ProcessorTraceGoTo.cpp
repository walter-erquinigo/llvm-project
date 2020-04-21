#include <cinttypes>
#include <cstring>

#include "ProcessorTraceGoTo.h"
#include "CommandUtils.h"
#include "lldb/API/SBThread.h"

  ProcessorTraceGoTo::ProcessorTraceGoTo(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

  ProcessorTraceGoTo::~ProcessorTraceGoTo() {}

  bool ProcessorTraceGoTo::DoExecute(lldb::SBDebugger debugger, char **command,
                         lldb::SBCommandReturnObject &result) {
    lldb::SBProcess process;
    lldb::SBThread thread;
    if (!GetProcess(debugger, result, process))
      return false;

    // Default initialize API's arguments
    size_t insn_index;
    lldb::tid_t thread_id;

    // Parse command line options
    bool thread_argument_provided = false;
    if (command) {
      for (uint32_t i = 0; command[i]; i++) {
        if (!strcmp(command[i], "-t")) {
          thread_argument_provided = true;
          if (!ParseCommandArgThread(command, result, process, i, thread_id))
            return false;
        } else {
          if (!ParseCommandOption2(command, result, i, llvm::None, insn_index))
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

    size_t pos;
    lldb::SBError error;
    pt_decoder_sp->GetIteratorPosition(process, thread_id, pos, error);
    printf("before position %zu\n", pos);
    pt_decoder_sp->SetIteratorPosition(process, thread_id, insn_index, error);
    if (!error.Success()) {
      result.AppendMessage(error.GetCString());
      result.SetStatus(lldb::eReturnStatusFailed);
      return false;
    }

    pt_decoder_sp->GetIteratorPosition(process, thread_id, pos, error);
    printf("after position %zu\n", pos);
    result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
    return true;
  }

  const char *ProcessorTraceGoTo::GetCommandName() {
    return "go-to";
  }

  const char *ProcessorTraceGoTo::GetHelp() {
    return "Move the trace position to the specified instruction index.";
  }

  const char *ProcessorTraceGoTo::GetSyntax() {
    return "processor-trace go-to <cmd-options>\n\n"
      "\rcmd-options Usage:\n"
      "\r  processor-trace go-to -t [<thread-index>] [<instruction-index>]\n\n"
      "\t\b-t <thread-index>\n"
      "\t    thread index of the thread. If no threads are specified, "
      "the currently selected thread is taken.\n\n"
      "\t\b<instruction-index>\n"
      "\t    index of instruction in the trace\n";
  }
