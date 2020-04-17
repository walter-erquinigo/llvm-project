#include "FunctionCallTreeBuilder.h"

#include <cassert>

#include "lldb/API/SBThread.h"
#include "llvm/ADT/StringRef.h"

using namespace intelpt_private;
using namespace lldb;

FunctionCallTreeBuilder::FunctionCallTreeBuilder(lldb::SBProcess &process)
    : m_process(process) {}

bool DidSwitchFunctions(FunctionSegment &prev_segment,
                        lldb::SBFunction &new_sbfunction,
                        lldb::SBSymbol &new_symbol) {
  auto prev_sbfunction = prev_segment.GetSBFunction();
  auto prev_symbol = prev_segment.GetSymbol();

  // I tried comparing adddresses first, but it turns out that sometimes a
  // symbol was referring to a .plt section and sometimes to a .text section
  // depending on which instruction the symbol was creating from. In the end the
  // only good option is comparing the function name.

  // If the symbols are different, then we switched functions
  if (new_symbol.IsValid() && prev_symbol.IsValid() &&
      strcmp(prev_symbol.GetName(), new_symbol.GetName()) != 0)
    return true;

  // If the functions are different, then we switched functions
  if (new_sbfunction.IsValid() && prev_sbfunction.IsValid() &&
      new_sbfunction.GetStartAddress() != prev_sbfunction.GetStartAddress())
    return true;

  // If we lost symbol information, then we switched functions
  if ((prev_sbfunction.IsValid() || prev_symbol.IsValid()) &&
      !new_symbol.IsValid() && !new_sbfunction.IsValid())
    return true;

  // If we gained symbol information, then we switched functions
  if ((new_sbfunction.IsValid() || new_symbol.IsValid()) &&
      !prev_symbol.IsValid() && !prev_sbfunction.IsValid())
    return true;

  return false;
}

void FunctionCallTreeBuilder::UpdateFunctionSegmentsWithErrorInstruction(
    Instruction &insn) {
  if (m_segments.empty())
    m_segments.emplace_back(std::make_shared<FunctionSegment>(&insn, 0));
  else {
    // We don't add multiple consecutive gaps
    if (!m_segments.back()->IsGap()) {
      int level = m_segments.back()->GetLevel();
      m_segments.push_back(std::make_shared<FunctionSegment>(&insn, level));
    }
  }
  // printf("New error segment for %s\n", m_segments.back()->GetFunctionName());
}

/* Find the innermost caller in the back trace of function with
   the same sbfunction/symbol information starting with the provided function
   segment. */
FunctionSegment *GetInnermostCaller(FunctionSegment *segment,
                                    lldb::SBFunction &sbfunction,
                                    lldb::SBSymbol &symbol) {
  for (FunctionSegment *it = segment; it != nullptr; it = it->GetParent()) {
    /* Skip functions with incompatible symbol information.  */
    if (!DidSwitchFunctions(*it, sbfunction, symbol))
      return it;
  }
  return nullptr;
}

/* Find the innermost caller in the back trace of segment, skipping all
   function segments that do not end with a call instruction (e.g.
   tail calls ending with a jump).*/
FunctionSegment *GetInnermostCaller(FunctionSegment *segment) {
  for (FunctionSegment *it = segment; it != nullptr; it = it->GetParent()) {
    /* Skip gaps.  */
    if (it->IsGap())
      continue;

    if (it->GetLastInstruction()->GetInsnClass() == ptic_call)
      return it;
  }

  return nullptr;
}

/* Fix up the caller for all segments of a function.  */
void FixCaller(FunctionSegment &callee, FunctionSegment &caller) {
  callee.SetParent(&caller);

  /* Update all function segments belonging to the same function.  */
  for (FunctionSegment *fprev = callee.GetPrev(); fprev != nullptr;
       fprev = fprev->GetPrev())
    fprev->SetParent(&caller);

  for (FunctionSegment *fnext = callee.GetNext(); fnext != nullptr;
       fnext = fnext->GetNext())
    fnext->SetParent(&caller);
}

/* Add a continuation segment for a function into which we return at the end of
   the trace. */
