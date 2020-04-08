#include "FunctionSegment.h"

using namespace intelpt_private;

static int global_id = 0;
FunctionSegment::FunctionSegment(Instruction *insn_error, int level)
    : m_id(global_id++), m_insn_first(insn_error), m_insn_last(insn_error),
      m_level(level), m_is_gap(true), m_parent(nullptr), m_prev(nullptr),
      m_next(nullptr) {}

FunctionSegment::FunctionSegment(const lldb::SBFunction &function,
                                 const lldb::SBSymbol &symbol,
                                 Instruction *insn, int level,
                                 FunctionSegment *parent)
    : m_id(global_id++), m_function(function), m_symbol(symbol),
      m_insn_first(insn), m_insn_last(insn), m_level(level), m_is_gap(false),
      m_parent(parent), m_prev(nullptr), m_next(nullptr) {}

bool FunctionSegment::IsGap() const { return m_is_gap; }

int FunctionSegment::GetLevel() const { return m_level; }

void FunctionSegment::SetLevel(int level) { m_level = level; }

const lldb::SBFunction &FunctionSegment::GetSBFunction() const {
  return m_function;
}

const lldb::SBSymbol &FunctionSegment::GetSymbol() const { return m_symbol; }

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

FunctionSegment *FunctionSegment::GetParent() const { return m_parent; }

void FunctionSegment::SetParent(FunctionSegment *parent) { m_parent = parent; }

void FunctionSegment::AppendInstruction(Instruction *insn) {
  m_insn_last = insn;
}

FunctionSegment *FunctionSegment::GetNext() const { return m_next; }

Instruction *FunctionSegment::GetLastInstruction() const { return m_insn_last; }

FunctionSegment *FunctionSegment::GetPrev() const { return m_prev; }

void FunctionSegment::SetNextSegment(FunctionSegment *next_segment) {
  m_next = next_segment;
  next_segment->m_prev = this;

  next_segment->SetLevel(GetLevel());
  next_segment->SetParent(GetParent());
}

int FunctionSegment::GetID() { return m_id; }
