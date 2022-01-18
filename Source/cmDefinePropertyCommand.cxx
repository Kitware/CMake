/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDefinePropertyCommand.h"

#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmRange.h"
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
  std::vector<std::string> BriefDocs;
  std::vector<std::string> FullDocs;

  cmArgumentParser<void> parser;
  parser.Bind("PROPERTY"_s, PropertyName);
  parser.Bind("BRIEF_DOCS"_s, BriefDocs);
  parser.Bind("FULL_DOCS"_s, FullDocs);
  parser.Bind("INHERITED"_s, inherited);
  std::vector<std::string> invalidArgs;

  parser.Parse(cmMakeRange(args).advance(1), &invalidArgs);
  if (!invalidArgs.empty()) {
    status.SetError(
      cmStrCat("given invalid argument \"", invalidArgs.front(), "\"."));
    return false;
  }

  // Make sure a property name was found.
  if (PropertyName.empty()) {
    status.SetError("not given a PROPERTY <name> argument.");
    return false;
  }

  // Actually define the property.
  status.GetMakefile().GetState()->DefineProperty(
    PropertyName, scope, cmJoin(BriefDocs, ""), cmJoin(FullDocs, ""),
    inherited);

  return true;
}
