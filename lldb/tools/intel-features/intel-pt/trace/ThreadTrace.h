#pragma once

#include <vector>
#include <unordered_set>

#include "FunctionSegment.h"
#include "Instruction.h"
#include "intel-pt.h"

#include "lldb/API/SBError.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBThread.h"
#include "lldb/API/SBTrace.h"

namespace intelpt_private {
typedef struct pt_cpu CPUInfo;
typedef std::vector<uint8_t> Buffer;

// class to manage inferior's read-execute section information
class ReadExecuteSectionInfo {
public:
  uint64_t load_address;
  uint64_t file_offset;
  uint64_t size;
  std::string image_path;

  ReadExecuteSectionInfo(const uint64_t addr, const uint64_t offset,
                         const uint64_t sz, const std::string &path)
      : load_address(addr), file_offset(offset), size(sz), image_path(path) {}

  ReadExecuteSectionInfo(const ReadExecuteSectionInfo &rxsection) = default;
};

typedef std::vector<ReadExecuteSectionInfo> ReadExecuteSectionInfos;

class ThreadTrace {
public:
  ThreadTrace(const lldb::SBThread &thread, const lldb::SBTarget &target);

  ThreadTrace(const ThreadTrace &trace_info) = delete;

  ~ThreadTrace();

  Buffer &GetPTBuffer();

  void AllocatePTBuffer(uint64_t size);

  ReadExecuteSectionInfos &GetReadExecuteSectionInfos();

  CPUInfo &GetCPUInfo();

  const InstructionList &GetInstructionLog();

  void SetInstructionLog(InstructionList &instruction_log);

  std::vector<FunctionSegmentSP> &GetFunctionCallTree();

  void GetInstructionLogAtOffset(uint32_t offset, uint32_t count,
                                 InstructionList &result_list,
                                 lldb::SBError &sberror);

  uint32_t GetStopID() const;

  void SetStopID(uint32_t stop_id);

  lldb::SBTrace &GetUniqueTraceInstance();

  void SetUniqueTraceInstance(lldb::SBTrace &trace);

  int GetPosition() const;

  void SetPosition(int position, lldb::SBError &sberror);

  void GetFrames(std::vector<FrameSP> &frames);

  InstructionSP GetCurrentInstruction();

  bool ReverseStepInst(bool step_over = false);

  bool StepInst(bool sttep_over = false);

  bool ReverseStepOver();

  bool StepOver();

  bool Continue();

  bool ReverseContinue();

  bool StepIn();

  bool ReverseStepIn();

  bool StepOut();

  bool ReverseStepOut();

private:

 enum Direction {
  eDirectionForward = 0,
  eDirectionReverse,
 };

 bool DoStepInst(bool step_over, Direction dir);

 bool DoSourceLevelStepping(bool step_over, Direction dir);

 bool DoStepOut(Direction dir);

 bool DoContinue(Direction dir);

 void GetBreakpointAddresses(std::unordered_set<lldb::addr_t> &bp_addresses);

  lldb::SBThread m_thread; // thread associated with this trace
  lldb::SBTarget m_target; // target associated with this trace
  Buffer m_pt_buffer; // raw trace buffer
  ReadExecuteSectionInfos
      m_readExecuteSectionInfos; // inferior's memory image info
  uint32_t m_thread_stop_id;     // stop id for thread
  lldb::SBTrace m_trace;         // unique tracing instance of a thread/process
  CPUInfo m_pt_cpu; // cpu info of the target on which inferior is running
  InstructionList m_instruction_log; // complete instruction log
  std::vector<std::shared_ptr<FunctionSegment>>
      m_function_call_tree; // complete function call tree
  int m_insn_position;   // position of the instruction iterator
};
} // namespace intelpt_private
