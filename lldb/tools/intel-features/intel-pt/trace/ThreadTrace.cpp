#include "ThreadTrace.h"

#include <cassert>

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

const Instructions &ThreadTrace::GetInstructionLog() {
  return m_instruction_log;
}

void ThreadTrace::SetInstructionLog(Instructions &instruction_log) {
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

size_t ThreadTrace::GetIteratorPosition() { return m_insn_position; }

void ThreadTrace::SetIteratorPosition(size_t position, lldb::SBError &sberror) {
  if (position > m_instruction_log.size()) {
    sberror.SetErrorString("Position is beyond the trace size");
    return;
  }
  m_insn_position = position;
}
