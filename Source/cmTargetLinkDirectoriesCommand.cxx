/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetLinkDirectoriesCommand.h"

#include "cmGeneratorExpression.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetPropCommandBase.h"

namespace {

class TargetLinkDirectoriesImpl : public cmTargetPropCommandBase
{
public:
  using cmTargetPropCommandBase::cmTargetPropCommandBase;

private:
  void HandleMissingTarget(const std::string& name) override
  {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Cannot specify link directories for target \"", name,
               "\" which is not built by this project."));
  }

  std::string Join(const std::vector<std::string>& content) override;

  bool HandleDirectContent(cmTarget* tgt,
                           const std::vector<std::string>& content,
                           bool prepend, bool /*system*/) override
  {
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    tgt->InsertLinkDirectory(this->Join(content), lfbt, prepend);
    return true; // Successfully handled.
  }
};

std::string TargetLinkDirectoriesImpl::Join(
  const std::vector<std::string>& content)
{
  std::vector<std::string> directories;

  for (const auto& dir : content) {
    auto unixPath = dir;
    cmSystemTools::ConvertToUnixSlashes(unixPath);
    if (!cmSystemTools::FileIsFullPath(unixPath) &&
        !cmGeneratorExpression::StartsWithGeneratorExpression(unixPath)) {
      auto tmp = this->Makefile->GetCurrentSourceDirectory();
      tmp += "/";
      tmp += unixPath;
      unixPath = tmp;
    }
    directories.push_back(unixPath);
  }

  return cmJoin(directories, ";");
}

} // namespace

bool cmTargetLinkDirectoriesCommand(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  return TargetLinkDirectoriesImpl(status).HandleArguments(
    args, "LINK_DIRECTORIES", TargetLinkDirectoriesImpl::PROCESS_BEFORE);
}
