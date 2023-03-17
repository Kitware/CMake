/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSetPropertyCommand.h"

#include <set>
#include <sstream>
#include <unordered_set>

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmInstalledFile.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmProperty.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTest.h"
#include "cmValue.h"
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
                      bool appendMode, bool remove,
                      const std::vector<cmMakefile*>& directory_makefiles,
                      bool source_file_paths_should_be_absolute);
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

namespace SetPropertyCommand {
bool HandleSourceFileDirectoryScopes(
  cmExecutionStatus& status, std::vector<std::string>& source_file_directories,
  std::vector<std::string>& source_file_target_directories,
  std::vector<cmMakefile*>& directory_makefiles)
{
  std::unordered_set<cmMakefile*> directory_makefiles_set;

  cmMakefile* current_dir_mf = &status.GetMakefile();
  if (!source_file_directories.empty()) {
    for (const std::string& dir_path : source_file_directories) {
      const std::string absolute_dir_path = cmSystemTools::CollapseFullPath(
        dir_path, current_dir_mf->GetCurrentSourceDirectory());
      cmMakefile* dir_mf =
        status.GetMakefile().GetGlobalGenerator()->FindMakefile(
          absolute_dir_path);
      if (!dir_mf) {
        status.SetError(cmStrCat("given non-existent DIRECTORY ", dir_path));
        return false;
      }
      if (directory_makefiles_set.find(dir_mf) ==
          directory_makefiles_set.end()) {
        directory_makefiles.push_back(dir_mf);
        directory_makefiles_set.insert(dir_mf);
      }
    }
  }

  if (!source_file_target_directories.empty()) {
    for (const std::string& target_name : source_file_target_directories) {
      cmTarget* target = current_dir_mf->FindTargetToUse(target_name);
      if (!target) {
        status.SetError(cmStrCat(
          "given non-existent target for TARGET_DIRECTORY ", target_name));
        return false;
      }
      cmValue target_source_dir = target->GetProperty("BINARY_DIR");
      cmMakefile* target_dir_mf =
        status.GetMakefile().GetGlobalGenerator()->FindMakefile(
          *target_source_dir);

      if (directory_makefiles_set.find(target_dir_mf) ==
          directory_makefiles_set.end()) {
        directory_makefiles.push_back(target_dir_mf);
        directory_makefiles_set.insert(target_dir_mf);
      }
    }
  }

  if (source_file_directories.empty() &&
      source_file_target_directories.empty()) {
    directory_makefiles.push_back(current_dir_mf);
  }
  return true;
}

bool HandleSourceFileDirectoryScopeValidation(
  cmExecutionStatus& status, bool source_file_directory_option_enabled,
  bool source_file_target_option_enabled,
  std::vector<std::string>& source_file_directories,
  std::vector<std::string>& source_file_target_directories)
{
  // Validate source file directory scopes.
  if (source_file_directory_option_enabled &&
      source_file_directories.empty()) {
    std::string errors = "called with incorrect number of arguments "
                         "no value provided to the DIRECTORY option";
    status.SetError(errors);
    return false;
  }
  if (source_file_target_option_enabled &&
      source_file_target_directories.empty()) {
    std::string errors = "called with incorrect number of arguments "
                         "no value provided to the TARGET_DIRECTORY option";
    status.SetError(errors);
    return false;
  }
  return true;
}

bool HandleAndValidateSourceFileDirectoryScopes(
  cmExecutionStatus& status, bool source_file_directory_option_enabled,
  bool source_file_target_option_enabled,
  std::vector<std::string>& source_file_directories,
  std::vector<std::string>& source_file_target_directories,
  std::vector<cmMakefile*>& source_file_directory_makefiles)
{
  bool scope_options_valid =
    SetPropertyCommand::HandleSourceFileDirectoryScopeValidation(
      status, source_file_directory_option_enabled,
      source_file_target_option_enabled, source_file_directories,
      source_file_target_directories);
  if (!scope_options_valid) {
    return false;
  }

  scope_options_valid = SetPropertyCommand::HandleSourceFileDirectoryScopes(
    status, source_file_directories, source_file_target_directories,
    source_file_directory_makefiles);
  return scope_options_valid;
}

std::string MakeSourceFilePathAbsoluteIfNeeded(
  cmExecutionStatus& status, const std::string& source_file_path,
  const bool needed)
{
  if (!needed) {
    return source_file_path;
  }
  std::string absolute_file_path = cmSystemTools::CollapseFullPath(
    source_file_path, status.GetMakefile().GetCurrentSourceDirectory());
  return absolute_file_path;
}

void MakeSourceFilePathsAbsoluteIfNeeded(
  cmExecutionStatus& status,
  std::vector<std::string>& source_files_absolute_paths,
  std::vector<std::string>::const_iterator files_it_begin,
  std::vector<std::string>::const_iterator files_it_end, const bool needed)
{

  // Make the file paths absolute, so that relative source file paths are
  // picked up relative to the command calling site, regardless of the
  // directory scope.
  std::vector<std::string>::difference_type num_files =
    files_it_end - files_it_begin;
  source_files_absolute_paths.reserve(num_files);

  if (!needed) {
    source_files_absolute_paths.assign(files_it_begin, files_it_end);
    return;
  }

  for (; files_it_begin != files_it_end; ++files_it_begin) {
    const std::string absolute_file_path =
      MakeSourceFilePathAbsoluteIfNeeded(status, *files_it_begin, true);
    source_files_absolute_paths.push_back(absolute_file_path);
  }
}

bool HandleAndValidateSourceFilePropertyGENERATED(
  cmSourceFile* sf, std::string const& propertyValue, PropertyOp op)
{
  const auto& mf = *sf->GetLocation().GetMakefile();
  auto policyStatus = mf.GetPolicyStatus(cmPolicies::CMP0118);

  const bool policyWARN = policyStatus == cmPolicies::WARN;
  const bool policyNEW = policyStatus != cmPolicies::OLD && !policyWARN;

  if (policyWARN) {
    if (!cmIsOn(propertyValue) && !cmIsOff(propertyValue)) {
      mf.IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0118),
                 "\nAttempt to set property 'GENERATED' with the following "
                 "non-boolean value (which will be interpreted as \"0\"):\n",
                 propertyValue,
                 "\nThat exact value will not be retrievable. A value of "
                 "\"0\" will be returned instead.\n"
                 "This will be an error under policy CMP0118.\n"));
    }
    if (cmIsOff(propertyValue)) {
      mf.IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0118),
                 "\nUnsetting property 'GENERATED' will not be allowed under "
                 "policy CMP0118!\n"));
    }
    if (op == PropertyOp::Append || op == PropertyOp::AppendAsString) {
      mf.IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0118),
                 "\nAppending to property 'GENERATED' will not be allowed "
                 "under policy CMP0118!\n"));
    }
  } else if (policyNEW) {
    if (!cmIsOn(propertyValue) && !cmIsOff(propertyValue)) {
      mf.IssueMessage(
        MessageType::AUTHOR_ERROR,
        cmStrCat(
          "Policy CMP0118 is set to NEW and the following non-boolean value "
          "given for property 'GENERATED' is therefore not allowed:\n",
          propertyValue, "\nReplace it with a boolean value!\n"));
      return true;
    }
    if (cmIsOff(propertyValue)) {
      mf.IssueMessage(
        MessageType::AUTHOR_ERROR,
        "Unsetting the 'GENERATED' property is not allowed under CMP0118!\n");
      return true;
    }
    if (op == PropertyOp::Append || op == PropertyOp::AppendAsString) {
      mf.IssueMessage(MessageType::AUTHOR_ERROR,
                      "Policy CMP0118 is set to NEW and appending to the "
                      "'GENERATED' property is therefore not allowed. Only "
                      "setting it to \"1\" is allowed!\n");
      return true;
    }
  }

  // Set property.
  if (!policyNEW) {
    // Do it the traditional way.
    switch (op) {
      case PropertyOp::Append:
        sf->AppendProperty("GENERATED", propertyValue, false);
        break;
      case PropertyOp::AppendAsString:
        sf->AppendProperty("GENERATED", propertyValue, true);
        break;
      case PropertyOp::Remove:
        sf->SetProperty("GENERATED", nullptr);
        break;
      case PropertyOp::Set:
        sf->SetProperty("GENERATED", propertyValue);
        break;
    }
  } else {
    sf->MarkAsGenerated();
  }
  return true;
}

} // END: namespace SetPropertyCommand

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

  std::vector<std::string> source_file_directories;
  std::vector<std::string> source_file_target_directories;
  bool source_file_directory_option_enabled = false;
  bool source_file_target_option_enabled = false;

  // Parse the rest of the arguments up to the values.
  enum Doing
  {
    DoingNone,
    DoingNames,
    DoingProperty,
    DoingValues,
    DoingSourceDirectory,
    DoingSourceTargetDirectory
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
    } else if (doing != DoingProperty && doing != DoingValues &&
               scope == cmProperty::SOURCE_FILE && arg == "DIRECTORY") {
      doing = DoingSourceDirectory;
      source_file_directory_option_enabled = true;
    } else if (doing != DoingProperty && doing != DoingValues &&
               scope == cmProperty::SOURCE_FILE && arg == "TARGET_DIRECTORY") {
      doing = DoingSourceTargetDirectory;
      source_file_target_option_enabled = true;
    } else if (doing == DoingNames) {
      names.insert(arg);
    } else if (doing == DoingSourceDirectory) {
      source_file_directories.push_back(arg);
    } else if (doing == DoingSourceTargetDirectory) {
      source_file_target_directories.push_back(arg);
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

  std::vector<cmMakefile*> source_file_directory_makefiles;
  bool file_scopes_handled =
    SetPropertyCommand::HandleAndValidateSourceFileDirectoryScopes(
      status, source_file_directory_option_enabled,
      source_file_target_option_enabled, source_file_directories,
      source_file_target_directories, source_file_directory_makefiles);
  if (!file_scopes_handled) {
    return false;
  }
  bool source_file_paths_should_be_absolute =
    source_file_directory_option_enabled || source_file_target_option_enabled;

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
                              appendAsString, appendMode, remove,
                              source_file_directory_makefiles,
                              source_file_paths_should_be_absolute);
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