void FunctionCallTreeBuilder::AppendNewReturnFunctionSegment(
    lldb::SBFunction &sbfunction, lldb::SBSymbol &symbol, Instruction &insn) {
  assert(!m_segments.empty());
  FunctionSegment &prev_segment = *m_segments.back();
  int level = prev_segment.GetLevel();
  m_segments.push_back(std::make_shared<FunctionSegment>(sbfunction, symbol,
                                                         &insn, level,
                                                         /*parent*/ nullptr));
  FunctionSegment &new_segment = *m_segments.back();
  // printf("New return segment for %s\n", new_segment.GetFunctionName());

  // We are looking for prev's caller
  FunctionSegment *caller =
      GetInnermostCaller(prev_segment.GetParent(), sbfunction, symbol);
  if (caller != nullptr) {
    // We are in the same function
    assert(caller->GetNext() == nullptr);
    caller->SetNextSegment(&new_segment);
  } else {
    // We did not find the caller, so maybe the caller was not traced or
    // something went wrong
    caller = GetInnermostCaller(prev_segment.GetParent());
    if (caller == nullptr) {
      /* There is no call in PREV's back trace.  We assume that the
            branch trace did not include it.  */

      /* Let's find the topmost function and add a new caller for it.
        This should handle a series of initial tail calls.  */
      caller = &prev_segment;
      while (caller->GetParent() != nullptr)
        caller = caller->GetParent();
      new_segment.SetLevel(caller->GetLevel() - 1);

      // fix the call stack for caller. Thew new function will be on top
      FixCaller(*caller, new_segment);
    } else {
      /* There is a call in PREV's back trace to which we should have
             returned but didn't.  Let's start a new, separate back trace
             from PREV's level.  */
      new_segment.SetLevel(prev_segment.GetLevel() - 1);

      /* We fix up the back trace for PREV but leave other function m_segments
        on the same level as they are.
        This should handle things like schedule () correctly where we're
        switching contexts.  */
      prev_segment.SetParent(&new_segment);
    }
  }
}

void FunctionCallTreeBuilder::AppendNewTailCallFunctionSegment(
    const lldb::SBFunction &sbfunction, const lldb::SBSymbol &symbol,
    Instruction &insn) {
  assert(!m_segments.empty());
  FunctionSegment *parent = m_segments.back().get();
  int level = parent->GetLevel() + 1;
  m_segments.push_back(std::make_shared<FunctionSegment>(sbfunction, symbol,
                                                         &insn, level, parent));
  // printf("New tail call segment for %s\n",
  // m_segments.back()->GetFunctionName());
}

void FunctionCallTreeBuilder::AppendNewCallFunctionSegment(
    const lldb::SBFunction &sbfunction, const lldb::SBSymbol &symbol,
    Instruction &insn) {
  assert(!m_segments.empty());
  FunctionSegment *parent = m_segments.back().get();
  int level = parent->GetLevel() + 1;
  m_segments.push_back(std::make_shared<FunctionSegment>(sbfunction, symbol,
                                                         &insn, level, parent));
  // printf("New call segment for %s\n", m_segments.back()->GetFunctionName());
}

void FunctionCallTreeBuilder::AppendNewSwitchFunctionSegment(
    const lldb::SBFunction &sbfunction, const lldb::SBSymbol &symbol,
    Instruction &insn) {
  assert(!m_segments.empty());
  FunctionSegment &prev_segment = *m_segments.back();
  m_segments.push_back(std::make_shared<FunctionSegment>(
      sbfunction, symbol, &insn, prev_segment.GetLevel(),
      prev_segment.GetParent()));
  // printf("New switch segment for %s\n",
  // m_segments.back()->GetFunctionName());
}

void FunctionCallTreeBuilder::AppendPC(lldb::tid_t tid) {
  pt_insn raw_current_insn;
  raw_current_insn.ip = m_process.GetThreadByID(tid).GetFrameAtIndex(0).GetPC();
  raw_current_insn.size = 1; // fake
  raw_current_insn.iclass = ptic_other;
  Instruction *current_insn = new Instruction(raw_current_insn);
  AppendInstruction(*current_insn);
}

