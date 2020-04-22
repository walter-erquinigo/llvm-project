add_lldb_library(lldbIntelPTCommands
  CommandUtils.cpp
  ProcessorTraceBacktrace.cpp
  ProcessorTraceGoTo.cpp
  ProcessorTraceShowFunctionCallHistory.cpp
  ProcessorTraceShowInstrLog.cpp
  ProcessorTraceShowTraceOptions.cpp
  ProcessorTraceStart.cpp
  ProcessorTraceStop.cpp

  LINK_LIBS
    liblldb
    lldbIntelPTTrace
)
