/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDefinePropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"

bool cmDefinePropertyCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // Get the scope in which to define the property.
  cmProperty::ScopeType scope;
  std::string const& scope_arg = args[0];

  if (scope_arg == "GLOBAL") {
    scope = cmProperty::GLOBAL;
  } else if (scope_arg == "DIRECTORY") {
    scope = cmProperty::DIRECTORY;
  } else if (scope_arg == "TARGET") {
    scope = cmProperty::TARGET;
  } else if (scope_arg == "SOURCE") {
    scope = cmProperty::SOURCE_FILE;
  } else if (scope_arg == "TEST") {
    scope = cmProperty::TEST;
  } else if (scope_arg == "VARIABLE") {
    scope = cmProperty::VARIABLE;
  } else if (scope_arg == "CACHED_VARIABLE") {
    scope = cmProperty::CACHED_VARIABLE;
  } else {
    status.SetError(cmStrCat("given invalid scope ", scope_arg,
                             ".  Valid scopes are GLOBAL, DIRECTORY, TARGET, "
                             "SOURCE, TEST, VARIABLE, CACHED_VARIABLE."));
    return false;
  }

  // Parse remaining arguments.
  bool inherited = false;
  std::string PropertyName;
  std::string BriefDocs;
  std::string FullDocs;
  enum Doing
  {
    DoingNone,
    DoingProperty,
    DoingBrief,
    DoingFull
  };
  Doing doing = DoingNone;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "PROPERTY") {
      doing = DoingProperty;
    } else if (args[i] == "BRIEF_DOCS") {
      doing = DoingBrief;
    } else if (args[i] == "FULL_DOCS") {
      doing = DoingFull;
    } else if (args[i] == "INHERITED") {
      doing = DoingNone;
      inherited = true;
    } else if (doing == DoingProperty) {
      doing = DoingNone;
      PropertyName = args[i];
    } else if (doing == DoingBrief) {
      BriefDocs += args[i];
    } else if (doing == DoingFull) {
      FullDocs += args[i];
    } else {
      status.SetError(cmStrCat("given invalid argument \"", args[i], "\"."));
      return false;
    }
  }

  // Make sure a property name was found.
  if (PropertyName.empty()) {
    status.SetError("not given a PROPERTY <name> argument.");
    return false;
  }

  // Make sure documentation was given.
  if (BriefDocs.empty()) {
    status.SetError("not given a BRIEF_DOCS <brief-doc> argument.");
    return false;
  }
  if (FullDocs.empty()) {
    status.SetError("not given a FULL_DOCS <full-doc> argument.");
    return false;
  }

  // Actually define the property.
  status.GetMakefile().GetState()->DefineProperty(
    PropertyName, scope, BriefDocs.c_str(), FullDocs.c_str(), inherited);

  return true;
}
