#ifndef LLDB_TOOLS_INTEL_PT_COMMAND_UTILS_H
#define LLDB_TOOLS_INTEL_PT_COMMAND_UTILS_H

#include <string>
#include <sstream>

#include "lldb/API/SBCommandReturnObject.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBProcess.h"
#include "llvm/ADT/Optional.h"

bool GetProcess(lldb::SBDebugger &debugger, lldb::SBCommandReturnObject &result,
                lldb::SBProcess &process);

template<typename T>
bool ParseCommandOption2(char **command, lldb::SBCommandReturnObject &result,
                        uint32_t &index, const llvm::Optional<std::string> &arg,
                        T &parsed_result) {
                            if (arg) {
    if (!command[++index]) {
      result.Printf("error: option \"%s\" requires an argument\n", arg->c_str());
      result.SetStatus(lldb::eReturnStatusFailed);
      return false;
    }
  }

  errno = 0;
  std::istringstream iss(command[index]);
  if (!(iss >> parsed_result) || !iss.eof()) {
    result.Printf("error: invalid value \"%s\" provided",
                  command[index]);
    if (arg) {
     result.Printf("for option \"%s\"", arg->c_str());
    }
    result.Printf("\n");
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }
  return true;
                        }

bool ParseCommandOption(char **command, lldb::SBCommandReturnObject &result,
                        uint32_t &index, const std::string &arg,
                        uint32_t &parsed_result);

bool ParseCommandArgThread(char **command, lldb::SBCommandReturnObject &result,
                           lldb::SBProcess &process, uint32_t &index,
                           lldb::tid_t &thread_id);

#endif // LLDB_TOOLS_INTEL_PT_COMMAND_UTILS_H
