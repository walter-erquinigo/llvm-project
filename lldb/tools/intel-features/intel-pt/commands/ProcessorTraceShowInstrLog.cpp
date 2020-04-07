#include <cinttypes>
#include <cstring>

#include "ProcessorTraceShowInstrLog.h"
#include "CommandUtils.h"
#include "lldb/API/SBThread.h"

  ProcessorTraceShowInstrLog::ProcessorTraceShowInstrLog(std::shared_ptr<ptdecoder::PTManager> &pt_decoder)
      : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

  ProcessorTraceShowInstrLog::~ProcessorTraceShowInstrLog() {}

  bool ProcessorTraceShowInstrLog::DoExecute(lldb::SBDebugger debugger, char **command,
                         lldb::SBCommandReturnObject &result) {
    lldb::SBProcess process;
    lldb::SBThread thread;
    if (!GetProcess(debugger, result, process))
      return false;

    // Default initialize API's arguments
    uint32_t offset;
    bool offset_provided = false;
    uint32_t count = m_default_count;
    lldb::tid_t thread_id;

    // Parse command line options
    bool thread_argument_provided = false;
    if (command) {
      for (uint32_t i = 0; command[i]; i++) {
        if (!strcmp(command[i], "-o")) {
          if (!ParseCommandOption(command, result, i, "-o", offset))
            return false;
          offset_provided = true;
        } else if (!strcmp(command[i], "-c")) {
          if (!ParseCommandOption(command, result, i, "-c", count))
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

    size_t loop_count = 1;
    bool entire_process_tracing = false;
    if (thread_id == LLDB_INVALID_THREAD_ID) {
      entire_process_tracing = true;
      loop_count = process.GetNumThreads();
    }

    // Get instruction log and disassemble it
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

      // If offset is not provided then calculate a default offset (to display
      // last 'count' number of instructions)
      if (!offset_provided)
        offset = count - 1;

      // Get the instruction log
      ptdecoder::PTInstructionList insn_list;
      pt_decoder_sp->GetInstructionLogAtOffset(process, thread_id, offset,
                                               count, insn_list, error);
      if (!error.Success()) {
        res.Printf("thread #%" PRIu32 ": tid=%" PRIu64 ", error: %s",
                   thread.GetIndexID(), thread_id, error.GetCString());
        result.AppendMessage(res.GetOutput());
        continue;
      }

      // Disassemble the instruction log
      std::string disassembler_command("dis -c 1 -s ");
      res.Printf("thread #%" PRIu32 ": tid=%" PRIu64 "\n", thread.GetIndexID(),
                 thread_id);
      lldb::SBCommandInterpreter sb_cmnd_interpreter(
          debugger.GetCommandInterpreter());
      lldb::SBCommandReturnObject result_obj;
      for (size_t i = 0; i < insn_list.GetSize(); i++) {
        ptdecoder::PTInstruction insn = insn_list.GetInstructionAtIndex(i);
        uint64_t addr = insn.GetInsnAddress();
        std::string error = insn.GetError();
        if (!error.empty()) {
          res.AppendMessage(error.c_str());
          continue;
        }

        result_obj.Clear();
        std::string complete_disassembler_command =
            disassembler_command + std::to_string(addr);
        sb_cmnd_interpreter.HandleCommand(complete_disassembler_command.c_str(),
                                          result_obj, false);
        std::string result_str(result_obj.GetOutput());
        if (result_str.empty()) {
          lldb::SBCommandReturnObject output;
          output.Printf(" Disassembly not found for address: %" PRIu64, addr);
          res.AppendMessage(output.GetOutput());
          continue;
        }

        // LLDB's disassemble command displays assembly instructions along with
        // the names of the functions they belong to. Parse this result to
        // display only the assembly instructions and not the function names
        // in an instruction log
        std::size_t first_new_line_index = result_str.find_first_of('\n');
        std::size_t last_new_line_index = result_str.find_last_of('\n');
        if (first_new_line_index != last_new_line_index)
          res.AppendMessage((result_str.substr(first_new_line_index + 1,
                                               last_new_line_index -
                                                   first_new_line_index - 1))
                                .c_str());
        else
          res.AppendMessage(
              (result_str.substr(0, result_str.length() - 1)).c_str());
      }
      result.AppendMessage(res.GetOutput());
    }
    result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
    return true;
  }

const char *ProcessorTraceShowInstrLog::GetCommandName() {
  return "show-instr-log";
}

  const char *ProcessorTraceShowInstrLog::GetHelp() {
    return "display a log of assembly instructions executed for a specific thread "
      "or for the whole process.\n"
      "The length of the log to be displayed and the offset in the whole "
      "instruction log from where the log needs to be displayed can also be "
      "provided. The offset is counted from the end of this whole "
      "instruction log which means the last executed instruction is at offset "
      "0 (zero)";
  }

  const char *ProcessorTraceShowInstrLog::GetSyntax() {
    return "processor-trace show-instr-log  <cmd-options>\n\n"
      "\rcmd-options Usage:\n"
      "\r  processor-trace show-instr-log [-o <offset>] [-c <count>] "
      "[<thread-index>]\n\n"
      "\t\b-o <offset>\n"
      "\t    offset in the whole instruction log from where the log will be "
      "displayed. If not specified then a default value will be taken\n\n"
      "\t\b-c <count>\n"
      "\t    number of instructions to be displayed. If not specified then a "
      "default value will be taken\n\n"
      "\t\b<thread-index>\n"
      "\t    thread index of the thread. If no threads are specified, "
      "currently selected thread is taken.\n"
      "\t    Use the thread-index 'all' to show instruction log for all the "
      "threads of the process\n";
  }
