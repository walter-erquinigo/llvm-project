#pragma once

#include <memory>
#include <vector>

#include "FunctionSegment.h"
#include "Instruction.h"

#include "lldb/API/SBProcess.h"

namespace intelpt_private {

class FunctionCallTreeBuilder {
public:
  FunctionCallTreeBuilder() = delete;

  FunctionCallTreeBuilder(const FunctionCallTreeBuilder &builder) = delete;

  FunctionCallTreeBuilder(lldb::SBProcess &process);

  void AppendInstruction(const InstructionSP &insn);

  void Finalize(std::vector<FunctionSegmentSP> &segments);

  void AppendNewSwitchFunctionSegment(const lldb::SBFunction &sbfunction,
                                      const lldb::SBSymbol &symbol,
                                      const InstructionSP &insn);

  void AppendNewCallFunctionSegment(const lldb::SBFunction &sbfunction,
                                    const lldb::SBSymbol &symbol,
                                    const InstructionSP &insn);

  void AppendNewTailCallFunctionSegment(const lldb::SBFunction &sbfunction,
                                        const lldb::SBSymbol &symbol,
                                        const InstructionSP &insn);

  /* Add a continuation segment for a function into which we return at the end
     of the trace. */
  void AppendNewReturnFunctionSegment(lldb::SBFunction &sbfunction,
                                      lldb::SBSymbol &symbol,
                                      const InstructionSP &insn);

  void UpdateFunctionSegmentsWithErrorInstruction(const InstructionSP &insn);

private:
  std::vector<FunctionSegmentSP> m_segments;
  lldb::SBProcess m_process;
};

} // namespace intelpt_private
