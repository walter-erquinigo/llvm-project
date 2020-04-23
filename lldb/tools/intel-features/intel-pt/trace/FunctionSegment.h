#pragma once

#include "Instruction.h"

#include "lldb/API/SBProcess.h"

namespace intelpt_private {

static int global_id = 0;
class FunctionSegment : public std::enable_shared_from_this<FunctionSegment> {
public:
  FunctionSegment(const FunctionSegment &segment) = delete;

  FunctionSegment(size_t id, const InstructionSP &insn_error, int level);

  FunctionSegment(size_t id, const lldb::SBFunction &function,
                  const lldb::SBSymbol &symbol, const InstructionSP &insn,
                  int level, const FunctionSegmentSP &parent);

  bool IsGap() const;

  int GetLevel() const;

  void SetLevel(int level);

  const lldb::SBFunction GetSBFunction() const;

  const lldb::SBSymbol GetSymbol() const;

  const char *GetFunctionName();

  lldb::addr_t GetStartLoadAddress() const;

  lldb::addr_t GetEndLoadAddress() const;

  const char *GetDisplayName();

  FunctionSegmentSP GetParent() const;

  void SetParent(const FunctionSegmentSP &parent);

  void AppendInstruction(const InstructionSP &insn);

  FunctionSegmentSP GetNext() const;

  InstructionSP GetLastInstruction() const;

  InstructionSP GetFirstInstruction() const;

  FunctionSegmentSP GetPrev() const;

  void SetNextSegment(const FunctionSegmentSP &next_segment);

  int GetID();

private:
  friend class FunctionCallTreeBuilder;

  int m_id;
  lldb::SBFunction m_function;
  lldb::SBSymbol m_symbol;
  InstructionSP m_insn_first;
  InstructionSP m_insn_last;
  int m_level;
  bool m_is_gap;
  /* The function segment number of the directly preceding function segment in
     a (fake) call stack. */
  FunctionSegmentSP m_parent;
  /* The function segment numbers of the previous and next segment belonging to
     the same function.  If a function calls another function, the former will
     have at least two segments: one before the call and another after the
     return.  */
  FunctionSegmentSP m_prev;
  std::weak_ptr<FunctionSegment> m_next;
};

} // namespace intelpt_private
