/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGetPropertyCommand.h"

#include "cmDirectoryPropertyHelper.h"
#include "cmExecutionStatus.h"
#include "cmFileSet.h"
#include "cmInstalledFile.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmPropertyDefinition.h"
#include "cmSetPropertyCommand.h"
#include "cmSourceFilePropertyHelper.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetPropertyHelper.h"
#include "cmTestPropertyHelper.h"
#include "cmValue.h"
#include "cmake.h"

namespace {
enum OutType
{
  OutValue,
  OutDefined,
  OutBriefDoc,
  OutFullDoc,
  OutSet
};

// Implementation of each property type.
bool HandleGlobalMode(cmExecutionStatus& status, std::string const& name,
                      OutType infoType, std::string const& variable,
                      std::string const& propertyName);
bool HandleDirectoryMode(cmExecutionStatus& status, std::string const& name,
                         OutType infoType, std::string const& variable,
                         std::string const& propertyName);
bool HandleTargetMode(cmExecutionStatus& status, std::string const& name,
                      OutType infoType, std::string const& variable,
                      std::string const& propertyName);
bool HandleFileSetMode(cmExecutionStatus& status, std::string const& name,
                       OutType infoType, std::string const& variable,
                       std::string const& propertyName, cmTarget* target);
bool HandleSourceMode(cmExecutionStatus& status, std::string const& name,
                      OutType infoType, std::string const& variable,
                      std::string const& propertyName,
                      bool sourceFileDirectoryOptionEnabled,
                      bool sourceFileTargetOptionEnabled,
                      std::vector<std::string>& sourceFileDirectories,
                      std::vector<std::string>& sourceFileTargetDirectories);
bool HandleTestMode(cmExecutionStatus& status, std::string const& name,
                    OutType infoType, std::string const& variable,
                    std::string const& propertyName,
                    bool testDirectoryOptionEnabled,
                    std::string& testDirectory);
bool HandleVariableMode(cmExecutionStatus& status, std::string const& name,
                        OutType infoType, std::string const& variable,
                        std::string const& propertyName);
bool HandleCacheMode(cmExecutionStatus& status, std::string const& name,
                     OutType infoType, std::string const& variable,
                     std::string const& propertyName);
bool HandleInstallMode(cmExecutionStatus& status, std::string const& name,
                       OutType infoType, std::string const& variable,
                       std::string const& propertyName);
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
  std::string const& variable = args[0];

  std::string name;
  std::string propertyName;

  std::string file_set_target_name;
  bool file_set_target_option_enabled = false;

  std::vector<std::string> source_file_directories;
  std::vector<std::string> source_file_target_directories;
  bool source_file_directory_option_enabled = false;
  bool source_file_target_option_enabled = false;

  std::string test_directory;
  bool test_directory_option_enabled = false;

  // Get the scope from which to get the property.
  cmProperty::ScopeType scope;
  if (args[1] == "GLOBAL") {
    scope = cmProperty::GLOBAL;
  } else if (args[1] == "DIRECTORY") {
    scope = cmProperty::DIRECTORY;
  } else if (args[1] == "TARGET") {
    scope = cmProperty::TARGET;
  } else if (args[1] == "FILE_SET") {
    scope = cmProperty::FILE_SET;
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
    status.SetError(cmStrCat("given invalid scope ", args[1],
                             ".  "
                             "Valid scopes are "
                             "GLOBAL, DIRECTORY, TARGET, FILE_SET, SOURCE, "
                             "TEST, VARIABLE, CACHE, INSTALL."));
    return false;
  }

