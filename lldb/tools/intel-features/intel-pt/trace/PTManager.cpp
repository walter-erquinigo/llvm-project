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

PTInstruction::PTInstruction() : m_opaque_sp() {}

// PTInstruction class member functions definitions
PTInstruction::PTInstruction(intelpt_private::InstructionSP insn)
    : m_opaque_sp(insn) {}

PTInstruction::~PTInstruction() {}

uint64_t PTInstruction::GetInsnAddress() const {
  return (m_opaque_sp ? m_opaque_sp->GetInsnAddress() : LLDB_INVALID_ADDRESS);
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

int PTInstruction::GetID() const {
  return m_opaque_sp ? m_opaque_sp->GetID() : -1;
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

lldb::addr_t PTFunctionSegment::GetEndLoadAddress() const {
  return (m_opaque_sp ? m_opaque_sp->GetEndLoadAddress()
                      : LLDB_INVALID_ADDRESS);
}

size_t PTFunctionSegment::GetID() const {
  return (m_opaque_sp ? m_opaque_sp->GetID() : LLDB_INVALID_ADDRESS);
}

int PTFunctionSegment::GetLevel() const {
  return (m_opaque_sp ? m_opaque_sp->GetLevel() : 0);
}

PTFunctionSegment::PTFunctionSegment(FunctionSegmentSP segment)
    : m_opaque_sp(segment) {}

PTFunctionSegment::PTFunctionSegment() {}

PTInstruction PTFunctionSegment::GetFirstInstruction() const {
  return m_opaque_sp ? PTInstruction(m_opaque_sp->GetFirstInstruction())
                     : PTInstruction();
}

// PTFrame

PTFrame::PTFrame() : m_opaque_sp() {}

PTFrame::PTFrame(intelpt_private::FrameSP sp) : m_opaque_sp(sp) {}

PTInstruction PTFrame::GetInstruction() const {
  return m_opaque_sp ? PTInstruction(m_opaque_sp->GetInstruction())
                     : PTInstruction();
}

PTFunctionSegment PTFrame::GetFunctionSegment() const {
  return m_opaque_sp ? PTFunctionSegment(m_opaque_sp->GetFunctionSegment())
                     : PTFunctionSegment();
}

// PTFrameList

PTFrameList::PTFrameList() : m_opaque_sp() {}

PTFrameList::PTFrameList(std::shared_ptr<intelpt_private::FrameList> sp)
    : m_opaque_sp(sp) {}

size_t PTFrameList::GetNumFrames() const {
  return m_opaque_sp ? m_opaque_sp->size() : 0;
}

PTFrame PTFrameList::GetFrameAtIndex(size_t index) const {
  return m_opaque_sp ? PTFrame(m_opaque_sp->at(index)) : PTFrame();
}

// CALL TREE

PTFunctionSegment
PTFunctionCallTree::GetFunctionSegmentAtIndex(size_t index) const {
  return m_opaque_ptr ? PTFunctionSegment(m_opaque_ptr->at(index))
                      : PTFunctionSegment();
}

size_t PTFunctionCallTree::GetSize() const {
  return m_opaque_ptr ? m_opaque_ptr->size() : 0;
}

void PTFunctionCallTree::SetPtr(
    std::vector<intelpt_private::FunctionSegmentSP> *ptr) {
  m_opaque_ptr = ptr;
}

PTFunctionCallTree::PTFunctionCallTree() : m_opaque_ptr() {}

// PT THREAD TRACE

PTFunctionCallTree PTThreadTrace::GetFunctionCallTree() {
  PTFunctionCallTree call_tree;
  if (m_opaque_ptr)
    call_tree.SetPtr(&m_opaque_ptr->GetFunctionCallTree());
  return call_tree;
}

void PTThreadTrace::GetInstructionLogAtOffset(uint32_t offset, uint32_t count,
                                              PTInstructionList &result_list,
                                              lldb::SBError &sberror) {
  if (!m_opaque_ptr)
    return;

  auto insn_list_sp = std::make_shared<intelpt_private::InstructionList>();
  m_opaque_ptr->GetInstructionLogAtOffset(offset, count, *insn_list_sp,
                                          sberror);
  if (sberror.Success())
    result_list.SetSP(insn_list_sp);
}

void PTThreadTrace::SetPtr(intelpt_private::ThreadTrace *ptr) {
  m_opaque_ptr = ptr;
}

size_t PTThreadTrace::GetPosition() {
  return m_opaque_ptr ? m_opaque_ptr->GetPosition() : 0;
}

void PTThreadTrace::SetPosition(size_t position, lldb::SBError &sberror) {
  if (m_opaque_ptr != nullptr)
    m_opaque_ptr->SetPosition(position, sberror);
}

PTFrameList PTThreadTrace::GetFrames() {
  if (!m_opaque_ptr)
    return PTFrameList();
  auto sp = std::make_shared<intelpt_private::FrameList>();
  m_opaque_ptr->GetFrames(*sp);
  return PTFrameList(sp);
}

PTInstruction PTThreadTrace::GetCurrentInstruction() {
  return m_opaque_ptr ? PTInstruction(m_opaque_ptr->GetCurrentInstruction())
                      : PTInstruction();
}

// PTInstructionList class member functions definitions
size_t PTInstructionList::GetSize() const {
  return (m_opaque_sp ? m_opaque_sp->size() : 0);
}

PTInstruction PTInstructionList::GetInstructionAtIndex(uint32_t idx) {
  return m_opaque_sp ? PTInstruction(m_opaque_sp->at(idx)) : PTInstruction();
}

void PTInstructionList::SetSP(
    std::shared_ptr<intelpt_private::InstructionList> insn_list) {
  m_opaque_sp = insn_list;
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
    const std::shared_ptr<intelpt_private::TraceOptions> &sp) {
  m_opaque_sp = sp;
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


PTThreadTrace PTManager::GetThreadTrace(lldb::SBProcess &sbprocess,
                                        lldb::tid_t tid,
                                        lldb::SBError &sberror) {
  PTThreadTrace thread_trace;
  if (m_opaque_sp == nullptr) {
    sberror.SetErrorString("invalid PTManager instance");
    return thread_trace;
  }

  thread_trace.SetPtr(m_opaque_sp->GetThreadTrace(sbprocess, tid, sberror));
  return thread_trace;
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
