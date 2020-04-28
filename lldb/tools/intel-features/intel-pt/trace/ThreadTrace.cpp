#include <cassert>
#include <cinttypes>

#include "ThreadTrace.h"
#include "Frame.h"
#include "lldb/API/SBBreakpoint.h"
#include "lldb/API/SBBreakpointLocation.h"
#include "lldb/API/SBModule.h"
#include "lldb/API/SBSymbolContext.h"
#include "lldb/API/SBLineEntry.h"

using namespace intelpt_private;

ThreadTrace::ThreadTrace(const lldb::SBThread &thread, const lldb::SBTarget &target)
    : m_thread(thread), m_target(target), m_pt_buffer(), m_readExecuteSectionInfos(),
      m_thread_stop_id(0), m_trace(), m_pt_cpu(), m_instruction_log(),
      m_insn_position(0) {}

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
  m_instruction_log.assign(instruction_log.begin(), instruction_log.end());
  assert(!m_instruction_log.empty());
  m_insn_position = m_instruction_log.size() - 1;
}

std::vector<FunctionSegmentSP> &ThreadTrace::GetFunctionCallTree() {
  return m_function_call_tree;
}

uint32_t ThreadTrace::GetStopID() const { return m_thread_stop_id; }

void ThreadTrace::SetStopID(uint32_t stop_id) { m_thread_stop_id = stop_id; }

lldb::SBTrace &ThreadTrace::GetUniqueTraceInstance() { return m_trace; }

void ThreadTrace::SetUniqueTraceInstance(lldb::SBTrace &trace) {
  m_trace = trace;
}

size_t ThreadTrace::GetPosition() const { return m_insn_position; }

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

InstructionSP ThreadTrace::GetCurrentInstruction() {
  return m_instruction_log[GetPosition()];
}

void ThreadTrace::GetFrames(std::vector<FrameSP> &frames) {
  InstructionSP current_insn = GetCurrentInstruction();
  FunctionSegmentSP segment = current_insn->GetFunctionSegment();
  FunctionSegmentSP inner_segment;

  do {
    if (!inner_segment)
      frames.push_back(std::make_shared<Frame>(segment, current_insn));
    else if (inner_segment->GetID() > segment->GetID())
      // We saw when inner_segment was called
      frames.push_back(
          std::make_shared<Frame>(segment, segment->GetLastInstruction()));
    else
      // We only know that the inner_segment returns to this segment
      frames.push_back(std::make_shared<Frame>(segment, InstructionSP()));

    inner_segment = segment;
    segment = inner_segment->GetParent();
  } while (segment);
}

void ThreadTrace::GetBreakpointAddresses(
  std::unordered_set<lldb::addr_t> &bp_addresses
) {
  for (size_t i = 0; i < m_target.GetNumBreakpoints(); i++) {
    lldb::SBBreakpoint breakpoint = m_target.GetBreakpointAtIndex(i);
    for (size_t j = 0; j < breakpoint.GetNumLocations(); j++) {
      lldb::SBBreakpointLocation location = breakpoint.GetLocationAtIndex(j);
      bp_addresses.insert(location.GetLoadAddress());
    }
  }
}

enum Direction {
  eDirectionForward = 0,
  eDirectionReverse,
};

// Reverse with direction
bool ThreadTrace::DoStepInst(bool step_over, Direction direction) {
  int delta = direction == eDirectionForward ? 1 : -1;

  if (m_insn_position + delta < 0 || m_insn_position + delta >= (int)m_instruction_log.size())
    return false;

  if (!step_over) {
    m_insn_position += delta;
    return true;
  }

  FunctionSegmentSP start_segment = m_instruction_log[m_insn_position]->GetFunctionSegment();
  std::unordered_set<lldb::addr_t> bp_addresses;
  GetBreakpointAddresses(bp_addresses);

  while (m_insn_position + delta >= 0 && m_insn_position + delta < (int)m_instruction_log.size()) {
    m_insn_position += delta;

    if (bp_addresses.count(m_instruction_log[m_insn_position]->GetInsnAddress()))
      break; // we stopped at a breakpoint

    FunctionSegmentSP cur_segment = m_instruction_log[m_insn_position]->GetFunctionSegment();
    if (cur_segment->GetLevel() > start_segment->GetLevel())
      continue; // we've stepped in

    break;
  }
  return true;
}

bool ThreadTrace::ReverseStepInst(bool step_over) {
  return DoStepInst(step_over, eDirectionReverse);
}

bool ThreadTrace::StepInst(bool step_over) {
  return DoStepInst(step_over, eDirectionForward);
}

bool ThreadTrace::DoContinue(Direction direction) {
  int delta = direction == eDirectionForward ? 1 : -1;

  if (m_insn_position + delta < 0 || m_insn_position + delta >= (int)m_instruction_log.size())
    return false;

  std::unordered_set<lldb::addr_t> bp_addresses;
  GetBreakpointAddresses(bp_addresses);


  while (m_insn_position + delta >= 0 && m_insn_position + delta < (int)m_instruction_log.size()) {
    m_insn_position += delta;

    lldb::addr_t cur_address = m_instruction_log[m_insn_position]->GetInsnAddress();

    if (bp_addresses.count(cur_address))
      break; // we stopped at a breakpoint
  }
  return true;
}

bool ThreadTrace::Continue() {
  return DoContinue(eDirectionForward);
}

bool ThreadTrace::ReverseContinue() {
  return DoContinue(eDirectionReverse);
}

bool ThreadTrace::DoStepOver(Direction direction) {
  int delta = direction == eDirectionForward ? 1 : -1;

  if (m_insn_position + delta < 0 || m_insn_position + delta >= (int)m_instruction_log.size())
    return false;

  lldb::addr_t load_addresss = m_instruction_log[m_insn_position]->GetInsnAddress();
  lldb::SBAddress address(load_addresss, m_target);
  lldb::SBModule module = address.GetModule();
  lldb::SBSymbolContext sc = module.ResolveSymbolContextForAddress(address, lldb::eSymbolContextLineEntry);
  lldb::SBLineEntry line_entry = sc.GetLineEntry();

  // We don't have line debug info
  if (!line_entry.IsValid())
    return DoStepInst(true, direction);


  lldb::addr_t start_address = line_entry.GetStartAddress().GetLoadAddress(m_target);
  lldb::addr_t end_address = line_entry.GetEndAddress().GetLoadAddress(m_target);

  FunctionSegmentSP start_segment = m_instruction_log[m_insn_position]->GetFunctionSegment();
  std::unordered_set<lldb::addr_t> bp_addresses;
  GetBreakpointAddresses(bp_addresses);

  while (m_insn_position + delta >= 0 && m_insn_position + delta < (int)m_instruction_log.size()) {
    m_insn_position += delta;

    lldb::addr_t cur_address = m_instruction_log[m_insn_position]->GetInsnAddress();

    if (bp_addresses.count(cur_address))
      break;; // we stopped at a breakpoint

    FunctionSegmentSP cur_segment = m_instruction_log[m_insn_position]->GetFunctionSegment();

    if (cur_segment->GetLevel() > start_segment->GetLevel())
      continue; // we've stepped in

    if (cur_segment->GetLevel() < start_segment->GetLevel())
      break; // we've stepped out, so we stop

    if (cur_address < start_address || cur_address > end_address)
      break; // we are in a different line range, so we stop
  }
  return true;
}

bool ThreadTrace::ReverseStepOver() {
  return DoStepOver(eDirectionReverse);
}

bool ThreadTrace::StepOver() {
  return DoStepOver(eDirectionForward);
}
