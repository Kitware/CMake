/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetPropertyCommand.h"

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmInstalledFile.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmProperty.h"
#include "cmPropertyDefinition.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetPropertyComputer.h"
#include "cmTest.h"
#include "cmake.h"

class cmMessenger;

namespace {
enum OutType
{
  OutValue,
  OutDefined,
  OutBriefDoc,
  OutFullDoc,
  OutSet
};

// Implementation of result storage.
bool StoreResult(OutType infoType, cmMakefile& makefile,
                 const std::string& variable, const char* value);

// Implementation of each property type.
bool HandleGlobalMode(cmExecutionStatus& status, const std::string& name,
                      OutType infoType, const std::string& variable,
                      const std::string& propertyName);
bool HandleDirectoryMode(cmExecutionStatus& status, const std::string& name,
                         OutType infoType, const std::string& variable,
                         const std::string& propertyName);
bool HandleTargetMode(cmExecutionStatus& status, const std::string& name,
                      OutType infoType, const std::string& variable,
                      const std::string& propertyName);
bool HandleSourceMode(cmExecutionStatus& status, const std::string& name,
                      OutType infoType, const std::string& variable,
                      const std::string& propertyName);
bool HandleTestMode(cmExecutionStatus& status, const std::string& name,
                    OutType infoType, const std::string& variable,
                    const std::string& propertyName);
bool HandleVariableMode(cmExecutionStatus& status, const std::string& name,
                        OutType infoType, const std::string& variable,
                        const std::string& propertyName);
bool HandleCacheMode(cmExecutionStatus& status, const std::string& name,
                     OutType infoType, const std::string& variable,
                     const std::string& propertyName);
bool HandleInstallMode(cmExecutionStatus& status, const std::string& name,
                       OutType infoType, const std::string& variable,
                       const std::string& propertyName);
}

bool cmGetPropertyCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  OutType infoType = OutValue;
  if (args.size() < 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // The cmake variable in which to store the result.
  const std::string variable = args[0];

  std::string name;
  std::string propertyName;

  // Get the scope from which to get the property.
  cmProperty::ScopeType scope;
  if (args[1] == "GLOBAL") {
    scope = cmProperty::GLOBAL;
  } else if (args[1] == "DIRECTORY") {
    scope = cmProperty::DIRECTORY;
  } else if (args[1] == "TARGET") {
    scope = cmProperty::TARGET;
  } else if (args[1] == "SOURCE") {
    scope = cmProperty::SOURCE_FILE;
  } else if (args[1] == "TEST") {
    scope = cmProperty::TEST;
  } else if (args[1] == "VARIABLE") {
    scope = cmProperty::VARIABLE;
  } else if (args[1] == "CACHE") {
    scope = cmProperty::CACHE;
  } else if (args[1] == "INSTALL") {
    scope = cmProperty::INSTALL;
  } else {
    status.SetError(cmStrCat(
      "given invalid scope ", args[1],
      ".  "
      "Valid scopes are "
      "GLOBAL, DIRECTORY, TARGET, SOURCE, TEST, VARIABLE, CACHE, INSTALL."));
    return false;
  }

  // Parse remaining arguments.
  enum Doing
  {
    DoingNone,
    DoingName,
    DoingProperty,
    DoingType
  };
  Doing doing = DoingName;
  for (unsigned int i = 2; i < args.size(); ++i) {
    if (args[i] == "PROPERTY") {
      doing = DoingProperty;
    } else if (args[i] == "BRIEF_DOCS") {
      doing = DoingNone;
      infoType = OutBriefDoc;
    } else if (args[i] == "FULL_DOCS") {
      doing = DoingNone;
      infoType = OutFullDoc;
    } else if (args[i] == "SET") {
      doing = DoingNone;
      infoType = OutSet;
    } else if (args[i] == "DEFINED") {
      doing = DoingNone;
      infoType = OutDefined;
    } else if (doing == DoingName) {
      doing = DoingNone;
      name = args[i];
    } else if (doing == DoingProperty) {
      doing = DoingNone;
      propertyName = args[i];
    } else {
      status.SetError(cmStrCat("given invalid argument \"", args[i], "\"."));
      return false;
    }
  }

  // Make sure a property name was found.
  if (propertyName.empty()) {
    status.SetError("not given a PROPERTY <name> argument.");
    return false;
  }

  // Compute requested output.
  if (infoType == OutBriefDoc) {
    // Lookup brief documentation.
    std::string output;
    if (cmPropertyDefinition const* def =
          status.GetMakefile().GetState()->GetPropertyDefinition(propertyName,
                                                                 scope)) {
      output = def->GetShortDescription();
    } else {
      output = "NOTFOUND";
    }
    status.GetMakefile().AddDefinition(variable, output);
  } else if (infoType == OutFullDoc) {
    // Lookup full documentation.
    std::string output;
    if (cmPropertyDefinition const* def =
          status.GetMakefile().GetState()->GetPropertyDefinition(propertyName,
                                                                 scope)) {
      output = def->GetFullDescription();
    } else {
      output = "NOTFOUND";
    }
    status.GetMakefile().AddDefinition(variable, output);
  } else if (infoType == OutDefined) {
    // Lookup if the property is defined
    if (status.GetMakefile().GetState()->GetPropertyDefinition(propertyName,
                                                               scope)) {
      status.GetMakefile().AddDefinition(variable, "1");
    } else {
      status.GetMakefile().AddDefinition(variable, "0");
    }
  } else {
    // Dispatch property getting.
    switch (scope) {
      case cmProperty::GLOBAL:
        return HandleGlobalMode(status, name, infoType, variable,
                                propertyName);
      case cmProperty::DIRECTORY:
        return HandleDirectoryMode(status, name, infoType, variable,
                                   propertyName);
      case cmProperty::TARGET:
        return HandleTargetMode(status, name, infoType, variable,
                                propertyName);
      case cmProperty::SOURCE_FILE:
        return HandleSourceMode(status, name, infoType, variable,
                                propertyName);
      case cmProperty::TEST:
        return HandleTestMode(status, name, infoType, variable, propertyName);
      case cmProperty::VARIABLE:
        return HandleVariableMode(status, name, infoType, variable,
                                  propertyName);
      case cmProperty::CACHE:
        return HandleCacheMode(status, name, infoType, variable, propertyName);
      case cmProperty::INSTALL:
        return HandleInstallMode(status, name, infoType, variable,
                                 propertyName);

      case cmProperty::CACHED_VARIABLE:
        break; // should never happen
    }
  }

  return true;
}