  // Parse remaining arguments.
  enum Doing
  {
    DoingNone,
    DoingName,
    DoingProperty,
    DoingType,
    DoingFileSetTarget,
    DoingSourceDirectory,
    DoingSourceTargetDirectory,
    DoingTestDirectory,
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
    } else if (doing == DoingNone && scope == cmProperty::FILE_SET &&
               args[i] == "TARGET") {
      doing = DoingFileSetTarget;
      file_set_target_option_enabled = true;
    } else if (doing == DoingNone && scope == cmProperty::SOURCE_FILE &&
               args[i] == "DIRECTORY") {
      doing = DoingSourceDirectory;
      source_file_directory_option_enabled = true;
    } else if (doing == DoingNone && scope == cmProperty::SOURCE_FILE &&
               args[i] == "TARGET_DIRECTORY") {
      doing = DoingSourceTargetDirectory;
      source_file_target_option_enabled = true;
    } else if (doing == DoingNone && scope == cmProperty::TEST &&
               args[i] == "DIRECTORY") {
      doing = DoingTestDirectory;
      test_directory_option_enabled = true;
    } else if (doing == DoingFileSetTarget) {
      file_set_target_name = args[i];
      doing = DoingNone;
    } else if (doing == DoingSourceDirectory) {
      source_file_directories.push_back(args[i]);
      doing = DoingNone;
    } else if (doing == DoingSourceTargetDirectory) {
      source_file_target_directories.push_back(args[i]);
      doing = DoingNone;
    } else if (doing == DoingTestDirectory) {
      test_directory = args[i];
      doing = DoingNone;
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
    }
    if (output.empty()) {
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
    }
    if (output.empty()) {
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
      case cmProperty::FILE_SET: {
        cmTarget* file_set_target;
        if (!SetPropertyCommand::HandleAndValidateFileSetTargetScopes(
              status, file_set_target_option_enabled, file_set_target_name,
              file_set_target)) {
          return false;
        }
        return HandleFileSetMode(status, name, infoType, variable,
                                 propertyName, file_set_target);
      }
      case cmProperty::SOURCE_FILE:
        return HandleSourceMode(status, name, infoType, variable, propertyName,
                                source_file_directory_option_enabled,
                                source_file_target_option_enabled,
                                source_file_directories,
                                source_file_target_directories);
      case cmProperty::TEST:
        return HandleTestMode(status, name, infoType, variable, propertyName,
                              test_directory_option_enabled, test_directory);
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

// Implementation of result storage.
template <typename ValueType>
bool StoreResult(OutType infoType, cmMakefile& makefile,
                 std::string const& variable, ValueType value)
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

bool HandleGlobalMode(cmExecutionStatus& status, std::string const& name,
                      OutType infoType, std::string const& variable,
                      std::string const& propertyName)
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

bool HandleDirectoryMode(cmExecutionStatus& status, std::string const& name,
                         OutType infoType, std::string const& variable,
                         std::string const& propertyName)
{
  cmValue prop;
  if (!GetPropertyCommand::LookupDirectoryProperty(status, name, propertyName,
                                                   prop)) {
    return false;
  }
  return StoreResult(infoType, status.GetMakefile(), variable, prop);
}

bool HandleTargetMode(cmExecutionStatus& status, std::string const& name,
                      OutType infoType, std::string const& variable,
                      std::string const& propertyName)
{
  cmValue prop;
  if (!GetPropertyCommand::LookupTargetProperty(status, name, propertyName,
                                                prop)) {
    return false;
  }
  return StoreResult(infoType, status.GetMakefile(), variable, prop);
}

bool HandleFileSetMode(cmExecutionStatus& status, std::string const& name,
                       OutType infoType, std::string const& variable,
                       std::string const& propertyName, cmTarget* target)
{
  if (name.empty()) {
    status.SetError("not given name for FILE_SET scope.");
    return false;
  }

  if (cmFileSet* fileSet = target->GetFileSet(name)) {
    cmValue prop = fileSet->GetProperty(propertyName);
    return StoreResult(infoType, status.GetMakefile(), variable, prop);
  }
  status.SetError(cmStrCat("could not find FILE_SET ", name, " for TARGET ",
                           target->GetName(),
                           ".  Perhaps it has not yet been created."));
  return false;
}

bool HandleSourceMode(cmExecutionStatus& status, std::string const& name,
                      OutType infoType, std::string const& variable,
                      std::string const& propertyName,
                      bool sourceFileDirectoryOptionEnabled,
                      bool sourceFileTargetOptionEnabled,
                      std::vector<std::string>& sourceFileDirectories,
                      std::vector<std::string>& sourceFileTargetDirectories)
{
  cmValue prop;
  if (!GetPropertyCommand::LookupSourceProperty(
        status, name, propertyName, sourceFileDirectoryOptionEnabled,
        sourceFileTargetOptionEnabled, sourceFileDirectories,
        sourceFileTargetDirectories, prop)) {
    return false;
  }
  return StoreResult(infoType, status.GetMakefile(), variable, prop);
}

bool HandleTestMode(cmExecutionStatus& status, std::string const& name,
                    OutType infoType, std::string const& variable,
                    std::string const& propertyName,
                    bool testDirectoryOptionEnabled,
                    std::string& testDirectory)
{
  cmValue prop;
  if (!GetPropertyCommand::LookupTestProperty(status, name, propertyName,
                                              testDirectoryOptionEnabled,
                                              testDirectory, prop)) {
    return false;
  }
  return StoreResult(infoType, status.GetMakefile(), variable, prop);
}

bool HandleVariableMode(cmExecutionStatus& status, std::string const& name,
                        OutType infoType, std::string const& variable,
                        std::string const& propertyName)
{
  if (!name.empty()) {
    status.SetError("given name for VARIABLE scope.");
    return false;
  }

  return StoreResult(infoType, status.GetMakefile(), variable,
                     status.GetMakefile().GetDefinition(propertyName));
}

bool HandleCacheMode(cmExecutionStatus& status, std::string const& name,
                     OutType infoType, std::string const& variable,
                     std::string const& propertyName)
{
  cmValue value;
  if (!GetPropertyCommand::LookupCacheProperty(status, name, propertyName,
                                               value)) {
    return false;
  }
  StoreResult(infoType, status.GetMakefile(), variable, value);
  return true;
}

bool HandleInstallMode(cmExecutionStatus& status, std::string const& name,
                       OutType infoType, std::string const& variable,
                       std::string const& propertyName)
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

namespace GetPropertyCommand {

bool LookupTargetProperty(cmExecutionStatus& status, std::string const& name,
                          std::string const& propertyName, cmValue& out)
{
  if (name.empty()) {
    status.SetError("not given name for TARGET scope.");
    return false;
  }

  auto result =
    cmGetTargetProperty(name, propertyName, status.GetMakefile(), out);
  if (result == cmGetTargetPropertyResult::Success) {
    return true;
  }
  if (result == cmGetTargetPropertyResult::TargetNotFound) {
    status.SetError(cmStrCat("could not find TARGET ", name,
                             ".  Perhaps it has not yet been created."));
  }
  return false;
}

bool LookupDirectoryProperty(cmExecutionStatus& status,
                             std::string const& name,
                             std::string const& propertyName, cmValue& out)
{
  auto result =
    cmGetDirectoryProperty(name, propertyName, status.GetMakefile(), out);
  if (result == cmGetDirectoryPropertyResult::Success) {
    return true;
  }
  if (result == cmGetDirectoryPropertyResult::DirectoryNotFound) {
    status.SetError(
      "DIRECTORY scope provided but requested directory was not found. "
      "This could be because the directory argument was invalid or, "
      "it is valid but has not been processed yet.");
  }
  return false;
}

bool LookupSourceProperty(
  cmExecutionStatus& status, std::string const& name,
  std::string const& propertyName, bool sourceFileDirectoryOptionEnabled,
  bool sourceFileTargetOptionEnabled,
  std::vector<std::string>& sourceFileDirectories,
  std::vector<std::string>& sourceFileTargetDirectories, cmValue& out)
{
  if (name.empty()) {
    status.SetError("not given name for SOURCE scope.");
    return false;
  }

  auto result = cmGetSourceFileProperty(
    name, propertyName, status, sourceFileDirectoryOptionEnabled,
    sourceFileTargetOptionEnabled, sourceFileDirectories,
    sourceFileTargetDirectories, out, /*alwaysCreateSource=*/true);
  if (result == cmGetSourceFilePropertyResult::Success) {
    return true;
  }
  if (result == cmGetSourceFilePropertyResult::Error ||
      result == cmGetSourceFilePropertyResult::SourceNotFound) {
    status.SetError(cmStrCat(
      "given SOURCE name that could not be found or created: ", name));
  }
  return false;
}

bool LookupSourceProperty(cmExecutionStatus& status, std::string const& name,
                          std::string const& propertyName, cmValue& out)
{
  std::vector<std::string> noDirectories;
  std::vector<std::string> noTargetDirectories;
  return LookupSourceProperty(status, name, propertyName,
                              /*sourceFileDirectoryOptionEnabled=*/false,
                              /*sourceFileTargetOptionEnabled=*/false,
                              noDirectories, noTargetDirectories, out);
}

bool LookupTestProperty(cmExecutionStatus& status, std::string const& name,
                        std::string const& propertyName,
                        bool testDirectoryOptionEnabled,
                        std::string& testDirectory, cmValue& out)
{
  if (name.empty()) {
    status.SetError("not given name for TEST scope.");
    return false;
  }

  auto result =
    cmGetTestProperty(name, propertyName, status, testDirectoryOptionEnabled,
                      testDirectory, out);
  if (result == cmGetTestPropertyResult::Success) {
    return true;
  }
  if (result == cmGetTestPropertyResult::TestNotFound) {
    status.SetError(cmStrCat("given TEST name that does not exist: ", name));
  }
  return false;
}

bool LookupTestProperty(cmExecutionStatus& status, std::string const& name,
                        std::string const& propertyName, cmValue& out)
{
  std::string noDirectory;
  return LookupTestProperty(status, name, propertyName,
                            /*testDirectoryOptionEnabled=*/false, noDirectory,
                            out);
}

bool LookupCacheProperty(cmExecutionStatus& status, std::string const& name,
                         std::string const& propertyName, cmValue& out)
{
  if (name.empty()) {
    status.SetError("not given name for CACHE scope.");
    return false;
  }

  out = nullptr;
  if (status.GetMakefile().GetState()->GetCacheEntryValue(name)) {
    out = status.GetMakefile().GetState()->GetCacheEntryProperty(name,
                                                                 propertyName);
  }
  return true;
}

}
