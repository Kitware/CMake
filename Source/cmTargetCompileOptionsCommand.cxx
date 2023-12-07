/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetCompileOptionsCommand.h"

#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetPropCommandBase.h"

namespace {

class TargetCompileOptionsImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

private:
  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify compile options for target \"", name,
               "\" which is not built by this project."));
  }

  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool prepend, bool /*system*/) override
  {
    cmPolicies::PolicyStatus policyStatus =
      this->Makefile->GetPolicyStatus(cmPolicies::CMP0101);
    if (policyStatus == cmPolicies::OLD || policyStatus == cmPolicies::WARN) {
      prepend = false;
    }

    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    tgt->InsertCompileOption(BT<std::string>(this->Join(content), lfbt),
                             prepend);
    return true; // Successfully handled.
  }

  std::string Join(const std::vector<std::string>& content) override
  {
    return cmList::to_string(content);
  }
};

} // namespace

bool cmTargetCompileOptionsCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  return TargetCompileOptionsImpl(status).HandleArguments(
    args, "COMPILE_OPTIONS", TargetCompileOptionsImpl::PROCESS_BEFORE);
}
