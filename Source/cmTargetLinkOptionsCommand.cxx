/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetLinkOptionsCommand.h"

#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetPropCommandBase.h"

namespace {

class TargetLinkOptionsImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

private:
  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify link options for target \"", name,
               "\" which is not built by this project."));
  }

  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool prepend, bool /*system*/) override
  {
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    tgt->InsertLinkOption(BT<std::string>(this->Join(content), lfbt), prepend);
    return true; // Successfully handled.
  }

  std::string Join(const std::vector<std::string>& content) override
  {
    return cmJoin(content, ";");
  }
};

} // namespace

bool cmTargetLinkOptionsCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  return TargetLinkOptionsImpl(status).HandleArguments(
    args, "LINK_OPTIONS", TargetLinkOptionsImpl::PROCESS_BEFORE);
}
