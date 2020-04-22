#include "ThreadTrace.h"

#include <cassert>
#include <cinttypes>

using namespace intelpt_private;

ThreadTrace::ThreadTrace()
    : m_pt_buffer(), m_readExecuteSectionInfos(), m_thread_stop_id(0),
      m_trace(), m_pt_cpu(), m_instruction_log(), m_insn_position(0) {}

ThreadTrace::ThreadTrace(const ThreadTrace &trace_info) = default;

ThreadTrace::~ThreadTrace() {}

Buffer &ThreadTrace::GetPTBuffer() { return m_pt_buffer; }

void ThreadTrace::AllocatePTBuffer(uint64_t size) {
  m_pt_buffer.assign(size, 0);
}

ReadExecuteSectionInfos &ThreadTrace::GetReadExecuteSectionInfos() {
  return m_readExecuteSectionInfos;
}

CPUInfo &ThreadTrace::GetCPUInfo() { return m_pt_cpu; }

const InstructionList &ThreadTrace::GetInstructionLog() {
  return m_instruction_log;
}

void ThreadTrace::SetInstructionLog(InstructionList &instruction_log) {
  m_instruction_log = std::move(instruction_log);
  assert(!m_instruction_log.empty());
  m_insn_position = m_instruction_log.size() - 1;
}

std::vector<std::shared_ptr<FunctionSegment>> &
ThreadTrace::GetFunctionCallTree() {
  return m_function_call_tree;
}

uint32_t ThreadTrace::GetStopID() const { return m_thread_stop_id; }

void ThreadTrace::SetStopID(uint32_t stop_id) { m_thread_stop_id = stop_id; }

lldb::SBTrace &ThreadTrace::GetUniqueTraceInstance() { return m_trace; }

void ThreadTrace::SetUniqueTraceInstance(lldb::SBTrace &trace) {
  m_trace = trace;
}

size_t ThreadTrace::GetPosition() { return m_insn_position; }

void ThreadTrace::SetPosition(size_t position, lldb::SBError &sberror) {
  if (position > m_instruction_log.size()) {
    sberror.SetErrorString("Position is beyond the trace size");
    return;
  }
  m_insn_position = position;
}

void ThreadTrace::GetInstructionLogAtOffset(uint32_t offset, uint32_t count,
                                            InstructionList &result_list,
                                            lldb::SBError &sberror) {
  // Return instruction log by populating 'result_list'
  const InstructionList &insn_list = GetInstructionLog();
  uint64_t sum = (uint64_t)offset + 1;
  if (((insn_list.size() <= offset) && (count <= sum) &&
       ((sum - count) >= insn_list.size())) ||
      (count < 1)) {
    sberror.SetErrorStringWithFormat(
        "Instruction Log not available for offset=%" PRIu32
        " and count=%" PRIu32,
        offset, count);
    return;
  }

  InstructionList::const_iterator itr_first =
      (insn_list.size() <= offset) ? insn_list.begin()
                                   : insn_list.begin() + insn_list.size() - sum;
  InstructionList::const_iterator itr_last =
      (count <= sum) ? insn_list.begin() + insn_list.size() - (sum - count)
                     : insn_list.end();
  InstructionList::const_iterator itr = itr_first;
  while (itr != itr_last) {
    result_list.push_back(*itr);
    ++itr;
  }
}
