//===-- PTManager.h ---------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef PTManager_h_
#define PTManager_h_

// C/C++ Includes
#include <vector>

#include "intelpt_private-forward.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBError.h"
#include "lldb/API/SBProcess.h"
#include "lldb/API/SBStructuredData.h"
#include "lldb/API/SBTraceOptions.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-types.h"

namespace intelpt {

/// \class PTInstruction
/// Represents an assembly instruction containing raw
///     instruction bytes, instruction address along with information
///     regarding execution flow context and Intel(R) Processor Trace
///     context.
class PTInstruction {
public:
  PTInstruction();

  PTInstruction(intelpt_private::InstructionSP insn);

  ~PTInstruction();

  // Get instruction address in inferior's memory image
  uint64_t GetInsnAddress() const;

  /// Get raw bytes of the instruction in the buffer.
  ///
  /// \param[out] buf
  ///     The buffer where the raw bytes will be written. This buffer should be
  ///     allocated by the caller of this API. Providing an unallocated buffer
  ///     is an error. In case of errors, the content of the buffer is not
  ///     valid.
  ///
  /// \param[in] size
  ///     Number of raw bytes to be written to @buf. Atleast @size bytes of
  ///     memory should be allocated to @buf otherwise the behaviour of the API
  ///     is undefined. Providing 0 for this argument is an error.
  ///
  /// \return
  ///     Number of bytes of the instruction actually written to @buf if API
  ///     succeeds. In case of errors, total number of raw bytes of the
  ///     instruction is returned.
  size_t GetRawBytes(void *buf, size_t size) const;

  // Get error string if it represents an invalid instruction. For a valid
  // instruction, an empty string is returned
  const char * GetError() const;

  // Instruction was executed speculatively or not
  bool GetSpeculative() const;

  bool IsError() const;

  int GetID() const;

private:
  intelpt_private::InstructionSP m_opaque_sp;
};

/// \class PTInstructionList
/// Represents a list of assembly instructions. Each instruction is of
///     type PTInstruction.
class PTInstructionList {
public:
  // Get number of instructions in the list
  size_t GetSize() const;

  // Get instruction at index
  PTInstruction GetInstructionAtIndex(uint32_t idx);
  void SetSP(std::shared_ptr<intelpt_private::InstructionList> sp);

private:
  std::shared_ptr<intelpt_private::InstructionList> m_opaque_sp;
};

class PTFunctionSegment {
public:
  PTFunctionSegment(intelpt_private::FunctionSegmentSP segment);

  const char *GetFunctionName() const;

  const char *GetDisplayName() const;

  lldb::addr_t GetStartLoadAddress() const;

  lldb::addr_t GetEndLoadAddress() const;

  PTInstruction GetFirstInstruction() const;

  size_t GetID() const;

  int GetLevel() const;
  PTFunctionSegment();

private:
  intelpt_private::FunctionSegmentSP m_opaque_sp;
};

class PTFrame {
public:
  PTFrame();

  PTFrame(intelpt_private::FrameSP sp);

  PTInstruction GetInstruction() const;

  PTFunctionSegment GetFunctionSegment() const;

private:
  intelpt_private::FrameSP m_opaque_sp;
};

class PTFrameList {
public:
  PTFrameList();

  PTFrameList(std::shared_ptr<intelpt_private::FrameList> sp);

  size_t GetNumFrames() const;

  PTFrame GetFrameAtIndex(size_t index) const;

private:
  std::shared_ptr<intelpt_private::FrameList> m_opaque_sp;
};

class PTFunctionCallTree {
public:
  PTFunctionCallTree();

  PTFunctionSegment GetFunctionSegmentAtIndex(size_t index) const;

  size_t GetSize() const;

  void SetPtr(std::vector<intelpt_private::FunctionSegmentSP> *ptr);

private:
  std::vector<intelpt_private::FunctionSegmentSP> *m_opaque_ptr;
};

class PTThreadTrace {
public:
  PTFunctionCallTree GetFunctionCallTree();

  void GetInstructionLogAtOffset(uint32_t offset, uint32_t count,
                                 PTInstructionList &result_list,
                                 lldb::SBError &sberror);

  int GetPosition();

  void SetPosition(int position, lldb::SBError &sberror);

  PTFrameList GetFrames();

  PTInstruction GetCurrentInstruction();

  bool ReverseStepInst();

  bool StepInst();

  bool ReverseStepOver();

  bool StepOver();

  bool Continue();

  bool ReverseContinue();

  bool StepIn();

  bool ReverseStepIn();

  bool StepOut();

  bool ReverseStepOut();

  void SetPtr(intelpt_private::ThreadTrace *ptr);

private:
  intelpt_private::ThreadTrace *m_opaque_ptr;
};

/// \class PTTraceOptions
/// Provides configuration options like trace type, trace buffer size,
///     meta data buffer size along with other Intel(R) Processor Trace
///     specific options.
class PTTraceOptions {
public:
  lldb::TraceType GetType() const;

  uint64_t GetTraceBufferSize() const;

  uint64_t GetMetaDataBufferSize() const;

