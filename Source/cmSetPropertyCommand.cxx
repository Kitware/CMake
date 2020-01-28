/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetPropertyCommand.h"

#include <set>
#include <sstream>

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmInstalledFile.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTest.h"
#include "cmake.h"

namespace {
bool HandleGlobalMode(cmExecutionStatus& status,
                      const std::set<std::string>& names,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove);
bool HandleDirectoryMode(cmExecutionStatus& status,
                         const std::set<std::string>& names,
                         const std::string& propertyName,
                         const std::string& propertyValue, bool appendAsString,
                         bool appendMode, bool remove);
bool HandleTargetMode(cmExecutionStatus& status,
                      const std::set<std::string>& names,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove);
bool HandleTarget(cmTarget* target, cmMakefile& makefile,
                  const std::string& propertyName,
                  const std::string& propertyValue, bool appendAsString,
                  bool appendMode, bool remove);
bool HandleSourceMode(cmExecutionStatus& status,
                      const std::set<std::string>& names,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove);
bool HandleSource(cmSourceFile* sf, const std::string& propertyName,
                  const std::string& propertyValue, bool appendAsString,
                  bool appendMode, bool remove);
bool HandleTestMode(cmExecutionStatus& status, std::set<std::string>& names,
                    const std::string& propertyName,
                    const std::string& propertyValue, bool appendAsString,
                    bool appendMode, bool remove);
bool HandleTest(cmTest* test, const std::string& propertyName,
                const std::string& propertyValue, bool appendAsString,
                bool appendMode, bool remove);
bool HandleCacheMode(cmExecutionStatus& status,
                     const std::set<std::string>& names,
                     const std::string& propertyName,
                     const std::string& propertyValue, bool appendAsString,
                     bool appendMode, bool remove);
bool HandleCacheEntry(std::string const& cacheKey, const cmMakefile& makefile,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove);
bool HandleInstallMode(cmExecutionStatus& status,
                       const std::set<std::string>& names,
                       const std::string& propertyName,
                       const std::string& propertyValue, bool appendAsString,
                       bool appendMode, bool remove);
bool HandleInstall(cmInstalledFile* file, cmMakefile& makefile,
                   const std::string& propertyName,
                   const std::string& propertyValue, bool appendAsString,
                   bool appendMode, bool remove);
}

bool cmSetPropertyCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // Get the scope on which to set the property.
  std::string const& scopeName = args.front();
  cmProperty::ScopeType scope;
  if (scopeName == "GLOBAL") {
    scope = cmProperty::GLOBAL;
  } else if (scopeName == "DIRECTORY") {
    scope = cmProperty::DIRECTORY;
  } else if (scopeName == "TARGET") {
    scope = cmProperty::TARGET;
  } else if (scopeName == "SOURCE") {
    scope = cmProperty::SOURCE_FILE;
  } else if (scopeName == "TEST") {
    scope = cmProperty::TEST;
  } else if (scopeName == "CACHE") {
    scope = cmProperty::CACHE;
  } else if (scopeName == "INSTALL") {
    scope = cmProperty::INSTALL;
  } else {
    status.SetError(cmStrCat("given invalid scope ", scopeName,
                             ".  "
                             "Valid scopes are GLOBAL, DIRECTORY, "
                             "TARGET, SOURCE, TEST, CACHE, INSTALL."));
    return false;
  }

  bool appendAsString = false;
  bool appendMode = false;
  bool remove = true;
  std::set<std::string> names;
  std::string propertyName;
  std::string propertyValue;

  // Parse the rest of the arguments up to the values.
  enum Doing
  {
    DoingNone,
    DoingNames,
    DoingProperty,
    DoingValues
  };
  Doing doing = DoingNames;
  const char* sep = "";
  for (std::string const& arg : cmMakeRange(args).advance(1)) {
    if (arg == "PROPERTY") {
      doing = DoingProperty;
    } else if (arg == "APPEND") {
      doing = DoingNone;
      appendMode = true;
      remove = false;
      appendAsString = false;
    } else if (arg == "APPEND_STRING") {
      doing = DoingNone;
      appendMode = true;
      remove = false;
      appendAsString = true;
    } else if (doing == DoingNames) {
      names.insert(arg);
    } else if (doing == DoingProperty) {
      propertyName = arg;
      doing = DoingValues;
    } else if (doing == DoingValues) {
      propertyValue += sep;
      sep = ";";
      propertyValue += arg;
      remove = false;
    } else {
      status.SetError(cmStrCat("given invalid argument \"", arg, "\"."));
      return false;
    }
  }

  // Make sure a property name was found.
  if (propertyName.empty()) {
    status.SetError("not given a PROPERTY <name> argument.");
    return false;
  }

  // Dispatch property setting.
  switch (scope) {
    case cmProperty::GLOBAL:
      return HandleGlobalMode(status, names, propertyName, propertyValue,
                              appendAsString, appendMode, remove);
    case cmProperty::DIRECTORY:
      return HandleDirectoryMode(status, names, propertyName, propertyValue,
                                 appendAsString, appendMode, remove);
    case cmProperty::TARGET:
      return HandleTargetMode(status, names, propertyName, propertyValue,
                              appendAsString, appendMode, remove);
    case cmProperty::SOURCE_FILE:
      return HandleSourceMode(status, names, propertyName, propertyValue,
                              appendAsString, appendMode, remove);
    case cmProperty::TEST:
      return HandleTestMode(status, names, propertyName, propertyValue,
                            appendAsString, appendMode, remove);
    case cmProperty::CACHE:
      return HandleCacheMode(status, names, propertyName, propertyValue,
                             appendAsString, appendMode, remove);
    case cmProperty::INSTALL:
      return HandleInstallMode(status, names, propertyName, propertyValue,
                               appendAsString, appendMode, remove);

    case cmProperty::VARIABLE:
    case cmProperty::CACHED_VARIABLE:
      break; // should never happen
  }
  return true;
}

