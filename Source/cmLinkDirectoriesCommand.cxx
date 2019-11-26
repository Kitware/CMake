/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLinkDirectoriesCommand.h"

#include <sstream>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static void AddLinkDir(cmMakefile& mf, std::string const& dir,
                       std::vector<std::string>& directories);

bool cmLinkDirectoriesCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  if (args.empty()) {
    return true;
  }

  cmMakefile& mf = status.GetMakefile();
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

  mf.AddLinkDirectory(cmJoin(directories, ";"), before);

  return true;
}

static void AddLinkDir(cmMakefile& mf, std::string const& dir,
                       std::vector<std::string>& directories)
{
  std::string unixPath = dir;
  cmSystemTools::ConvertToUnixSlashes(unixPath);
  if (!cmSystemTools::FileIsFullPath(unixPath) &&
      !cmGeneratorExpression::StartsWithGeneratorExpression(unixPath)) {
    bool convertToAbsolute = false;
    std::ostringstream e;
    /* clang-format off */
    e << "This command specifies the relative path\n"
      << "  " << unixPath << "\n"
      << "as a link directory.\n";
    /* clang-format on */
    switch (mf.GetPolicyStatus(cmPolicies::CMP0015)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0015);
        mf.IssueMessage(MessageType::AUTHOR_WARNING, e.str());
        break;
      case cmPolicies::OLD:
        // OLD behavior does not convert
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0015);
        mf.IssueMessage(MessageType::FATAL_ERROR, e.str());
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior converts
        convertToAbsolute = true;
        break;
    }
    if (convertToAbsolute) {
      unixPath = cmStrCat(mf.GetCurrentSourceDirectory(), '/', unixPath);
    }
  }
  directories.push_back(unixPath);
}