  /// Get Intel(R) Processor Trace specific configuration options (apart from
  /// trace buffer size, meta data buffer size and TraceType) formatted as
  /// json text i.e. {"Name":Value,"Name":Value} pairs, where "Value" is a
  /// 64-bit unsigned integer in hex format. For "Name", please refer to
  /// SBProcess::StartTrace API description for setting SBTraceOptions.
  ///
  /// \return
  ///     A string formatted as json text {"Name":Value,"Name":Value}
  lldb::SBStructuredData GetTraceParams(lldb::SBError &error);

private:
  friend class PTManager;

  void SetSP(const std::shared_ptr<intelpt_private::TraceOptions> &ptr);

  std::shared_ptr<intelpt_private::TraceOptions> m_opaque_sp;
};

/// \class PTManager
/// This class makes use of Intel(R) Processor Trace hardware feature
///     (implememted inside LLDB) to gather trace data for an inferior (being
///     debugged with LLDB) to provide meaningful information out of it.
///
///     Currently the meaningful information comprises of the execution flow
///     of the inferior (in terms of assembly instructions executed). The class
///     enables user to:
///     - start the trace with configuration options for a thread/process,
///     - stop the trace for a thread/process,
///     - get the execution flow (assembly instructions) for a thread and
///     - get trace specific information for a thread
class PTManager {
public:
  PTManager(lldb::SBDebugger &sbdebugger);

  /// Start Intel(R) Processor Trace on a thread or complete process with
  /// Intel(R) Processor Trace specific configuration options
  ///
  /// \param[in] sbprocess
  ///     A valid process on which this operation will be performed. An error is
  ///     returned in case of an invalid process.
  ///
  /// \param[in] sbtraceoptions
  ///     Contains thread id information and configuration options:
  ///
  ///     For tracing a single thread, provide a valid thread id. If sbprocess
  ///     doesn't contain this thread id, error will be returned. For tracing
  ///     complete process, set it to lldb::LLDB_INVALID_THREAD_ID
  ///     Configuration options comprises of:
  ///     a) trace buffer size, meta data buffer size, TraceType and
  ///     b) All other possible Intel(R) Processor Trace specific configuration
  ///     options (hereafter collectively referred as CUSTOM_OPTIONS), formatted
  ///     as json text i.e. {"Name":Value,"Name":Value,..} inside
  ///     sbtraceoptions, where "Value" should be a 64-bit unsigned integer in
  ///     hex format. For information regarding what all configuration options
  ///     are currently supported by LLDB and detailed information about
  ///     CUSTOM_OPTIONS usage, please refer to SBProcess::StartTrace() API
  ///     description. To know about all possible configuration options of
  ///     Intel(R) Processor Trace, please refer to Intel(R) 64 and IA-32
  ///     Architectures Software Developer's Manual.
  ///
  ///     TraceType should be set to lldb::TraceType::eTraceTypeProcessorTrace,
  ///     else error is returned. To find out any other requirement to start
  ///     tracing successfully, please refer to SBProcess::StartTrace() API
  ///     description. LLDB's current implementation of Intel(R) Processor Trace
  ///     feature may round off invalid values for configuration options.
  ///     Therefore, the configuration options with which the trace was actually
  ///     started, might be different to the ones with which trace was asked to
  ///     be started by user. The actual used configuration options can be
  ///     obtained from GetProcessorTraceInfo() API.
  ///
  /// \param[out] sberror
  ///     An error with the failure reason if API fails. Else success.
  void StartProcessorTrace(lldb::SBProcess &sbprocess,
                           lldb::SBTraceOptions &sbtraceoptions,
                           lldb::SBError &sberror);

  /// Stop Intel(R) Processor Trace on a thread or complete process.
  ///
  /// \param[in] sbprocess
  ///     A valid process on which this operation will be performed. An error is
  ///     returned in case of an invalid process.
  ///
  /// \param[in] tid
  ///     Case 1: To stop tracing a single thread, provide a valid thread id. If
  ///     sbprocess doesn't contain the thread tid, error will be returned.
  ///     Case 2: To stop tracing complete process, use
  ///     lldb::LLDB_INVALID_THREAD_ID.
  ///
  /// \param[out] sberror
  ///     An error with the failure reason if API fails. Else success.
  void StopProcessorTrace(lldb::SBProcess &sbprocess, lldb::SBError &sberror,
                          lldb::tid_t tid = LLDB_INVALID_THREAD_ID);

  PTThreadTrace GetThreadTrace(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                               lldb::SBError &sberror);

  /// Get Intel(R) Processor Trace specific information for a thread of a
  /// process. The information contains the actual configuration options with
  /// which the trace was started for this thread.
  ///
  /// \param[in] sbprocess
  ///     A valid process on which this operation will be performed. An error is
  ///     returned in case of an invalid process.
  ///
  /// \param[in] tid
  ///     A valid thread id of the thread for which the trace specific
  ///     information is required. If sbprocess doesn't contain the thread tid,
  ///     an error will be returned.
  ///
  /// \param[out] options
  ///     Contains actual configuration options (they may be different to the
  ///     ones with which tracing was asked to be started for this thread during
  ///     StartProcessorTrace() API call).
  ///
  /// \param[out] sberror
  ///     An error with the failure reason if API fails. Else success.
  void GetProcessorTraceInfo(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                             PTTraceOptions &options, lldb::SBError &sberror);

private:
  std::shared_ptr<intelpt_private::Decoder> m_opaque_sp;
};

} // namespace intelpt
#endif // PTManager_h_
