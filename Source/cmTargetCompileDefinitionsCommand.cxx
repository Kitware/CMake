/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetCompileDefinitionsCommand.h"

#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetPropCommandBase.h"

namespace {

class TargetCompileDefinitionsImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

private:
  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify compile definitions for target \"", name,
               "\" which is not built by this project."));
  }

  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool /*prepend*/, bool /*system*/) override
  {
    tgt->AppendProperty("COMPILE_DEFINITIONS", this->Join(content),
                        this->Makefile->GetBacktrace());
    return true; // Successfully handled.
  }

  std::string Join(const std::vector<std::string>& content) override
  {
    std::string defs;
    std::string sep;
    for (std::string const& it : content) {
      if (cmHasLiteralPrefix(it, "-D")) {
        defs += sep + it.substr(2);
      } else {
        defs += sep + it;
      }
      sep = ";";
    }
    return defs;
  }
};

} // namespace

bool cmTargetCompileDefinitionsCommand(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  return TargetCompileDefinitionsImpl(status).HandleArguments(
    args, "COMPILE_DEFINITIONS");
}
