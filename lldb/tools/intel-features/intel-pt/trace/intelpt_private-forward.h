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
typedef std::shared_ptr<FunctionSegment> FunctionSegmentSP;
typedef std::shared_ptr<Frame> FrameSP;

typedef std::vector<InstructionSP> InstructionList;
typedef std::vector<FrameSP> FrameList;
} // namespace intelpt_private
