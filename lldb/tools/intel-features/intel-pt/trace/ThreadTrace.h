#pragma once

#include <vector>

#include "FunctionSegment.h"
#include "Instruction.h"
#include "intel-pt.h"

#include "lldb/API/SBError.h"
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
  ThreadTrace();

  ThreadTrace(const ThreadTrace &trace_info);

  ~ThreadTrace();

  Buffer &GetPTBuffer();

  void AllocatePTBuffer(uint64_t size);

  ReadExecuteSectionInfos &GetReadExecuteSectionInfos();

  CPUInfo &GetCPUInfo();

  const InstructionList &GetInstructionLog();

  void SetInstructionLog(InstructionList &instruction_log);

  std::vector<std::shared_ptr<FunctionSegment>> &GetFunctionCallTree();

  void GetInstructionLogAtOffset(uint32_t offset, uint32_t count,
                                 InstructionList &result_list,
                                 lldb::SBError &sberror);

  uint32_t GetStopID() const;

  void SetStopID(uint32_t stop_id);

  lldb::SBTrace &GetUniqueTraceInstance();

  void SetUniqueTraceInstance(lldb::SBTrace &trace);

  size_t GetPosition() const;

  void SetPosition(size_t position, lldb::SBError &sberror);

  std::vector<FunctionSegment *> GetFrames();

  Instruction &GetCurrentInstruction();

  friend class Decoder;

private:
  Buffer m_pt_buffer; // raw trace buffer
  ReadExecuteSectionInfos
      m_readExecuteSectionInfos; // inferior's memory image info
  uint32_t m_thread_stop_id;     // stop id for thread
  lldb::SBTrace m_trace;         // unique tracing instance of a thread/process
  CPUInfo m_pt_cpu; // cpu info of the target on which inferior is running
  InstructionList m_instruction_log; // complete instruction log
  std::vector<std::shared_ptr<FunctionSegment>>
      m_function_call_tree; // complete function call tree
  size_t m_insn_position;   // position of the instruction iterator
};
} // namespace intelpt_private
