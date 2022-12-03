/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetIncludeDirectoriesCommand.h"

#include <set>

#include "cmGeneratorExpression.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetPropCommandBase.h"

namespace {

class TargetIncludeDirectoriesImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

private:
  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify include directories for target \"", name,
               "\" which is not built by this project."));
  }

  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool prepend, bool system) override;

  void HandleInterfaceContent(cmTarget* tgt,
                              const std::vector<std::string>& content,
                              bool prepend, bool system) override;

  std::string Join(const std::vector<std::string>& content) override;
};

std::string TargetIncludeDirectoriesImpl::Join(
  const std::vector<std::string>& content)
{
  std::string dirs;
  std::string sep;
  std::string prefix = this->Makefile->GetCurrentSourceDirectory() + "/";
  for (std::string const& it : content) {
    if (cmSystemTools::FileIsFullPath(it) ||
        cmGeneratorExpression::Find(it) == 0) {
      dirs += cmStrCat(sep, it);
    } else {
      dirs += cmStrCat(sep, prefix, it);
    }
    sep = ";";
  }
  return dirs;
}

bool TargetIncludeDirectoriesImpl::HandleDirectContent(
  cmTarget* tgt, const std::vector<std::string>& content, bool prepend,
  bool system)
{
  cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
  tgt->InsertInclude(BT<std::string>(this->Join(content), lfbt), prepend);
  if (system) {
    std::string prefix = this->Makefile->GetCurrentSourceDirectory() + "/";
    std::set<std::string> sdirs;
    for (std::string const& it : content) {
      if (cmSystemTools::FileIsFullPath(it) ||
          cmGeneratorExpression::Find(it) == 0) {
        sdirs.insert(it);
      } else {
        sdirs.insert(prefix + it);
      }
    }
    tgt->AddSystemIncludeDirectories(sdirs);
  }
  return true; // Successfully handled.
}

void TargetIncludeDirectoriesImpl::HandleInterfaceContent(
  cmTarget* tgt, const std::vector<std::string>& content, bool prepend,
  bool system)
{
  this->cmTargetPropCommandBase::HandleInterfaceContent(tgt, content, prepend,
                                                        system);
  if (system) {
    std::string joined = this->Join(content);
    tgt->AppendProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES", joined,
                        this->Makefile->GetBacktrace());
  }
}

} // namespace

bool cmTargetIncludeDirectoriesCommand(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  return TargetIncludeDirectoriesImpl(status).HandleArguments(
    args, "INCLUDE_DIRECTORIES",
    TargetIncludeDirectoriesImpl::PROCESS_BEFORE |
      TargetIncludeDirectoriesImpl::PROCESS_AFTER |
      TargetIncludeDirectoriesImpl::PROCESS_SYSTEM);
}
