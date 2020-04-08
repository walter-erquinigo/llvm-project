#pragma once

#include <string>
#include <vector>

#include "intel-pt.h"
#include "lldb/lldb-types.h"

namespace intelpt_private {
/// \class Instruction
/// Represents an assembly instruction containing raw
///     instruction bytes, instruction address along with information
///     regarding execution flow context and Intel(R) Processor Trace
///     context.
class Instruction {
public:
  Instruction();

  Instruction(const Instruction &insn) = default;

  Instruction(const struct pt_insn &insn);

  Instruction(const char *err);

  ~Instruction();

  lldb::addr_t GetInsnAddress() const;

  size_t GetRawBytes(void *buf, size_t size) const;

  pt_insn_class GetInsnClass() const;

  const std::string &GetError() const;

  bool GetSpeculative() const;

  size_t GetSize() const;

private:
  lldb::addr_t ip;           // instruction address in inferior's memory image
  std::vector<uint8_t> data; // raw bytes
  std::string error;         // Error string if instruction is invalid
  enum pt_insn_class iclass; // classification of the instruction
  // A collection of flags giving additional information about instruction
  uint32_t speculative : 1; // Instruction was executed speculatively or not
};

/// \class InstructionList
/// Represents a list of assembly instructions. Each instruction is of
///     type Instruction.
class InstructionList {
public:
  InstructionList();

  InstructionList(const InstructionList &insn_list);

  ~InstructionList();

  // Get number of instructions in the list
  size_t GetSize() const;

  // Get instruction at index
  Instruction GetInstructionAtIndex(uint32_t idx);

  // Append intruction at the end of the list
  void AppendInstruction(Instruction inst);

private:
  std::vector<Instruction> m_insn_vec;
};

} // namespace intelpt_private
