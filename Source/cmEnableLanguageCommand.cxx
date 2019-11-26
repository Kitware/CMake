/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmEnableLanguageCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"

bool cmEnableLanguageCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
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

  status.GetMakefile().EnableLanguage(languages, optional);
  return true;
}
