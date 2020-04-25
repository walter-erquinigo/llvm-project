add_lldb_library(lldbIntelPTCommands
  CommandUtils.cpp
  ProcessorTraceBacktrace.cpp
  ProcessorTraceGoTo.cpp
  ProcessorTraceCommandStepping.cpp
  ProcessorTraceShowFunctionCallHistory.cpp
  ProcessorTraceShowInstrLog.cpp
  ProcessorTraceShowTraceOptions.cpp
  ProcessorTraceStart.cpp
  ProcessorTraceStop.cpp

  LINK_LIBS
    liblldb
    lldbIntelPTTrace
)
