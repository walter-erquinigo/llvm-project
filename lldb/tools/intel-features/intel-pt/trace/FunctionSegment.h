#pragma once

#include "Instruction.h"

#include "lldb/API/SBProcess.h"

namespace intelpt_private {

static int global_id = 0;
class FunctionSegment {
public:
  FunctionSegment() = delete;

  FunctionSegment(Instruction *insn_error, int level);

  FunctionSegment(const lldb::SBFunction &function,
                  const lldb::SBSymbol &symbol, Instruction *insn, int level,
                  FunctionSegment *parent);

  bool IsGap() const;

  int GetLevel() const;

  void SetLevel(int level);

  const lldb::SBFunction &GetSBFunction() const;

  const lldb::SBSymbol &GetSymbol() const;

  const char *GetFunctionName();

  lldb::addr_t GetStartLoadAddress() const;

  const char *GetDisplayName();

  FunctionSegment *GetParent() const;

  void SetParent(FunctionSegment *parent);

  void AppendInstruction(Instruction *insn);

  FunctionSegment *GetNext() const;

  Instruction *GetLastInstruction() const;

  FunctionSegment *GetPrev() const;

  void SetNextSegment(FunctionSegment *next_segment);

  int GetID();

private:
  friend class FunctionCallTreeBuilder;

  int m_id;
  lldb::SBFunction m_function;
  lldb::SBSymbol m_symbol;
  Instruction *m_insn_first;
  Instruction *m_insn_last;
  int m_level;
  bool m_is_gap;
  /* The function segment number of the directly preceding function segment in
     a (fake) call stack. */
  FunctionSegment *m_parent;
  /* The function segment numbers of the previous and next segment belonging to
     the same function.  If a function calls another function, the former will
     have at least two segments: one before the call and another after the
     return.  */
  FunctionSegment *m_prev;
  FunctionSegment *m_next;
};

} // namespace intelpt_private
