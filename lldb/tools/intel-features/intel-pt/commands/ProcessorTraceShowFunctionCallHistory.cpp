#include <cinttypes>

#include "ProcessorTraceShowFunctionCallHistory.h"

#include "CommandUtils.h"
#include "lldb/API/SBAddress.h"
#include "lldb/API/SBThread.h"

using namespace intelpt;
using namespace lldb;

ProcessorTraceShowFunctionCallHistory::ProcessorTraceShowFunctionCallHistory(
    std::shared_ptr<PTManager> &pt_decoder)
    : ProcessorTraceCommand(), pt_decoder_sp(pt_decoder) {}

ProcessorTraceShowFunctionCallHistory::
    ~ProcessorTraceShowFunctionCallHistory() {}

bool ProcessorTraceShowFunctionCallHistory::DoExecute(
    SBDebugger debugger, char **command, SBCommandReturnObject &result) {
  SBProcess process;
  if (!GetProcess(debugger, result, process))
    return false;

  SBThread thread = process.GetSelectedThread();
  if (!thread.IsValid()) {
    result.Printf("error: invalid current selected thread\n");
    result.SetStatus(eReturnStatusFailed);
    return false;
  }

  SBError error;

  PTThreadTrace thread_trace =
      pt_decoder_sp->GetThreadTrace(process, thread.GetThreadID(), error);

  if (!error.Success()) {
    result.Printf("thread #%" PRIu32 ": tid=%" PRIu64 ", error: %s",
                  thread.GetIndexID(), thread.GetThreadID(),
                  error.GetCString());
    result.SetStatus(eReturnStatusFailed);
    return false;
  }

  PTFunctionCallTree call_tree = thread_trace.GetFunctionCallTree();

  SBTarget target = debugger.GetSelectedTarget();
  const char *yellowColor = "\e[33m";
  const char *defaultColor = "\e[39m";
  const char *cyanColor = "\e[96m";

  for (size_t i = 0; i < call_tree.GetSize(); i++) {
    auto segment = call_tree.GetFunctionSegmentAtIndex(i);
    lldb::addr_t load_address = segment.GetStartLoadAddress();
    printf("[%10zu] %s0x%16.16" PRIx64 ": %s",
           segment.GetFirstInstruction().GetID(), yellowColor, load_address,
           defaultColor);
    for (int j = 0; j < segment.GetLevel(); j++)
      printf(" ");

    lldb::SBAddress address(load_address, target);

    SBModule module = address.GetModule();
    lldb::SBFileSpec module_file_spec = module.GetFileSpec();
    if (module_file_spec.IsValid())
      printf("%s`", module_file_spec.GetFilename());
    printf("%s", segment.GetDisplayName());

    lldb::SBLineEntry line_entry = address.GetLineEntry();
    lldb::SBFileSpec file_spec = line_entry.GetFileSpec();
    lldb::addr_t offset = address.GetOffset();

    if (file_spec.IsValid() && line_entry.IsValid()) {
      printf(" at %s%s%s:%s%d%s", cyanColor, file_spec.GetFilename(),
             defaultColor, yellowColor, line_entry.GetLine(), defaultColor);
    } else if (offset > 0 && offset != LLDB_INVALID_ADDRESS) {
      printf(" + %lu", offset);
    }

    printf("\n");
  }

  result.SetStatus(eReturnStatusSuccessFinishResult);
  return true;
}

const char *ProcessorTraceShowFunctionCallHistory::GetCommandName() {
  return "show-function-call-history";
}

const char *ProcessorTraceShowFunctionCallHistory::GetHelp() { return ""; }

const char *ProcessorTraceShowFunctionCallHistory::GetSyntax() { return ""; }
