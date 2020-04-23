#pragma once

#include "intelpt_private-forward.h"
#include "lldb/lldb-types.h"

namespace intelpt_private {

class Frame {
 public:
  Frame(const FunctionSegmentSP &segment, const InstructionSP &instruction);

  InstructionSP GetInstruction() const;

  FunctionSegmentSP GetFunctionSegment() const;

 private:
  FunctionSegmentSP m_segment;
  InstructionSP m_insn;
};

} // intelpt_private
