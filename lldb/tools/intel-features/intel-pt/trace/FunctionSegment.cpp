#include "FunctionSegment.h"

using namespace intelpt_private;

FunctionSegment::FunctionSegment(size_t id, const InstructionSP &insn_error,
                                 int level)
    : m_id(global_id++), m_insn_first(insn_error), m_insn_last(insn_error),
      m_level(level), m_is_gap(true), m_parent(), m_prev(), m_next() {}

FunctionSegment::FunctionSegment(size_t id, const lldb::SBFunction &function,
                                 const lldb::SBSymbol &symbol,
                                 const InstructionSP &insn, int level,
                                 const FunctionSegmentSP &parent)
    : m_id(id), m_function(function), m_symbol(symbol), m_insn_first(insn),
      m_insn_last(insn), m_level(level), m_is_gap(false), m_parent(parent),
      m_prev(), m_next() {}

bool FunctionSegment::IsGap() const { return m_is_gap; }

int FunctionSegment::GetLevel() const { return m_level; }

void FunctionSegment::SetLevel(int level) { m_level = level; }

const lldb::SBFunction FunctionSegment::GetSBFunction() const {
  return m_function;
}

const lldb::SBSymbol FunctionSegment::GetSymbol() const { return m_symbol; }

const char *FunctionSegment::GetFunctionName() {
  if (m_function.IsValid())
    return m_function.GetName();
  if (m_symbol.IsValid())
    return m_symbol.GetName();
  return "(null)";
}

const char *FunctionSegment::GetDisplayName() {
  if (m_function.IsValid())
    return m_function.GetDisplayName();
  if (m_symbol.IsValid())
    return m_symbol.GetDisplayName();
  return "(null)";
}

lldb::addr_t FunctionSegment::GetStartLoadAddress() const {
  return m_insn_first->GetInsnAddress();
}

lldb::addr_t FunctionSegment::GetEndLoadAddress() const {
  return m_insn_last->GetInsnAddress();
}

FunctionSegmentSP FunctionSegment::GetParent() const { return m_parent; }

void FunctionSegment::SetParent(const FunctionSegmentSP &parent) {
  m_parent = parent;
}

void FunctionSegment::AppendInstruction(const InstructionSP &insn) {
  m_insn_last = insn;
  insn->SetFunctionSegment(shared_from_this());
}

FunctionSegmentSP FunctionSegment::GetNext() const { return m_next.lock(); }

InstructionSP FunctionSegment::GetLastInstruction() const {
  return m_insn_last;
}

InstructionSP FunctionSegment::GetFirstInstruction() const {
  return m_insn_first;
}

FunctionSegmentSP FunctionSegment::GetPrev() const { return m_prev; }

void FunctionSegment::SetNextSegment(const FunctionSegmentSP &next_segment) {
  m_next = next_segment;
  next_segment->m_prev = shared_from_this();

  next_segment->SetLevel(GetLevel());
  next_segment->SetParent(GetParent());
}

int FunctionSegment::GetID() { return m_id; }