namespace /* anonymous */ {
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
      cm->SetProperty(propertyName, propertyValue);
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
    std::string dir = cmSystemTools::CollapseFullPath(
      *names.begin(), status.GetMakefile().GetCurrentSourceDirectory());

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
      mf->SetProperty(propertyName, propertyValue);
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
    target->AppendProperty(propertyName, propertyValue,
                           makefile.GetBacktrace(), appendAsString);
  } else {
    if (remove) {
      target->SetProperty(propertyName, nullptr);
    } else {
      target->SetProperty(propertyName, propertyValue);
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
                      bool appendMode, bool remove,
                      const std::vector<cmMakefile*>& directory_makefiles,
                      const bool source_file_paths_should_be_absolute)
{
  std::vector<std::string> files_absolute;
  std::vector<std::string> unique_files(names.begin(), names.end());
  SetPropertyCommand::MakeSourceFilePathsAbsoluteIfNeeded(
    status, files_absolute, unique_files.begin(), unique_files.end(),
    source_file_paths_should_be_absolute);

  for (auto* const mf : directory_makefiles) {
    for (std::string const& name : files_absolute) {
      // Get the source file.
      if (cmSourceFile* sf = mf->GetOrCreateSource(name)) {
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
  }

  return true;
}

bool HandleSource(cmSourceFile* sf, const std::string& propertyName,
                  const std::string& propertyValue, bool appendAsString,
                  bool appendMode, bool remove)
{
  // Special validation and handling of GENERATED flag?
  if (propertyName == "GENERATED") {
    SetPropertyCommand::PropertyOp op = (remove)
      ? SetPropertyCommand::PropertyOp::Remove
      : (appendAsString) ? SetPropertyCommand::PropertyOp::AppendAsString
      : (appendMode)     ? SetPropertyCommand::PropertyOp::Append
                         : SetPropertyCommand::PropertyOp::Set;
    return SetPropertyCommand::HandleAndValidateSourceFilePropertyGENERATED(
      sf, propertyValue, op);
  }

  // Set or append the property.
  if (appendMode) {
    sf->AppendProperty(propertyName, propertyValue, appendAsString);
  } else {
    if (remove) {
      sf->SetProperty(propertyName, nullptr);
    } else {
      sf->SetProperty(propertyName, propertyValue);
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
      test->SetProperty(propertyName, propertyValue);
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
    cmValue existingValue = cm->GetState()->GetCacheEntryValue(name);
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
  cmState* state = makefile.GetState();
  if (remove) {
    state->RemoveCacheEntryProperty(cacheKey, propertyName);
  }
  if (appendMode) {
    state->AppendCacheEntryProperty(cacheKey, propertyName, propertyValue,
                                    appendAsString);
  } else {
    state->SetCacheEntryProperty(cacheKey, propertyName, propertyValue);
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
  if (remove) {
    file->RemoveProperty(propertyName);
  } else if (appendMode) {
    file->AppendProperty(&makefile, propertyName, propertyValue,
                         appendAsString);
  } else {
    file->SetProperty(&makefile, propertyName, propertyValue);
  }
  return true;
}
}
