//===-- PTManager.cpp -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "PTManager.h"
#include "Decoder.h"

using namespace intelpt;
using namespace intelpt_private;

// PTInstruction class member functions definitions
PTInstruction::PTInstruction(
    const std::shared_ptr<intelpt_private::Instruction> &ptr)
    : m_opaque_sp(ptr) {}

PTInstruction::~PTInstruction() {}

uint64_t PTInstruction::GetInsnAddress() const {
  return (m_opaque_sp ? m_opaque_sp->GetInsnAddress() : 0);
}

size_t PTInstruction::GetRawBytes(void *buf, size_t size) const {
  return (m_opaque_sp ? m_opaque_sp->GetRawBytes(buf, size) : 0);
}

const char *PTInstruction::GetError() const {
  return (m_opaque_sp ? m_opaque_sp->GetError() : "null pointer");
}

bool PTInstruction::IsError() const  {
  return m_opaque_sp ? m_opaque_sp->IsError() : true;
}

bool PTInstruction::GetSpeculative() const {
  return (m_opaque_sp ? m_opaque_sp->GetSpeculative() : 0);
}

// SEGMENT
const char *PTFunctionSegment::GetFunctionName() const {
  return (m_opaque_sp ? m_opaque_sp->GetFunctionName() : nullptr);
}

const char *PTFunctionSegment::GetDisplayName() const {
  return (m_opaque_sp ? m_opaque_sp->GetDisplayName() : nullptr);
}

lldb::addr_t PTFunctionSegment::GetStartLoadAddress() const {
  return (m_opaque_sp ? m_opaque_sp->GetStartLoadAddress()
                      : LLDB_INVALID_ADDRESS);
}

int PTFunctionSegment::GetLevel() const {
  return (m_opaque_sp ? m_opaque_sp->GetLevel() : 0);
}

PTFunctionSegment::PTFunctionSegment(std::shared_ptr<FunctionSegment> segment)
    : m_opaque_sp(segment) {}

PTFunctionSegment::PTFunctionSegment() {}

// CALL TREE
PTFunctionSegment
PTFunctionCallTree::GetFunctionSegmentAtIndex(size_t index) const {
  return m_opaque_sp ? PTFunctionSegment(m_opaque_sp->at(index))
                     : PTFunctionSegment();
}

size_t PTFunctionCallTree::GetSize() const {
  return m_opaque_sp ? m_opaque_sp->size() : 0;
}

void PTFunctionCallTree::SetSP(
    std::shared_ptr<
        std::vector<std::shared_ptr<intelpt_private::FunctionSegment>>>
        call_tree) {
  m_opaque_sp = call_tree;
}

// PTInstructionList class member functions definitions
size_t PTInstructionList::GetSize() const {
  return (m_opaque_sp ? m_opaque_sp->GetSize() : 0);
}

PTInstruction PTInstructionList::GetInstructionAtIndex(uint32_t idx) {
  if (m_opaque_sp)
    return PTInstruction(std::shared_ptr<intelpt_private::Instruction>(
        new Instruction(m_opaque_sp->GetInstructionAtIndex(idx))));

  return PTInstruction(std::shared_ptr<intelpt_private::Instruction>(
      new Instruction()));
}

void PTInstructionList::SetSP(
    const std::shared_ptr<intelpt_private::InstructionList> &ptr) {
  m_opaque_sp = ptr;
}
void PTInstructionList::Clear() {
  if (!m_opaque_sp)
    return;
  m_opaque_sp.reset();
}

// PTTraceOptions class member functions definitions
lldb::TraceType PTTraceOptions::GetType() const {
  return (m_opaque_sp ? m_opaque_sp->getType()
                      : lldb::TraceType::eTraceTypeNone);
}

uint64_t PTTraceOptions::GetTraceBufferSize() const {
  return (m_opaque_sp ? m_opaque_sp->getTraceBufferSize() : 0);
}

uint64_t PTTraceOptions::GetMetaDataBufferSize() const {
  return (m_opaque_sp ? m_opaque_sp->getMetaDataBufferSize() : 0);
}

lldb::SBStructuredData PTTraceOptions::GetTraceParams(lldb::SBError &error) {
  if (!m_opaque_sp)
    error.SetErrorString("null pointer");
  return (m_opaque_sp ? m_opaque_sp->getTraceParams(error)
                      : lldb::SBStructuredData());
}

void PTTraceOptions::SetSP(
    const std::shared_ptr<intelpt_private::TraceOptions> &ptr) {
  m_opaque_sp = ptr;
}

// PTManager class member functions definitions
PTManager::PTManager(lldb::SBDebugger &sbdebugger)
    : m_opaque_sp(new intelpt_private::Decoder(sbdebugger)) {}

void PTManager::StartProcessorTrace(lldb::SBProcess &sbprocess,
                                    lldb::SBTraceOptions &sbtraceoptions,
                                    lldb::SBError &sberror) {
  if (m_opaque_sp == nullptr) {
    sberror.SetErrorStringWithFormat("invalid PTManager instance");
    return;
  }

  m_opaque_sp->StartProcessorTrace(sbprocess, sbtraceoptions, sberror);
}

void PTManager::StopProcessorTrace(lldb::SBProcess &sbprocess,
                                   lldb::SBError &sberror, lldb::tid_t tid) {
  if (m_opaque_sp == nullptr) {
    sberror.SetErrorStringWithFormat("invalid PTManager instance");
    return;
  }

  m_opaque_sp->StopProcessorTrace(sbprocess, sberror, tid);
}

void PTManager::GetInstructionLogAtOffset(lldb::SBProcess &sbprocess,
                                          lldb::tid_t tid, uint32_t offset,
                                          uint32_t count,
                                          PTInstructionList &result_list,
                                          lldb::SBError &sberror) {
  if (m_opaque_sp == nullptr) {
    sberror.SetErrorStringWithFormat("invalid PTManager instance");
    return;
  }

  std::shared_ptr<intelpt_private::InstructionList> insn_list_ptr(
      new InstructionList());
  m_opaque_sp->GetInstructionLogAtOffset(sbprocess, tid, offset, count,
                                         *insn_list_ptr, sberror);
  if (!sberror.Success())
    return;

  result_list.SetSP(insn_list_ptr);
}

void PTManager::GetFunctionCallTree(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                                    PTFunctionCallTree &call_tree,
                                    lldb::SBError &sberror) {
  if (m_opaque_sp == nullptr) {
    sberror.SetErrorStringWithFormat("invalid PTManager instance");
    return;
  }

  std::shared_ptr<
      std::vector<std::shared_ptr<intelpt_private::FunctionSegment>>>
      call_tree_ptr(
          new std::vector<std::shared_ptr<intelpt_private::FunctionSegment>>());
  m_opaque_sp->GetFunctionCallTree(sbprocess, tid, *call_tree_ptr, sberror);
  if (!sberror.Success())
    return;
  call_tree.SetSP(call_tree_ptr);
}

void PTManager::GetProcessorTraceInfo(lldb::SBProcess &sbprocess,
                                      lldb::tid_t tid, PTTraceOptions &options,
                                      lldb::SBError &sberror) {
  if (m_opaque_sp == nullptr) {
    sberror.SetErrorStringWithFormat("invalid PTManager instance");
    return;
  }

  std::shared_ptr<intelpt_private::TraceOptions> trace_options_ptr(
      new TraceOptions());
  m_opaque_sp->GetProcessorTraceInfo(sbprocess, tid, *trace_options_ptr,
                                     sberror);
  if (!sberror.Success())
    return;

  options.SetSP(trace_options_ptr);
}
