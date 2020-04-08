add_lldb_library(lldbIntelPTTrace
  Decoder.cpp
  FunctionCallTreeBuilder.cpp
  FunctionSegment.cpp
  Instruction.cpp
  PTManager.cpp

  LINK_LIBS
    liblldb
)
