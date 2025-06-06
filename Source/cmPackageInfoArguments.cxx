/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPackageInfoArguments.h"

#include <utility>

#include <cm/string_view>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

template void cmPackageInfoArguments::Bind<void>(cmArgumentParser<void>&,
                                                 cmPackageInfoArguments*);

bool cmPackageInfoArguments::Check(cmExecutionStatus& status,
                                   bool enable) const
{
  if (!enable) {
    // Check if any options were given.
    if (this->LowerCase) {
      status.SetError("LOWER_CASE_FILE requires PACKAGE_INFO.");
      return false;
    }
    if (!this->Appendix.empty()) {
      status.SetError("APPENDIX requires PACKAGE_INFO.");
      return false;
    }
    if (!this->Version.empty()) {
      status.SetError("VERSION requires PACKAGE_INFO.");
      return false;
    }
    if (!this->DefaultTargets.empty()) {
      status.SetError("DEFAULT_TARGETS requires PACKAGE_INFO.");
      return false;
    }
    if (!this->DefaultConfigs.empty()) {
      status.SetError("DEFAULT_CONFIGURATIONS requires PACKAGE_INFO.");
      return false;
    }
    if (!this->ProjectName.empty()) {
      status.SetError("PROJECT requires PACKAGE_INFO.");
      return false;
    }
    if (this->NoProjectDefaults) {
      status.SetError("NO_PROJECT_METADATA requires PACKAGE_INFO.");
      return false;
    }
  }

  // Check for incompatible options.
  if (!this->Appendix.empty()) {
    if (!this->Version.empty()) {
      status.SetError("APPENDIX and VERSION are mutually exclusive.");
      return false;
    }
    if (!this->DefaultTargets.empty()) {
      status.SetError("APPENDIX and DEFAULT_TARGETS "
                      "are mutually exclusive.");
      return false;
    }
    if (!this->DefaultConfigs.empty()) {
      status.SetError("APPENDIX and DEFAULT_CONFIGURATIONS "
                      "are mutually exclusive.");
      return false;
    }
    if (!this->ProjectName.empty()) {
      status.SetError("APPENDIX and PROJECT are mutually exclusive.");
      return false;
    }
  }
  if (this->NoProjectDefaults) {
    if (!this->ProjectName.empty()) {
      status.SetError("PROJECT and NO_PROJECT_METADATA "
                      "are mutually exclusive.");
      return false;
    }
  }

  // Check for options that require other options.
  if (this->Version.empty()) {
    if (!this->VersionCompat.empty()) {
      status.SetError("COMPAT_VERSION requires VERSION.");
      return false;
    }
    if (!this->VersionSchema.empty()) {
      status.SetError("VERSION_SCHEMA requires VERSION.");
      return false;
    }
  }

  // Validate the package name.
  if (!this->PackageName.empty()) {
    if (!cmGeneratorExpression::IsValidTargetName(this->PackageName) ||
        this->PackageName.find(':') != std::string::npos) {
      status.SetError(
        cmStrCat(R"(PACKAGE_INFO given invalid package name ")"_s,
                 this->PackageName, R"(".)"_s));
      return false;
    }
  }

  return true;
}

bool cmPackageInfoArguments::SetMetadataFromProject(cmExecutionStatus& status)
{
  // Determine what project to use for inherited metadata.
  if (!this->SetEffectiveProject(status)) {
    return false;
  }

  if (this->ProjectName.empty()) {
    // We are not inheriting from a project.
    return true;
  }

  cmMakefile& mf = status.GetMakefile();
  auto mapProjectValue = [&](std::string& arg, cm::string_view suffix) {
    cmValue const& projectValue =
      mf.GetDefinition(cmStrCat(this->ProjectName, '_', suffix));
    if (projectValue) {
      arg = *projectValue;
      return true;
    }
    return false;
  };

  if (this->Version.empty()) {
    if (mapProjectValue(this->Version, "VERSION"_s)) {
      mapProjectValue(this->VersionCompat, "COMPAT_VERSION"_s);
    }
  }

  if (this->Description.empty()) {
    mapProjectValue(this->Description, "DESCRIPTION"_s);
  }

  if (this->Website.empty()) {
    mapProjectValue(this->Website, "HOMEPAGE_URL"_s);
  }

  return true;
}

bool cmPackageInfoArguments::SetEffectiveProject(cmExecutionStatus& status)
{
  if (!this->Appendix.empty()) {
    // Appendices are not allowed to specify package metadata.
    return true;
  }

  if (this->NoProjectDefaults) {
    // User requested that metadata not be inherited.
    return true;
  }

  cmMakefile& mf = status.GetMakefile();
  if (!this->ProjectName.empty()) {
    // User specified a project; make sure it exists.
    if (!mf.GetStateSnapshot().CheckProjectName(this->ProjectName)) {
      status.SetError(cmStrCat(R"(PROJECT given invalid project name ")"_s,
                               this->ProjectName, R"(".)"_s));
      return false;
    }
  } else {
    // No project was specified; check if the package name is also a project.
    std::string project = mf.GetStateSnapshot().GetProjectName();
    if (this->PackageName == project) {
      this->ProjectName = std::move(project);
    }
  }

  return true;
}

std::string cmPackageInfoArguments::GetNamespace() const
{
  return cmStrCat(this->PackageName, "::"_s);
}

std::string cmPackageInfoArguments::GetPackageDirName() const
{
  if (this->LowerCase) {
    return cmSystemTools::LowerCase(this->PackageName);
  }
  return this->PackageName;
}

std::string cmPackageInfoArguments::GetPackageFileName() const
{
  std::string const pkgNameOnDisk = this->GetPackageDirName();
  if (!this->Appendix.empty()) {
    return cmStrCat(pkgNameOnDisk, '-', this->Appendix, ".cps"_s);
  }
  return cmStrCat(pkgNameOnDisk, ".cps"_s);
}
