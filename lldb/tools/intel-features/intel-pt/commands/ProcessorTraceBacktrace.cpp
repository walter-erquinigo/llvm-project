#include <cinttypes>
#include <cstring>

#include "CommandUtils.h"
#include "ProcessorTraceBacktrace.h"
#include "lldb/API/SBThread.h"

using namespace lldb;

ProcessorTraceBacktrace::ProcessorTraceBacktrace(
    std::shared_ptr<intelpt::PTManager> &pt_decoder)
    : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

ProcessorTraceBacktrace::~ProcessorTraceBacktrace() {}

bool ProcessorTraceBacktrace::DoExecute(lldb::SBDebugger debugger, char **command,
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
    for (uint32_t i = 0; command[i]; i++)
      if (!strcmp(command[i], "-t")) {
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

  lldb::SBError error;
  intelpt::PTThreadTrace thread_trace =
      pt_decoder_sp->GetThreadTrace(process, thread_id, error);
  if (!error.Success()) {
    result.AppendMessage(error.GetCString());
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  lldb::SBTarget target = debugger.GetSelectedTarget();
  const char *yellowColor = "\e[33m";
  const char *defaultColor = "\e[39m";
  const char *cyanColor = "\e[96m";

  result.Printf("thread #%lu\n", thread_id);

  int prev_segment_id = -1;
  for (size_t i = 0; i < thread_trace.GetNumFrames(); i++) {
    intelpt::PTFunctionSegment segment = thread_trace.GetFrameAtIndex(i);
    lldb::addr_t load_address = LLDB_INVALID_ADDRESS;
    if (i == 0)
      load_address = thread_trace.GetCurrentInstruction().GetInsnAddress();
    else if ((int)segment.GetID() < prev_segment_id) {
      // We know when this segment called the previous one
      load_address = segment.GetEndLoadAddress();
    }

    result.Printf("\t frame #%zu: ", i);

    if (load_address != LLDB_INVALID_ADDRESS)
      result.Printf("[%10zu] %s0x%16.16" PRIx64 "%s",
                    segment.GetFirstInstruction().GetID(), yellowColor,
                    load_address, defaultColor);
    else
      result.Printf("[ .         ] %s0x%16.16" PRIx64 "%s", yellowColor,
                    load_address, defaultColor);

    result.Printf(" ");

    lldb::addr_t valid_load_address = load_address == LLDB_INVALID_ADDRESS ? segment.GetEndLoadAddress() : load_address;
    lldb::SBAddress valid_address(valid_load_address, target);
    lldb::SBModule module = valid_address.GetModule();
    lldb::SBFileSpec module_file_spec = module.GetFileSpec();

    if (module_file_spec.IsValid())
      result.Printf("%s`", module_file_spec.GetFilename());
    result.Printf("%s", segment.GetDisplayName());

    if (load_address != LLDB_INVALID_ADDRESS) {
      lldb::SBLineEntry line_entry = valid_address.GetLineEntry();
      lldb::SBFileSpec file_spec = line_entry.GetFileSpec();
      lldb::addr_t offset = valid_address.GetOffset();

      if (file_spec.IsValid() && line_entry.IsValid()) {
        result.Printf(" at %s%s%s:%s%d%s", cyanColor, file_spec.GetFilename(),
              defaultColor, yellowColor, line_entry.GetLine(), defaultColor);
      } else if (offset > 0 && offset != LLDB_INVALID_ADDRESS)
        result.Printf(" + %lu", offset);
    }
    result.Printf("\n");
    prev_segment_id = segment.GetID();
  }

  result.SetStatus(lldb::eReturnStatusSuccessFinishResult);
  return true;
}

const char *ProcessorTraceBacktrace::GetCommandName() { return "backtrace"; }

const char *ProcessorTraceBacktrace::GetHelp() {
  return "Print the backtrace";
}

const char *ProcessorTraceBacktrace::GetSyntax() {
  return "processor-trace ProcessorTraceBacktrace <cmd-options>\n\n"
         "\rcmd-options Usage:\n"
         "\r  processor-trace ProcessorTraceBacktrace -t [<thread-index>]\n\n"
         "\t\b-t <thread-index>\n"
         "\t    thread index of the thread. If no threads are specified, "
         "the currently selected thread is taken.\n";
}
