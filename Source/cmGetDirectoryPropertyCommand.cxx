/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetDirectoryPropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
void StoreResult(cmMakefile& makefile, std::string const& variable,
                 const char* prop);
}

// cmGetDirectoryPropertyCommand
bool cmGetDirectoryPropertyCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  auto i = args.begin();
  std::string const& variable = *i;
  ++i;

  // get the directory argument if there is one
  cmMakefile* dir = &status.GetMakefile();
  if (*i == "DIRECTORY") {
    ++i;
    if (i == args.end()) {
      status.SetError(
        "DIRECTORY argument provided without subsequent arguments");
      return false;
    }
    std::string sd = *i;
    // make sure the start dir is a full path
    if (!cmSystemTools::FileIsFullPath(sd)) {
      sd = cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', *i);
    }

    // The local generators are associated with collapsed paths.
    sd = cmSystemTools::CollapseFullPath(sd);

    // lookup the makefile from the directory name
    dir = status.GetMakefile().GetGlobalGenerator()->FindMakefile(sd);
    if (!dir) {
      status.SetError(
        "DIRECTORY argument provided but requested directory not found. "
        "This could be because the directory argument was invalid or, "
        "it is valid but has not been processed yet.");
      return false;
    }
    ++i;
  }

  // OK, now we have the directory to process, we just get the requested
  // information out of it

  if (*i == "DEFINITION") {
    ++i;
    if (i == args.end()) {
      status.SetError("A request for a variable definition was made without "
                      "providing the name of the variable to get.");
      return false;
    }
    std::string const& output = dir->GetSafeDefinition(*i);
    status.GetMakefile().AddDefinition(variable, output);
    return true;
  }

  const char* prop = nullptr;
  if (!i->empty()) {
    if (*i == "DEFINITIONS") {
      switch (status.GetMakefile().GetPolicyStatus(cmPolicies::CMP0059)) {
        case cmPolicies::WARN:
          status.GetMakefile().IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmPolicies::GetPolicyWarning(cmPolicies::CMP0059));
          CM_FALLTHROUGH;
        case cmPolicies::OLD:
          StoreResult(status.GetMakefile(), variable,
                      status.GetMakefile().GetDefineFlagsCMP0059());
          return true;
        case cmPolicies::NEW:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::REQUIRED_IF_USED:
          break;
      }
    }
    prop = dir->GetProperty(*i);
  }
  StoreResult(status.GetMakefile(), variable, prop);
  return true;
}

namespace {
void StoreResult(cmMakefile& makefile, std::string const& variable,
                 const char* prop)
{
  makefile.AddDefinition(variable, prop ? prop : "");
}
}
