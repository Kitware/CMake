/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetCompileFeaturesCommand.h"

#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStandardLevelResolver.h"
#include "cmStringAlgorithms.h"
#include "cmTargetPropCommandBase.h"

class cmTarget;

namespace {

class TargetCompileFeaturesImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

private:
  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify compile features for target \"", name,
               "\" which is not built by this project."));
  }

  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool /*prepend*/, bool /*system*/) override
  {
    cmStandardLevelResolver standardResolver(this->Makefile);
    for (std::string const& it : content) {
      std::string error;
      if (!standardResolver.AddRequiredTargetFeature(tgt, it, &error)) {
        this->SetError(error);
        return false; // Not (successfully) handled.
      }
    }
    return true; // Successfully handled.
  }

  std::string Join(const std::vector<std::string>& content) override
  {
    return cmList::to_string(content);
  }
};

} // namespace

bool cmTargetCompileFeaturesCommand(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  return TargetCompileFeaturesImpl(status).HandleArguments(args,
                                                           "COMPILE_FEATURES");
}