namespace {

bool StoreResult(OutType infoType, cmMakefile& makefile,
                 const std::string& variable, const char* value)
{
  if (infoType == OutSet) {
    makefile.AddDefinition(variable, value ? "1" : "0");
  } else // if(infoType == OutValue)
  {
    if (value) {
      makefile.AddDefinition(variable, value);
    } else {
      makefile.RemoveDefinition(variable);
    }
  }
  return true;
}

bool HandleGlobalMode(cmExecutionStatus& status, const std::string& name,
                      OutType infoType, const std::string& variable,
                      const std::string& propertyName)
{
  if (!name.empty()) {
    status.SetError("given name for GLOBAL scope.");
    return false;
  }

  // Get the property.
  cmake* cm = status.GetMakefile().GetCMakeInstance();
  return StoreResult(infoType, status.GetMakefile(), variable,
                     cm->GetState()->GetGlobalProperty(propertyName));
}

bool HandleDirectoryMode(cmExecutionStatus& status, const std::string& name,
                         OutType infoType, const std::string& variable,
                         const std::string& propertyName)
{
  // Default to the current directory.
  cmMakefile* mf = &status.GetMakefile();

  // Lookup the directory if given.
  if (!name.empty()) {
    // Construct the directory name.  Interpret relative paths with
    // respect to the current directory.
    std::string dir = name;
    if (!cmSystemTools::FileIsFullPath(dir)) {
      dir =
        cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/', name);
    }

    // The local generators are associated with collapsed paths.
    dir = cmSystemTools::CollapseFullPath(dir);

    // Lookup the generator.
    mf = status.GetMakefile().GetGlobalGenerator()->FindMakefile(dir);
    if (!mf) {
      // Could not find the directory.
      status.SetError(
        "DIRECTORY scope provided but requested directory was not found. "
        "This could be because the directory argument was invalid or, "
        "it is valid but has not been processed yet.");
      return false;
    }
  }

  if (propertyName == "DEFINITIONS") {
    switch (mf->GetPolicyStatus(cmPolicies::CMP0059)) {
      case cmPolicies::WARN:
        mf->IssueMessage(MessageType::AUTHOR_WARNING,
                         cmPolicies::GetPolicyWarning(cmPolicies::CMP0059));
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        return StoreResult(infoType, status.GetMakefile(), variable,
                           mf->GetDefineFlagsCMP0059());
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
        break;
    }
  }

  // Get the property.
  return StoreResult(infoType, status.GetMakefile(), variable,
                     mf->GetProperty(propertyName));
}

bool HandleTargetMode(cmExecutionStatus& status, const std::string& name,
                      OutType infoType, const std::string& variable,
                      const std::string& propertyName)
{
  if (name.empty()) {
    status.SetError("not given name for TARGET scope.");
    return false;
  }

  if (cmTarget* target = status.GetMakefile().FindTargetToUse(name)) {
    if (propertyName == "ALIASED_TARGET") {
      if (status.GetMakefile().IsAlias(name)) {
        return StoreResult(infoType, status.GetMakefile(), variable,
                           target->GetName().c_str());
      }
      return StoreResult(infoType, status.GetMakefile(), variable, nullptr);
    }
    const char* prop_cstr = nullptr;
    cmListFileBacktrace bt = status.GetMakefile().GetBacktrace();
    cmMessenger* messenger = status.GetMakefile().GetMessenger();
    if (cmTargetPropertyComputer::PassesWhitelist(
          target->GetType(), propertyName, messenger, bt)) {
      prop_cstr = target->GetComputedProperty(propertyName, messenger, bt);
      if (!prop_cstr) {
        prop_cstr = target->GetProperty(propertyName);
      }
    }
    return StoreResult(infoType, status.GetMakefile(), variable, prop_cstr);
  }
  status.SetError(cmStrCat("could not find TARGET ", name,
                           ".  Perhaps it has not yet been created."));
  return false;
}

bool HandleSourceMode(cmExecutionStatus& status, const std::string& name,
                      OutType infoType, const std::string& variable,
                      const std::string& propertyName)
{
  if (name.empty()) {
    status.SetError("not given name for SOURCE scope.");
    return false;
  }

  // Get the source file.
  if (cmSourceFile* sf = status.GetMakefile().GetOrCreateSource(name)) {
    return StoreResult(infoType, status.GetMakefile(), variable,
                       sf->GetPropertyForUser(propertyName));
  }
  status.SetError(
    cmStrCat("given SOURCE name that could not be found or created: ", name));
  return false;
}

bool HandleTestMode(cmExecutionStatus& status, const std::string& name,
                    OutType infoType, const std::string& variable,
                    const std::string& propertyName)
{
  if (name.empty()) {
    status.SetError("not given name for TEST scope.");
    return false;
  }

  // Loop over all tests looking for matching names.
  if (cmTest* test = status.GetMakefile().GetTest(name)) {
    return StoreResult(infoType, status.GetMakefile(), variable,
                       test->GetProperty(propertyName));
  }

  // If not found it is an error.
  status.SetError(cmStrCat("given TEST name that does not exist: ", name));
  return false;
}

bool HandleVariableMode(cmExecutionStatus& status, const std::string& name,
                        OutType infoType, const std::string& variable,
                        const std::string& propertyName)
{
  if (!name.empty()) {
    status.SetError("given name for VARIABLE scope.");
    return false;
  }

  return StoreResult(infoType, status.GetMakefile(), variable,
                     status.GetMakefile().GetDefinition(propertyName));
}

bool HandleCacheMode(cmExecutionStatus& status, const std::string& name,
                     OutType infoType, const std::string& variable,
                     const std::string& propertyName)
{
  if (name.empty()) {
    status.SetError("not given name for CACHE scope.");
    return false;
  }

  const char* value = nullptr;
  if (status.GetMakefile().GetState()->GetCacheEntryValue(name)) {
    value = status.GetMakefile().GetState()->GetCacheEntryProperty(
      name, propertyName);
  }
  StoreResult(infoType, status.GetMakefile(), variable, value);
  return true;
}

bool HandleInstallMode(cmExecutionStatus& status, const std::string& name,
                       OutType infoType, const std::string& variable,
                       const std::string& propertyName)
{
  if (name.empty()) {
    status.SetError("not given name for INSTALL scope.");
    return false;
  }

  // Get the installed file.
  cmake* cm = status.GetMakefile().GetCMakeInstance();

  if (cmInstalledFile* file =
        cm->GetOrCreateInstalledFile(&status.GetMakefile(), name)) {
    std::string value;
    bool isSet = file->GetProperty(propertyName, value);

    return StoreResult(infoType, status.GetMakefile(), variable,
                       isSet ? value.c_str() : nullptr);
  }
  status.SetError(
    cmStrCat("given INSTALL name that could not be found or created: ", name));
  return false;
}
}
