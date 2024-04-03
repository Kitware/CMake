/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEnableLanguageCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmSystemTools.h"

bool cmEnableLanguageCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  if (!mf.IsNormalDefinitionSet("PROJECT_NAME")) {
    switch (mf.GetPolicyStatus(cmPolicies::CMP0165)) {
      case cmPolicies::WARN:
        mf.IssueMessage(
          MessageType::AUTHOR_WARNING,
          "project() should be called prior to this enable_language() call.");
        break;
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
        mf.IssueMessage(
          MessageType::FATAL_ERROR,
          "project() must be called prior to this enable_language() call.");
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      default:
        break;
    }
  }

  bool optional = false;
  std::vector<std::string> languages;
  for (std::string const& it : args) {
    if (it == "OPTIONAL") {
      optional = true;
    } else {
      languages.push_back(it);
    }
  }

  mf.EnableLanguage(languages, optional);
  return true;
}
