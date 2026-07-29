/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmLinkDirectoriesCommand.h"

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static void AddLinkDir(cmMakefile& mf, std::string const& dir,
                       std::vector<std::string>& directories);

bool cmLinkDirectoriesCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  mf.IssueDiagnostic(cmDiagnostics::CMD_NON_TARGET_DIRECTIVE,
                     "Use of non-target directives is not recommended. "
                     "Consider target_link_directories instead.");

  if (args.empty()) {
    return true;
  }

  bool before = mf.IsOn("CMAKE_LINK_DIRECTORIES_BEFORE");

  auto i = args.cbegin();
  if ((*i) == "BEFORE") {
    before = true;
    ++i;
  } else if ((*i) == "AFTER") {
    before = false;
    ++i;
  }

  std::vector<std::string> directories;
  for (; i != args.cend(); ++i) {
    AddLinkDir(mf, *i, directories);
  }

  mf.AddLinkDirectory(cmList::to_string(directories), before);

  return true;
}

static void AddLinkDir(cmMakefile& mf, std::string const& dir,
                       std::vector<std::string>& directories)
{
  std::string unixPath = dir;
  cmSystemTools::ConvertToUnixSlashes(unixPath);
  if (!cmSystemTools::FileIsFullPath(unixPath) &&
      !cmGeneratorExpression::StartsWithGeneratorExpression(unixPath)) {
    unixPath = cmStrCat(mf.GetCurrentSourceDirectory(), '/', unixPath);
  }
  directories.push_back(unixPath);
}