void FunctionCallTreeBuilder::AppendInstruction(Instruction &insn) {
  Instruction *last_insn =
      m_segments.empty() ? nullptr : m_segments.back()->GetLastInstruction();

  // Errors indicate gaps
  if (insn.IsError()) {
    UpdateFunctionSegmentsWithErrorInstruction(insn);
    return;
  }

  // We got instructions
  lldb::SBAddress address =
      m_process.GetTarget().ResolveLoadAddress(insn.GetInsnAddress());
  lldb::SBFunction sbfunction = address.GetFunction();
  lldb::SBSymbol symbol = address.GetSymbol();

  if (m_segments.empty() || m_segments.back()->IsGap() ||
      last_insn == nullptr) {
    // int level = m_segments.empty() ? 0 : m_segments.back()->GetLevel();
    m_segments.push_back(std::make_shared<FunctionSegment>(sbfunction, symbol,
                                                           &insn, 0,
                                                           /*parent*/ nullptr));
    // printf("New after-gap segment for %s\n",
    //       m_segments.back()->GetFunctionName());
    return;
  }
  FunctionSegment &last_segment = *m_segments.back();

  switch (last_insn->GetInsnClass()) {
  case ptic_call:
    // printf("CALL\n");
    // We ignore calls to the next address. They are used in PIC
    if (last_insn->GetInsnAddress() + last_insn->GetSize() ==
        insn.GetInsnAddress())
      break;
    AppendNewCallFunctionSegment(sbfunction, symbol, insn);
    return;

  case ptic_return: {
    // printf("RETURN\n");
    /* On some systems, _dl_runtime_resolve returns to the resolved
      function instead of jumping to it.  From our perspective,
      however, this is a tailcall.
      If we treated it as return, we wouldn't be able to find the
      resolved function in our stack back trace.  Hence, we would
      lose the current stack back trace and start anew with an empty
      back trace.  When the resolved function returns, we would then
      create a stack back trace with the same function names but
      different frame id's.  This will confuse stepping.  */
    const char *last_segment_name = last_segment.GetFunctionName();
    if (std::strcmp(last_segment_name, "_dl_runtime_resolve") == 0 ||
        std::strcmp(last_segment_name, "_dl_runtime_resolve_xsave") == 0) {
      AppendNewTailCallFunctionSegment(sbfunction, symbol, insn);
      return;
    }
    AppendNewReturnFunctionSegment(sbfunction, symbol, insn);
    return;
  }
  case ptic_jump: {
    // printf("JUMP\n");
    lldb::SBAddress function_start_sbaddress =
        sbfunction.IsValid() ? sbfunction.GetStartAddress()
                             : symbol.GetStartAddress();
    lldb::addr_t function_start_address =
        function_start_sbaddress.GetLoadAddress(m_process.GetTarget());

    // A jump to the start of a function is (typically) a tail call.
    if (function_start_address == insn.GetInsnAddress()) {
      AppendNewTailCallFunctionSegment(sbfunction, symbol, insn);
      return;
    }

    // Some versions of _Unwind_RaiseException use an indirect
    //    jump to 'return' to the exception handler of the caller
    //    handling the exception instead of a return.  Let's restrict
    //    this heuristic to that and related functions.
    if (llvm::StringRef(last_segment.GetFunctionName())
            .startswith("_Unwind_")) {
      FunctionSegment *caller =
          GetInnermostCaller(last_segment.GetParent(), sbfunction, symbol);
      if (caller != nullptr) {
        AppendNewReturnFunctionSegment(sbfunction, symbol, insn);
        return;
      }
    }

    // If we can't determine the function for PC, we treat a jump at
    //    the end of the block as tail call if we're switching functions
    //    and as an intra-function branch if we don't.
    if (function_start_address == LLDB_INVALID_ADDRESS &&
        DidSwitchFunctions(last_segment, sbfunction, symbol)) {
      AppendNewTailCallFunctionSegment(sbfunction, symbol, insn);
      return;
    }
    break;
  }
  default:
    // printf("OTHER\n");
    break;
  }

  if (DidSwitchFunctions(last_segment, sbfunction, symbol)) {
    AppendNewSwitchFunctionSegment(sbfunction, symbol, insn);
    return;
  }

  // No new function was created, so we append the instruction to the last
  // function
  last_segment.AppendInstruction(&insn);
}

void FunctionCallTreeBuilder::Finalize(
    std::vector<std::shared_ptr<FunctionSegment>> &segments) {
  // correct levels. Each contiguous section should have 0 as minimum level
  for (size_t i = 0; i < segments.size();) {
    int min_level = segments[i]->GetLevel();

    size_t j = i;
    if (i == 0 || segments[i - 1]->IsGap()) {
      while (j + 1 < segments.size() && !segments[j + 1]->IsGap()) {
        j++;
        min_level = std::min(min_level, segments[j]->GetLevel());
      }
    }

    for (size_t k = i; k <= j; k++)
      segments[k]->SetLevel(segments[k]->GetLevel() - min_level);
    i = j + 1;
  }

  segments.reserve(m_segments.size());
  segments.assign(m_segments.begin(), m_segments.end());
}
