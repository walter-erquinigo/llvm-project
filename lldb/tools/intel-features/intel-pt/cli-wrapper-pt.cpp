//===-- cli-wrapper-pt.cpp -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// CLI Wrapper of PTDecoder Tool to enable it to be used through LLDB's CLI. The
// wrapper provides a new command called processor-trace with 4 child
// subcommands as follows:
// processor-trace start
// processor-trace stop
// processor-trace show-trace-options
// processor-trace show-instr-log
//
//===----------------------------------------------------------------------===//

#include "cli-wrapper-pt.h"
#include "commands/ProcessorTraceShowInstrLog.h"
#include "commands/ProcessorTraceShowTraceOptions.h"
#include "commands/ProcessorTraceStart.h"
#include "commands/ProcessorTraceStop.h"
#include "lldb/API/SBCommandInterpreter.h"

bool PTPluginInitialize(lldb::SBDebugger &debugger) {
  lldb::SBCommandInterpreter interpreter = debugger.GetCommandInterpreter();
  lldb::SBCommand proc_trace = interpreter.AddMultiwordCommand(
      "processor-trace", "Intel(R) Processor Trace for thread/process");
  std::shared_ptr<ptdecoder::PTDecoder> PTDecoderSP(
      new ptdecoder::PTDecoder(debugger));

  ProcessorTraceCommand *commands[] = {
      new ProcessorTraceStart(PTDecoderSP),
      new ProcessorTraceStop(PTDecoderSP),
      new ProcessorTraceShowInstrLog(PTDecoderSP),
      new ProcessorTraceShowTraceOptions(PTDecoderSP),
  };

  for (ProcessorTraceCommand *command : commands)
    proc_trace.AddCommand(command->GetCommandName(), command,
                          command->GetHelp(), command->GetSyntax());

  return true;
}
