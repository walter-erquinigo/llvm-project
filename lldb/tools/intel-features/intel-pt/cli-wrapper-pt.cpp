//===-- cli-wrapper-pt.cpp -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// CLI Wrapper of PTManager Tool to enable it to be used through LLDB's CLI. The
// wrapper provides a new command called processor-trace with 4 child
// subcommands as follows:
// processor-trace start
// processor-trace stop
// processor-trace show-trace-options
// processor-trace show-instr-log
//
//===----------------------------------------------------------------------===//

#include <sstream>

#include "cli-wrapper-pt.h"
#include "commands/ProcessorTraceBacktrace.h"
#include "commands/ProcessorTraceCommandStepping.h"
#include "commands/ProcessorTraceGoTo.h"
#include "commands/ProcessorTraceShowFunctionCallHistory.h"
#include "commands/ProcessorTraceShowInstrLog.h"
#include "commands/ProcessorTraceShowTraceOptions.h"
#include "commands/ProcessorTraceStart.h"
#include "commands/ProcessorTraceStop.h"
#include "lldb/API/SBCommandInterpreter.h"

bool PTPluginInitialize(lldb::SBDebugger &debugger) {
  lldb::SBCommandInterpreter interpreter = debugger.GetCommandInterpreter();
  lldb::SBCommand proc_trace = interpreter.AddMultiwordCommand(
      "processor-trace", "Intel(R) Processor Trace for thread/process");
  interpreter.AddAlias("processor-trace", "pt");

  std::shared_ptr<intelpt::PTManager> PTManagerSP(
      new intelpt::PTManager(debugger));

  ProcessorTraceCommand *commands[] = {
      new ProcessorTraceStart(PTManagerSP),
      new ProcessorTraceStop(PTManagerSP),
      new ProcessorTraceShowInstrLog(PTManagerSP),
      new ProcessorTraceShowTraceOptions(PTManagerSP),
      new ProcessorTraceShowFunctionCallHistory(PTManagerSP),
      new ProcessorTraceBacktrace(PTManagerSP),
      new ProcessorTraceGoTo(PTManagerSP),
      new ProcessorTraceReverseStepInst(PTManagerSP),
      new ProcessorTraceStepInst(PTManagerSP),
      new ProcessorTraceReverseStepOver(PTManagerSP),
      new ProcessorTraceStepOver(PTManagerSP),
  };

  for (ProcessorTraceCommand *command : commands) {
    proc_trace.AddCommand(command->GetCommandName(), command,
                          command->GetHelp(), command->GetSyntax());
    const char *alias = command->GetAlias();
    if (alias) {
      std::ostringstream oss;
      oss << "processor-trace " << command->GetCommandName();
      interpreter.AddAlias(oss.str().c_str(), alias);
    }
  }

  return true;
}
