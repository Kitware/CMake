/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPackageInfoArguments.h"

#include <cm/string_view>

#include "cmExecutionStatus.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

template void cmPackageInfoArguments::Bind<void>(cmArgumentParser<void>&,
                                                 cmPackageInfoArguments*);

cm::string_view cmPackageInfoArguments::CommandName() const
{
  return "PACKAGE_INFO"_s;
}

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

bool cmPackageInfoArguments::Check(cmExecutionStatus& status) const
{
  // Check for incompatible options.
  if (!this->Appendix.empty()) {
    ENFORCE_EXCLUSIVE("APPENDIX", this->Version, "VERSION");
    ENFORCE_EXCLUSIVE("APPENDIX", this->License, "LICENSE");
    ENFORCE_EXCLUSIVE("APPENDIX", this->Description, "DESCRIPTION");
    ENFORCE_EXCLUSIVE("APPENDIX", this->Website, "HOMEPAGE_URL");
    ENFORCE_EXCLUSIVE("APPENDIX", this->DefaultTargets, "DEFAULT_TARGETS");
    ENFORCE_EXCLUSIVE("APPENDIX", this->DefaultConfigs,
                      "DEFAULT_CONFIGURATIONS");
    ENFORCE_EXCLUSIVE("APPENDIX", this->ProjectName, "PROJECT");
  }

  // Check for options that require other options.
  if (this->Version.empty()) {
    ENFORCE_REQUIRES("VERSION", this->VersionCompat, "COMPAT_VERSION");
    ENFORCE_REQUIRES("VERSION", this->VersionSchema, "VERSION_SCHEMA");
  }

  return cmProjectInfoArguments::Check(status);
}

#undef ENFORCE_REQUIRES
#undef ENFORCE_EXCLUSIVE

bool cmPackageInfoArguments::SetEffectiveProject(cmExecutionStatus& status)
{
  if (!this->Appendix.empty()) {
    // Appendices are not allowed to specify package metadata.
    return true;
  }

  return cmProjectInfoArguments::SetEffectiveProject(status);
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
