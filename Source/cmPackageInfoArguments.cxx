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

namespace {

bool ArgWasSpecified(bool value)
{
  return value;
}

bool ArgWasSpecified(std::string const& value)
{
  return !value.empty();
}

bool ArgWasSpecified(std::vector<std::string> const& value)
{
  return !value.empty();
}

} // anonymous namespace

#define ENFORCE_REQUIRES(req, value, arg)                                     \
  do {                                                                        \
    if (ArgWasSpecified(value)) {                                             \
      status.SetError(arg " requires " req ".");                              \
      return false;                                                           \
    }                                                                         \
  } while (false)

#define ENFORCE_EXCLUSIVE(arg1, value, arg2)                                  \
  do {                                                                        \
    if (ArgWasSpecified(value)) {                                             \
      status.SetError(arg1 " and " arg2 " are mutually exclusive.");          \
      return false;                                                           \
    }                                                                         \
  } while (false)

bool cmPackageInfoArguments::Check(cmExecutionStatus& status,
                                   bool enable) const
{
  if (!enable) {
    // Check if any options were given.
    ENFORCE_REQUIRES("PACKAGE_INFO", this->LowerCase, "LOWER_CASE_FILE");
    ENFORCE_REQUIRES("PACKAGE_INFO", this->Appendix, "APPENDIX");
    ENFORCE_REQUIRES("PACKAGE_INFO", this->Version, "VERSION");
    ENFORCE_REQUIRES("PACKAGE_INFO", this->DefaultTargets, "DEFAULT_TARGETS");
    ENFORCE_REQUIRES("PACKAGE_INFO", this->DefaultConfigs,
                     "DEFAULT_CONFIGURATIONS");
    ENFORCE_REQUIRES("PACKAGE_INFO", this->ProjectName, "PROJECT");
    ENFORCE_REQUIRES("PACKAGE_INFO", this->NoProjectDefaults,
                     "NO_PROJECT_METADATA");
  }

  // Check for incompatible options.
  if (!this->Appendix.empty()) {
    ENFORCE_EXCLUSIVE("APPENDIX", this->Version, "VERSION");
    ENFORCE_EXCLUSIVE("APPENDIX", this->DefaultTargets, "DEFAULT_TARGETS");
    ENFORCE_EXCLUSIVE("APPENDIX", this->DefaultConfigs,
                      "DEFAULT_CONFIGURATIONS");
    ENFORCE_EXCLUSIVE("APPENDIX", this->ProjectName, "PROJECT");
  }
  if (this->NoProjectDefaults) {
    ENFORCE_EXCLUSIVE("PROJECT", this->ProjectName, "NO_PROJECT_METADATA");
  }

  // Check for options that require other options.
  if (this->Version.empty()) {
    ENFORCE_REQUIRES("VERSION", this->VersionCompat, "COMPAT_VERSION");
    ENFORCE_REQUIRES("VERSION", this->VersionSchema, "VERSION_SCHEMA");
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

#undef ENFORCE_REQUIRES
#undef ENFORCE_EXCLUSIVE

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
