add_lldb_library(lldbIntelPTTrace
  Decoder.cpp
  PTDecoder.cpp

  LINK_LIBS
    liblldb
)
