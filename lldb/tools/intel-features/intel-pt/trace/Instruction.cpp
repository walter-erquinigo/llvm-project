#include "Instruction.h"

#include "intel-pt.h"
#include "lldb/lldb-defines.h"

using namespace intelpt_private;

Instruction::Instruction(size_t id, const struct pt_insn &insn)
    : m_id(id), m_function_segment(), ip(insn.ip), data(), m_error_code(0),
      iclass(insn.iclass), speculative(insn.speculative) {
  if (insn.size != 0)
    data.assign(insn.raw, insn.raw + insn.size);
}

Instruction::Instruction(size_t id, int error_code)
    : m_id(id), m_function_segment(), ip(LLDB_INVALID_ADDRESS), data(), m_error_code(error_code),
      iclass(ptic_error), speculative(0) {}

Instruction::~Instruction() {}

lldb::addr_t Instruction::GetInsnAddress() const { return ip; }

size_t Instruction::GetRawBytes(void *buf, size_t size) const {
  if ((buf == nullptr) || (size == 0))
    return data.size();

  size_t bytes_to_read = ((size <= data.size()) ? size : data.size());
  ::memcpy(buf, data.data(), bytes_to_read);
  return bytes_to_read;
}

pt_insn_class Instruction::GetInsnClass() const { return iclass; }

const char *Instruction::GetError() const { return pt_errstr(pt_errcode(m_error_code)); }

bool Instruction::IsError() const {
  return m_error_code != 0;
}

bool Instruction::GetSpeculative() const { return speculative; }

size_t Instruction::GetSize() const { return data.size(); }

FunctionSegmentSP Instruction::GetFunctionSegment() const {
  return m_function_segment.lock();
}

void Instruction::SetFunctionSegment(const FunctionSegmentSP &segment) {
  m_function_segment = segment;
}

size_t Instruction::GetID() const { return m_id; }
