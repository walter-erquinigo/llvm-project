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

  FunctionCallTreeBuilder(lldb::SBProcess &process);

  void AppendInstruction(Instruction &insn);

  void Finalize(std::vector<std::shared_ptr<FunctionSegment>> &segments);

  void AppendNewSwitchFunctionSegment(const lldb::SBFunction &sbfunction,
                                      const lldb::SBSymbol &symbol,
                                      Instruction &insn);

  void AppendNewCallFunctionSegment(const lldb::SBFunction &sbfunction,
                                    const lldb::SBSymbol &symbol,
                                    Instruction &insn);

  void AppendNewTailCallFunctionSegment(const lldb::SBFunction &sbfunction,
                                        const lldb::SBSymbol &symbol,
                                        Instruction &insn);

  /* Add a continuation segment for a function into which we return at the end
     of the trace. */
  void AppendNewReturnFunctionSegment(lldb::SBFunction &sbfunction,
                                      lldb::SBSymbol &symbol,
                                      Instruction &insn);

  void UpdateFunctionSegmentsWithErrorInstruction(Instruction &insn);

  void AppendPC(lldb::tid_t tid);

private:
  std::vector<std::shared_ptr<FunctionSegment>> m_segments;
  lldb::SBProcess m_process;
};

} // namespace intelpt_private