namespace {
bool HandleGlobalMode(cmExecutionStatus& status,
                      const std::set<std::string>& names,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove)
{
  if (!names.empty()) {
    status.SetError("given names for GLOBAL scope.");
    return false;
  }

  // Set or append the property.
  cmake* cm = status.GetMakefile().GetCMakeInstance();
  if (appendMode) {
    cm->AppendProperty(propertyName, propertyValue, appendAsString);
  } else {
    if (remove) {
      cm->SetProperty(propertyName, nullptr);
    } else {
      cm->SetProperty(propertyName, propertyValue.c_str());
    }
  }

  return true;
}

bool HandleDirectoryMode(cmExecutionStatus& status,
                         const std::set<std::string>& names,
                         const std::string& propertyName,
                         const std::string& propertyValue, bool appendAsString,
                         bool appendMode, bool remove)
{
  if (names.size() > 1) {
    status.SetError("allows at most one name for DIRECTORY scope.");
    return false;
  }

  // Default to the current directory.
  cmMakefile* mf = &status.GetMakefile();

  // Lookup the directory if given.
  if (!names.empty()) {
    // Construct the directory name.  Interpret relative paths with
    // respect to the current directory.
    std::string dir = *names.begin();
    if (!cmSystemTools::FileIsFullPath(dir)) {
      dir = cmStrCat(status.GetMakefile().GetCurrentSourceDirectory(), '/',
                     *names.begin());
    }

    // The local generators are associated with collapsed paths.
    dir = cmSystemTools::CollapseFullPath(dir);

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

  // Set or append the property.
  if (appendMode) {
    mf->AppendProperty(propertyName, propertyValue, appendAsString);
  } else {
    if (remove) {
      mf->SetProperty(propertyName, nullptr);
    } else {
      mf->SetProperty(propertyName, propertyValue.c_str());
    }
  }

  return true;
}

bool HandleTargetMode(cmExecutionStatus& status,
                      const std::set<std::string>& names,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove)
{
  for (std::string const& name : names) {
    if (status.GetMakefile().IsAlias(name)) {
      status.SetError("can not be used on an ALIAS target.");
      return false;
    }
    if (cmTarget* target = status.GetMakefile().FindTargetToUse(name)) {
      // Handle the current target.
      if (!HandleTarget(target, status.GetMakefile(), propertyName,
                        propertyValue, appendAsString, appendMode, remove)) {
        return false;
      }
    } else {
      status.SetError(cmStrCat("could not find TARGET ", name,
                               ".  Perhaps it has not yet been created."));
      return false;
    }
  }
  return true;
}

bool HandleTarget(cmTarget* target, cmMakefile& makefile,
                  const std::string& propertyName,
                  const std::string& propertyValue, bool appendAsString,
                  bool appendMode, bool remove)
{
  // Set or append the property.
  if (appendMode) {
    target->AppendProperty(propertyName, propertyValue, appendAsString);
  } else {
    if (remove) {
      target->SetProperty(propertyName, nullptr);
    } else {
      target->SetProperty(propertyName, propertyValue.c_str());
    }
  }

  // Check the resulting value.
  target->CheckProperty(propertyName, &makefile);

  return true;
}

bool HandleSourceMode(cmExecutionStatus& status,
                      const std::set<std::string>& names,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove)
{
  for (std::string const& name : names) {
    // Get the source file.
    if (cmSourceFile* sf = status.GetMakefile().GetOrCreateSource(name)) {
      if (!HandleSource(sf, propertyName, propertyValue, appendAsString,
                        appendMode, remove)) {
        return false;
      }
    } else {
      status.SetError(cmStrCat(
        "given SOURCE name that could not be found or created: ", name));
      return false;
    }
  }
  return true;
}

bool HandleSource(cmSourceFile* sf, const std::string& propertyName,
                  const std::string& propertyValue, bool appendAsString,
                  bool appendMode, bool remove)
{
  // Set or append the property.
  if (appendMode) {
    sf->AppendProperty(propertyName, propertyValue, appendAsString);
  } else {
    if (remove) {
      sf->SetProperty(propertyName, nullptr);
    } else {
      sf->SetProperty(propertyName, propertyValue.c_str());
    }
  }
  return true;
}

bool HandleTestMode(cmExecutionStatus& status, std::set<std::string>& names,
                    const std::string& propertyName,
                    const std::string& propertyValue, bool appendAsString,
                    bool appendMode, bool remove)
{
  // Look for tests with all names given.
  std::set<std::string>::iterator next;
  for (auto ni = names.begin(); ni != names.end(); ni = next) {
    next = ni;
    ++next;
    if (cmTest* test = status.GetMakefile().GetTest(*ni)) {
      if (HandleTest(test, propertyName, propertyValue, appendAsString,
                     appendMode, remove)) {
        names.erase(ni);
      } else {
        return false;
      }
    }
  }

  // Names that are still left were not found.
  if (!names.empty()) {
    std::ostringstream e;
    e << "given TEST names that do not exist:\n";
    for (std::string const& name : names) {
      e << "  " << name << "\n";
    }
    status.SetError(e.str());
    return false;
  }
  return true;
}

bool HandleTest(cmTest* test, const std::string& propertyName,
                const std::string& propertyValue, bool appendAsString,
                bool appendMode, bool remove)
{
  // Set or append the property.
  if (appendMode) {
    test->AppendProperty(propertyName, propertyValue, appendAsString);
  } else {
    if (remove) {
      test->SetProperty(propertyName, nullptr);
    } else {
      test->SetProperty(propertyName, propertyValue.c_str());
    }
  }

  return true;
}

bool HandleCacheMode(cmExecutionStatus& status,
                     const std::set<std::string>& names,
                     const std::string& propertyName,
                     const std::string& propertyValue, bool appendAsString,
                     bool appendMode, bool remove)
{
  if (propertyName == "ADVANCED") {
    if (!remove && !cmIsOn(propertyValue) && !cmIsOff(propertyValue)) {
      status.SetError(cmStrCat("given non-boolean value \"", propertyValue,
                               R"(" for CACHE property "ADVANCED".  )"));
      return false;
    }
  } else if (propertyName == "TYPE") {
    if (!cmState::IsCacheEntryType(propertyValue)) {
      status.SetError(
        cmStrCat("given invalid CACHE entry TYPE \"", propertyValue, "\""));
      return false;
    }
  } else if (propertyName != "HELPSTRING" && propertyName != "STRINGS" &&
             propertyName != "VALUE") {
    status.SetError(
      cmStrCat("given invalid CACHE property ", propertyName,
               ".  "
               "Settable CACHE properties are: "
               "ADVANCED, HELPSTRING, STRINGS, TYPE, and VALUE."));
    return false;
  }

  for (std::string const& name : names) {
    // Get the source file.
    cmake* cm = status.GetMakefile().GetCMakeInstance();
    const char* existingValue = cm->GetState()->GetCacheEntryValue(name);
    if (existingValue) {
      if (!HandleCacheEntry(name, status.GetMakefile(), propertyName,
                            propertyValue, appendAsString, appendMode,
                            remove)) {
        return false;
      }
    } else {
      status.SetError(cmStrCat("could not find CACHE variable ", name,
                               ".  Perhaps it has not yet been created."));
      return false;
    }
  }
  return true;
}

bool HandleCacheEntry(std::string const& cacheKey, const cmMakefile& makefile,
                      const std::string& propertyName,
                      const std::string& propertyValue, bool appendAsString,
                      bool appendMode, bool remove)
{
  // Set or append the property.
  const char* value = propertyValue.c_str();
  cmState* state = makefile.GetState();
  if (remove) {
    state->RemoveCacheEntryProperty(cacheKey, propertyName);
  }
  if (appendMode) {
    state->AppendCacheEntryProperty(cacheKey, propertyName, value,
                                    appendAsString);
  } else {
    state->SetCacheEntryProperty(cacheKey, propertyName, value);
  }

  return true;
}

bool HandleInstallMode(cmExecutionStatus& status,
                       const std::set<std::string>& names,
                       const std::string& propertyName,
                       const std::string& propertyValue, bool appendAsString,
                       bool appendMode, bool remove)
{
  cmake* cm = status.GetMakefile().GetCMakeInstance();

  for (std::string const& name : names) {
    if (cmInstalledFile* file =
          cm->GetOrCreateInstalledFile(&status.GetMakefile(), name)) {
      if (!HandleInstall(file, status.GetMakefile(), propertyName,
                         propertyValue, appendAsString, appendMode, remove)) {
        return false;
      }
    } else {
      status.SetError(cmStrCat(
        "given INSTALL name that could not be found or created: ", name));
      return false;
    }
  }
  return true;
}

bool HandleInstall(cmInstalledFile* file, cmMakefile& makefile,
                   const std::string& propertyName,
                   const std::string& propertyValue, bool appendAsString,
                   bool appendMode, bool remove)
{
  // Set or append the property.
  const char* value = propertyValue.c_str();
  if (remove) {
    file->RemoveProperty(propertyName);
  } else if (appendMode) {
    file->AppendProperty(&makefile, propertyName, value, appendAsString);
  } else {
    file->SetProperty(&makefile, propertyName, value);
  }
  return true;
}
}
