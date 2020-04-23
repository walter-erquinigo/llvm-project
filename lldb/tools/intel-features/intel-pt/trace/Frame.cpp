#include "Frame.h"
#include "Instruction.h"
#include "FunctionSegment.h"
#include "lldb/lldb-defines.h"

using namespace intelpt_private;

Frame::Frame(const FunctionSegmentSP &segment, const InstructionSP &instruction): m_segment(segment), m_insn(instruction) {}

lldb::addr_t Frame::GetLoadAddress() const {
  return m_insn ? m_insn->GetInsnAddress() : LLDB_INVALID_ADDRESS;
}

const char* Frame::GetDisplayName() const {
  return m_segment->GetDisplayName();
}
