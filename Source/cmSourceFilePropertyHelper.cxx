/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSourceFilePropertyHelper.h"

#include <cm/string_view>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmSetPropertyCommand.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmValue.h"

static bool GetSourceFilePropertyGENERATED(std::string const& name,
                                           cmMakefile& mf,
                                           cmValue& propertyValue)
{
  // Globally set as generated?
  // Note: If the given "name" only contains a filename or a relative path
  //       the file's location is ambiguous. In general, one would expect
  //       it in the source-directory, because that is where source files
  //       are located normally. However, generated files are normally
  //       generated in the build-directory. Therefore, we first check for
  //       a generated file in the build-directory before we check for a
  //       generated file in the source-directory.
  static std::string const sOne = "1";
  static std::string const sZero = "0";
  {
    auto file =
      cmSystemTools::CollapseFullPath(name, mf.GetCurrentBinaryDirectory());
    if (mf.GetGlobalGenerator()->IsGeneratedFile(file)) {
      propertyValue = cmValue(sOne);
      return true;
    }
  }
  {
    auto file =
      cmSystemTools::CollapseFullPath(name, mf.GetCurrentSourceDirectory());
    if (mf.GetGlobalGenerator()->IsGeneratedFile(file)) {
      propertyValue = cmValue(sOne);
      return true;
    }
  }
  propertyValue = cmValue(sZero);
  return true;
}

cmGetSourceFilePropertyResult cmGetSourceFileProperty(
  std::string const& sourceName, std::string const& propertyName,
  cmExecutionStatus& status, bool sourceFileDirectoryOptionEnabled,
  bool sourceFileTargetOptionEnabled,
  std::vector<std::string>& sourceFileDirectories,
  std::vector<std::string>& sourceFileTargetDirectories,
  cmValue& propertyValue, bool alwaysCreateSource)
{
  std::vector<cmMakefile*> sourceFileDirectoryMakefiles;
  if (!SetPropertyCommand::HandleAndValidateSourceFileDirectoryScopes(
        status, sourceFileDirectoryOptionEnabled,
        sourceFileTargetOptionEnabled, sourceFileDirectories,
        sourceFileTargetDirectories, sourceFileDirectoryMakefiles)) {
    return cmGetSourceFilePropertyResult::ScopeError;
  }

  bool const sourceFilePathsShouldBeAbsolute =
    sourceFileDirectoryOptionEnabled || sourceFileTargetOptionEnabled;
  cmMakefile& directoryMakefile = *sourceFileDirectoryMakefiles[0];

  // Special handling for GENERATED property.
  // Note: Only if CMP0163 is set to NEW.
  if (propertyName == "GENERATED"_s) {
    auto cmp0163 = directoryMakefile.GetPolicyStatus(cmPolicies::CMP0163);
    bool const cmp0163new =
      cmp0163 != cmPolicies::OLD && cmp0163 != cmPolicies::WARN;
    if (cmp0163new) {
      if (!GetSourceFilePropertyGENERATED(sourceName, status.GetMakefile(),
                                          propertyValue)) {
        return cmGetSourceFilePropertyResult::Error;
      }
      return cmGetSourceFilePropertyResult::Success;
    }
  }

  std::string const absolutePath =
    SetPropertyCommand::MakeSourceFilePathAbsoluteIfNeeded(
      status, sourceName, sourceFilePathsShouldBeAbsolute);

  cmSourceFile* sf = nullptr;
  if (alwaysCreateSource || propertyName == "LOCATION"_s) {
    sf = directoryMakefile.GetOrCreateSource(absolutePath);
    if (!sf) {
      return cmGetSourceFilePropertyResult::Error;
    }
  } else {
    sf = directoryMakefile.GetSource(absolutePath);
    if (!sf) {
      return cmGetSourceFilePropertyResult::SourceNotFound;
    }
  }

  propertyValue = sf->GetPropertyForUser(propertyName);
  return cmGetSourceFilePropertyResult::Success;
}
