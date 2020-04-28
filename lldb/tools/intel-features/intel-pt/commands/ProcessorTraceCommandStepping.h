#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STEPPING_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STEPPING_H

#include "../trace/PTManager.h"
#include "ProcessorTraceCommand.h"
#include "lldb/API/SBCommandInterpreter.h"
#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"

enum PTSteppingKind {
  eStepInst = 0,
  eStepOver,
  eReverseStepInst,
  eReverseStepOver,
  eContinue,
  eReverseContinue,
  eStepIn,
  eReverseStepIn,
  eStepOut,
  eReverseStepOut,
};

class ProcessorTraceCommandStepping : public ProcessorTraceCommand {
public:
  ProcessorTraceCommandStepping(std::shared_ptr<intelpt::PTManager> &pt_decoder,
                                PTSteppingKind stepping_kind);

  bool DoExecute(lldb::SBDebugger debugger, char **command,
                 lldb::SBCommandReturnObject &result) override;

  bool SupportAutoRepeat() override;

private:
  std::shared_ptr<intelpt::PTManager> pt_decoder_sp;
  PTSteppingKind m_stepping_kind;
};

class ProcessorTraceReverseStepInst : public ProcessorTraceCommandStepping {
public:
  ProcessorTraceReverseStepInst(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eReverseStepInst) {}

  const char *GetCommandName() { return "reverse-step-inst"; }

  const char *GetAlias() { return "ptrsi"; }

  const char *GetHelp() {
    return "Move the trace position to the previous instruction.";
  }
  const char *GetSyntax() {
    return "processor-trace reverse-step-inst <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace reverse-step-in -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }
};

class ProcessorTraceStepInst : public ProcessorTraceCommandStepping {
public:
  ProcessorTraceStepInst(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eStepInst) {}

  const char *GetCommandName() { return "step-inst"; }

  const char *GetAlias() { return "ptsi"; }

  const char *GetHelp() {
    return "Move the trace position to the nextt instruction.";
  }
  const char *GetSyntax() {
    return "processor-trace step-inst <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace step-in -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }
};

class ProcessorTraceReverseStepOver : public ProcessorTraceCommandStepping {
public:
  ProcessorTraceReverseStepOver(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eReverseStepOver) {}

  const char *GetCommandName() { return "reverse-step-over"; }

  const char *GetHelp() {
    return "Move the trace position over to the previous source-level "
           "position.";
  }

  const char *GetSyntax() {
    return "processor-trace reverse-step-over <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace reverse-step-over -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "ptrn"; }
};

class ProcessorTraceStepOver : public ProcessorTraceCommandStepping {
public:
  ProcessorTraceStepOver(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eStepOver) {}

  const char *GetCommandName() { return "step-over"; }

  const char *GetHelp() {
    return "Move the trace position over to the next source-level position.";
  }

  const char *GetSyntax() {
    return "processor-trace step-over <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace step-over -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "ptn"; }
};

class ProcessorTraceContinue : public ProcessorTraceCommandStepping {
public:
  ProcessorTraceContinue(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eContinue) {}

  const char *GetCommandName() { return "continue"; }

  const char *GetHelp() {
    return "Move the trace position to the end or until a breakpoint is hit.";
  }

  const char *GetSyntax() {
    return "processor-trace continue <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace continue -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "ptc"; }
};

class ProcessorTraceReverseContinue : public ProcessorTraceCommandStepping {
public:
  ProcessorTraceReverseContinue(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eReverseContinue) {}

  const char *GetCommandName() { return "reverse-continue"; }

  const char *GetHelp() {
    return "Move the trace position to the beginning or until a breakpoint is hit.";
  }

  const char *GetSyntax() {
    return "processor-trace reverse-continue <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace reverse-continue -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "ptrc"; }
};

class ProcessorTraceStepIn: public ProcessorTraceCommandStepping {
public:
  ProcessorTraceStepIn(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eStepIn) {}

  const char *GetCommandName() { return "step-in"; }

  const char *GetHelp() {
    return "Move the trace position to the next source-level position stepping into calls";
  }

  const char *GetSyntax() {
    return "processor-trace step-in <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace step-in -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "pts"; }
};

class ProcessorTraceReverseStepIn: public ProcessorTraceCommandStepping {
public:
  ProcessorTraceReverseStepIn(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eReverseStepIn) {}

  const char *GetCommandName() { return "reverse-step-in"; }

  const char *GetHelp() {
    return "Move the trace position to the previous source-level position stepping into calls";
  }

  const char *GetSyntax() {
    return "processor-trace step-in <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace step-in -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "ptrs"; }
};

class ProcessorTraceStepOut: public ProcessorTraceCommandStepping {
public:
  ProcessorTraceStepOut(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eStepOut) {}

  const char *GetCommandName() { return "step-out"; }

  const char *GetHelp() {
    return "Move the trace position to position after the end of the current function";
  }

  const char *GetSyntax() {
    return "processor-trace step-out <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace step-out -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "ptfinish"; }
};

class ProcessorTraceReverseStepOut: public ProcessorTraceCommandStepping {
public:
  ProcessorTraceReverseStepOut(std::shared_ptr<intelpt::PTManager> &pt_decoder)
      : ProcessorTraceCommandStepping(pt_decoder, eReverseStepOut) {}

  const char *GetCommandName() { return "reverse-step-out"; }

  const char *GetHelp() {
    return "Move the trace position to position before the beginning of the current function";
  }

  const char *GetSyntax() {
    return "processor-trace reverse-step-out <cmd-options>\n\n"
           "\rcmd-options Usage:\n"
           "\r  processor-trace reverse-step-out -t [<thread-index>]\n\n"
           "\t\b-t <thread-index>\n"
           "\t    thread index of the thread. If no threads are specified, "
           "the currently selected thread is taken.\n";
  }

  const char *GetAlias() { return "ptrfinish"; }
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_STEPPING_H
