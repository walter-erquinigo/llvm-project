//===-- Decoder.h -----------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef Decoder_h_
#define Decoder_h_

// C/C++ Includes
#include <cassert>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Frame.h"
#include "FunctionSegment.h"
#include "Instruction.h"
#include "ThreadTrace.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBError.h"
#include "lldb/API/SBProcess.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBStructuredData.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBTrace.h"
#include "lldb/API/SBTraceOptions.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-types.h"

#include "intel-pt.h"

namespace intelpt_private {
/// \class TraceOptions
/// Provides Intel(R) Processor Trace specific configuration options and
///     other information obtained by decoding and post-processing the trace
///     data. Currently, this information comprises of the total number of
///     assembly instructions executed for an inferior.
class TraceOptions : public lldb::SBTraceOptions {
public:
  TraceOptions() : lldb::SBTraceOptions(), m_insn_log_size(0) {}

  ~TraceOptions() {}

  /// Get total number of assembly instructions obtained after decoding the
  /// complete Intel(R) Processor Trace data obtained from LLDB.
  ///
  /// \return
  ///     Total number of instructions.
  uint32_t getInstructionLogSize() const { return m_insn_log_size; }

  /// Set total number of assembly instructions.
  ///
  /// \param[in] size
  ///     Value to be set.
  void setInstructionLogSize(uint32_t size) { m_insn_log_size = size; }

private:
  uint32_t m_insn_log_size;
};

/// \class Decoder
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
class Decoder {
public:

  Decoder(lldb::SBDebugger &sbdebugger)
      : m_mapProcessUID_mapThreadID_TraceInfo_mutex(),
        m_mapProcessUID_mapThreadID_TraceInfo(),
        m_debugger_user_id(sbdebugger.GetID()) {}

  ~Decoder() {}

  void StartProcessorTrace(lldb::SBProcess &sbprocess,
                           lldb::SBTraceOptions &sbtraceoptions,
                           lldb::SBError &sberror);

  void StopProcessorTrace(lldb::SBProcess &sbprocess, lldb::SBError &sberror,
                          lldb::tid_t tid = LLDB_INVALID_THREAD_ID);

  ThreadTrace *GetThreadTrace(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                              lldb::SBError &sberror);

  std::vector<std::shared_ptr<FunctionSegment>> *
  GetFunctionCallTree(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                      lldb::SBError &sberror);

  void GetProcessorTraceInfo(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                             TraceOptions &traceinfo, lldb::SBError &sberror);

private:

  // Check whether the provided SBProcess belongs to the same SBDebugger with
  // which Decoder class instance was constructed.
  void CheckDebuggerID(lldb::SBProcess &sbprocess, lldb::SBError &sberror);

  // Function to remove entries of finished processes/threads in the class
  void RemoveDeadProcessesAndThreads(lldb::SBProcess &sbprocess);

  // Parse cpu information from trace configuration received from LLDB
  void ParseCPUInfo(CPUInfo &pt_cpu, lldb::SBStructuredData &s,
                    lldb::SBError &sberror);

  /// Function performs following tasks for a given process and thread:
  ///  - Checks if the given thread is registered in the class or not. If not
  ///  then tries to register it if trace was ever started on the entire
  ///  process. Else returns error.
  ///  - fetches trace and other necessary information from LLDB (using
  ///  ReadTraceDataAndImageInfo()) and decodes the trace (using
  ///  DecodeProcessorTrace())
  void FetchAndDecode(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                      lldb::SBError &sberror, ThreadTrace **threadTraceInfo);

  // Helper function of FetchAndDecode() to get raw trace data and memory image
  // info of inferior from LLDB
  void ReadTraceDataAndImageInfo(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                                 lldb::SBError &sberror,
                                 ThreadTrace &threadTraceInfo);

  // Helper function of FetchAndDecode() to initialize raw trace decoder and
  // start trace decoding
  void DecodeProcessorTrace(lldb::SBProcess &sbprocess, lldb::tid_t tid,
                            lldb::SBError &sberror,
                            ThreadTrace &threadTraceInfo);

  // Helper function of ReadTraceDataAndImageInfo() function for gathering
  // inferior's memory image info along with all dynamic libraries linked with
  // it
  void GetTargetModulesInfo(lldb::SBTarget &sbtarget,
                            ReadExecuteSectionInfos &readExecuteSectionInfos,
                            lldb::SBError &sberror);

  /// Helper functions of DecodeProcessorTrace() function for:
  ///  - initializing raw trace decoder (provided by Intel(R) Processor Trace
  ///    Decoding library)
  ///  - start trace decoding
  void InitializePTInstDecoder(
      struct pt_insn_decoder **decoder, struct pt_config *config,
      const CPUInfo &pt_cpu, Buffer &pt_buffer,
      const ReadExecuteSectionInfos &readExecuteSectionInfos,
      lldb::SBError &sberror) const;
  void DecodeTrace(struct pt_insn_decoder *decoder,
                   InstructionList &instruction_list);
  int HandlePTInstructionEvents(pt_insn_decoder *decoder, int errcode,
                                InstructionList &instruction_list);

  typedef std::map<lldb::user_id_t, ThreadTrace> MapThreadID_TraceInfo;
  typedef std::map<uint32_t, MapThreadID_TraceInfo>
      MapProcessUID_MapThreadID_TraceInfo;

  std::mutex m_mapProcessUID_mapThreadID_TraceInfo_mutex;
  MapProcessUID_MapThreadID_TraceInfo
      m_mapProcessUID_mapThreadID_TraceInfo; // to store trace information for
                                             // each process and its associated
                                             // threads
  lldb::user_id_t m_debugger_user_id; // SBDebugger instance which is associated
                                      // to this Decoder instance
};

} // namespace intelpt_private
#endif // Decoder_h_
