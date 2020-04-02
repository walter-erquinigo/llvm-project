#ifndef LLDB_TOOLS_INTEL_PT_COMMAND_UTILS_H
#define LLDB_TOOLS_INTEL_PT_COMMAND_UTILS_H

#include <string>

#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBProcess.h"

bool GetProcess(lldb::SBDebugger &debugger, lldb::SBCommandReturnObject &result,
                lldb::SBProcess &process);

bool ParseCommandOption(char **command, lldb::SBCommandReturnObject &result,
                        uint32_t &index, const std::string &arg,
                        uint32_t &parsed_result);

bool ParseCommandArgThread(char **command, lldb::SBCommandReturnObject &result,
                           lldb::SBProcess &process, uint32_t &index,
                           lldb::tid_t &thread_id);

#endif // LLDB_TOOLS_INTEL_PT_COMMAND_UTILS_H
