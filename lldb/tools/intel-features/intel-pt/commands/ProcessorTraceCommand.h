#ifndef LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_COMMAND_H
#define LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_COMMAND_H

#include "lldb/API/SBCommandInterpreter.h"

class ProcessorTraceCommand: public lldb::SBCommandPluginInterface {
 public:
  virtual const char *GetCommandName() = 0;

  virtual const char *GetHelp() = 0;

  virtual const char *GetSyntax() = 0;

  virtual const char *GetAlias() { return nullptr; }

  virtual bool SupportAutoRepeat() { return false; }
};

#endif // LLDB_TOOLS_INTEL_PT_PROCESSOR_TRACE_COMMAND_H
