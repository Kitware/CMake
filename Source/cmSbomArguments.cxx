/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSbomArguments.h"

#include <cm/string_view>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmStringAlgorithms.h"

namespace {

struct SbomFormatEntry
{
  cm::string_view Name;
  cmSbomArguments::SbomFormat Format;
};

SbomFormatEntry const SbomFormats[] = {
  { "spdx"_s, cmSbomArguments::SbomFormat::SPDX_3_0_JSON },
  { "spdx-3.0+json"_s, cmSbomArguments::SbomFormat::SPDX_3_0_JSON },
  { "spdx-3.0.1+json"_s, cmSbomArguments::SbomFormat::SPDX_3_0_JSON },
};

} // namespace

template void cmSbomArguments::Bind<void>(cmArgumentParser<void>&,
                                          cmSbomArguments*);

bool cmSbomArguments::Check(cmExecutionStatus& status) const
{
  if (!this->PackageName.empty()) {
    if (!cmGeneratorExpression::IsValidTargetName(this->PackageName) ||
        this->PackageName.find(':') != std::string::npos) {
      status.SetError(cmStrCat(R"(SBOM given invalid package name ")"_s,
                               this->PackageName, R"(".)"_s));
      return false;
    }
  }

  if (this->GetFormat() == SbomFormat::NONE) {
    status.SetError(
      cmStrCat(R"(SBOM given invalid format ")"_s, this->Format, R"(".)"_s));
    return false;
  }

  if (!this->License.empty()) {
    // Do not allow the license argument to be provided as SBOM only accepts a
    // DEFAULT_LICENSE
    status.SetError("SBOM given unknown argument: \"LICENSE\".");
    return false;
  }

  return true;
}

cm::string_view cmSbomArguments::CommandName() const
{
  return "SBOM"_s;
}

std::string cmSbomArguments::GetNamespace() const
{
  return cmStrCat(this->PackageName, "::"_s);
}

std::string cmSbomArguments::GetPackageName() const
{
  return this->PackageName;
}

std::string const& cmSbomArguments::GetDataLicense() const
{
  return this->DataLicense.empty() ? this->License : this->DataLicense;
}

std::string cmSbomArguments::GetDefaultDestination(
  std::string const& root) const
{
  if (root.empty()) {
    return cmStrCat("sbom/"_s, this->GetPackageName());
  }
  return cmStrCat(root, "/sbom/"_s, this->GetPackageName());
}

cmSbomArguments::SbomFormat cmSbomArguments::GetFormat() const
{
  if (this->Format.empty()) {
    return SbomFormat::SPDX_3_0_JSON;
  }

  for (auto const& entry : SbomFormats) {
    if (this->Format == entry.Name) {
      return entry.Format;
    }
  }
  return SbomFormat::NONE;
}
