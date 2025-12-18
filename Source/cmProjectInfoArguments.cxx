/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmProjectInfoArguments.h"

#include <utility>

#include <cm/string_view>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

template void cmProjectInfoArguments::Bind<void>(cmArgumentParser<void>&,
                                                 cmProjectInfoArguments*);

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

cmProjectInfoArguments::cmProjectInfoArguments() = default;

bool cmProjectInfoArguments::Check(cmExecutionStatus& status) const
{
  // Check for incompatible options.
  if (this->NoProjectDefaults) {
    ENFORCE_EXCLUSIVE("PROJECT", this->ProjectName, "NO_PROJECT_METADATA");
  }

  // Validate the package name.
  if (!this->PackageName.empty()) {
    if (!cmGeneratorExpression::IsValidTargetName(this->PackageName) ||
        this->PackageName.find(':') != std::string::npos) {
      status.SetError(cmStrCat(this->CommandName(),
                               " given invalid package name \""_s,
                               this->PackageName, "\"."_s));
      return false;
    }
  }

  return true;
}

#undef ENFORCE_REQUIRES
#undef ENFORCE_EXCLUSIVE

bool cmProjectInfoArguments::SetMetadataFromProject(cmExecutionStatus& status)
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

  if (this->License.empty()) {
    mapProjectValue(this->License, "SPDX_LICENSE"_s);
  }

  if (this->Description.empty()) {
    mapProjectValue(this->Description, "DESCRIPTION"_s);
  }

  if (this->Website.empty()) {
    mapProjectValue(this->Website, "HOMEPAGE_URL"_s);
  }

  return true;
}

bool cmProjectInfoArguments::SetEffectiveProject(cmExecutionStatus& status)
{
  if (this->NoProjectDefaults) {
    // User requested that metadata not be inherited.
    return true;
  }

  cmMakefile& mf = status.GetMakefile();
  if (!this->ProjectName.empty()) {
    // User specified a project; make sure it exists.
    if (!mf.GetStateSnapshot().CheckProjectName(this->ProjectName)) {
      status.SetError(cmStrCat(R"(PROJECT given unknown project name ")"_s,
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
