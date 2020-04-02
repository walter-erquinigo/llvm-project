#include <cstring>

#include "CommandUtils.h"
#include "lldb/API/SBThread.h"

bool GetProcess(lldb::SBDebugger &debugger, lldb::SBCommandReturnObject &result,
                lldb::SBProcess &process) {
  if (!debugger.IsValid()) {
    result.Printf("error: invalid debugger\n");
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  lldb::SBTarget target = debugger.GetSelectedTarget();
  if (!target.IsValid()) {
    result.Printf("error: invalid target inside debugger\n");
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  process = target.GetProcess();
  if (!process.IsValid() ||
      (process.GetState() == lldb::StateType::eStateDetached) ||
      (process.GetState() == lldb::StateType::eStateExited) ||
      (process.GetState() == lldb::StateType::eStateInvalid)) {
    result.Printf("error: invalid process inside debugger's target\n");
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  return true;
}

bool ParseCommandOption(char **command, lldb::SBCommandReturnObject &result,
                        uint32_t &index, const std::string &arg,
                        uint32_t &parsed_result) {
  char *endptr;
  if (!command[++index]) {
    result.Printf("error: option \"%s\" requires an argument\n", arg.c_str());
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }

  errno = 0;
  unsigned long output = strtoul(command[index], &endptr, 0);
  if ((errno != 0) || (*endptr != '\0')) {
    result.Printf("error: invalid value \"%s\" provided for option \"%s\"\n",
                  command[index], arg.c_str());
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }
  if (output > UINT32_MAX) {
    result.Printf("error: value \"%s\" for option \"%s\" exceeds UINT32_MAX\n",
                  command[index], arg.c_str());
    result.SetStatus(lldb::eReturnStatusFailed);
    return false;
  }
  parsed_result = (uint32_t)output;
  return true;
}

bool ParseCommandArgThread(char **command, lldb::SBCommandReturnObject &result,
                           lldb::SBProcess &process, uint32_t &index,
                           lldb::tid_t &thread_id) {
  char *endptr;
  if (!strcmp(command[index], "all"))
    thread_id = LLDB_INVALID_THREAD_ID;
  else {
    uint32_t thread_index_id;
    errno = 0;
    unsigned long output = strtoul(command[index], &endptr, 0);
    if ((errno != 0) || (*endptr != '\0') || (output > UINT32_MAX)) {
      result.Printf("error: invalid thread specification: \"%s\"\n",
                    command[index]);
      result.SetStatus(lldb::eReturnStatusFailed);
      return false;
    }
    thread_index_id = (uint32_t)output;

    lldb::SBThread thread = process.GetThreadByIndexID(thread_index_id);
    if (!thread.IsValid()) {
      result.Printf(
          "error: process has no thread with thread specification: \"%s\"\n",
          command[index]);
      result.SetStatus(lldb::eReturnStatusFailed);
      return false;
    }
    thread_id = thread.GetThreadID();
  }
  return true;
}
