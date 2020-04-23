#include "Frame.h"
#include "Instruction.h"
#include "FunctionSegment.h"
#include "lldb/lldb-defines.h"

using namespace intelpt_private;

Frame::Frame(const FunctionSegmentSP &segment, const InstructionSP &instruction): m_segment(segment), m_insn(instruction) {}

InstructionSP Frame::GetInstruction() const { return m_insn; }

FunctionSegmentSP Frame::GetFunctionSegment() const { return m_segment; }
