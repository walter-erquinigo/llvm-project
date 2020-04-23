#pragma once

#include <memory>
#include <vector>

namespace intelpt_private {
class Decoder;
class Frame;
class FunctionSegment;
class Instruction;
class TraceOptions;
class ThreadTrace;

typedef std::shared_ptr<Instruction> InstructionSP;
typedef std::vector<InstructionSP> InstructionList;
typedef std::shared_ptr<FunctionSegment> FunctionSegmentSP;
} // namespace intelpt_private
