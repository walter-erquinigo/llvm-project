#pragma once

#include <string>
#include <vector>

#include "intel-pt.h"
#include "intelpt_private-forward.h"
#include "lldb/lldb-types.h"

namespace intelpt_private {

/// \class Instruction
/// Represents an assembly instruction containing raw
///     instruction bytes, instruction address along with information
///     regarding execution flow context and Intel(R) Processor Trace
///     context.
class Instruction {
public:
  Instruction() = delete;

  Instruction(const Instruction &insn) = delete;

  Instruction(size_t id, const struct pt_insn &insn);

  Instruction(size_t id, int error_code);

  ~Instruction();

  lldb::addr_t GetInsnAddress() const;

  size_t GetRawBytes(void *buf, size_t size) const;

  pt_insn_class GetInsnClass() const;

  const char *GetError() const;

  bool GetSpeculative() const;

  size_t GetSize() const;

  bool IsError() const;

  FunctionSegmentSP GetFunctionSegment() const;

  void SetFunctionSegment(const FunctionSegmentSP &segment);

  size_t GetID() const;

private:
  size_t m_id;
  std::weak_ptr<FunctionSegment> m_function_segment;
  lldb::addr_t ip;           // instruction address in inferior's memory image
  std::vector<uint8_t> data; // raw bytes
  int m_error_code;         // libipt error code if instruction is invalid
  enum pt_insn_class iclass; // classification of the instruction
  // A collection of flags giving additional information about instruction
  uint32_t speculative : 1; // Instruction was executed speculatively or not
};
} // namespace intelpt_private
