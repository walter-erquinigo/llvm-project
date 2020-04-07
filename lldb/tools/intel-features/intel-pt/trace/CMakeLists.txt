add_lldb_library(lldbIntelPTTrace
  Decoder.cpp
  PTManager.cpp

  LINK_LIBS
    liblldb
)
