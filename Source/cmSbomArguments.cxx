/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSbomArguments.h"

#include <algorithm>
#include <cctype>

#include <cm/string_view>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmStringAlgorithms.h"

struct SbomFormatPattern
{
  cm::string_view Name;
  cm::string_view Alias;
  cmSbomArguments::SbomFormat Id;
};

static SbomFormatPattern SbomPatterns[] = {
  { "spdx-3.0+json", "spdx", cmSbomArguments::SbomFormat::SPDX_3_0_JSON },
};

cmSbomArguments::SbomFormat ParseSbomFormat(std::string const& input)
{
  if (input.empty()) {
    return cmSbomArguments::SbomFormat::SPDX_3_0_JSON;
  }

  cm::string_view s(input);
  for (auto const& p : SbomPatterns) {
    if (s == p.Name || (!p.Alias.empty() && s == p.Alias)) {
      return p.Id;
    }
  }
  return cmSbomArguments::SbomFormat::NONE;
}

std::string GetSbomFileExtension(cmSbomArguments::SbomFormat id)
{
  switch (id) {
    case cmSbomArguments::SbomFormat::SPDX_3_0_JSON:
      return ".spdx.json";
    default:
      return "";
  }
}

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

  if (!this->Format.empty()) {
    if (this->GetFormat() == SbomFormat::NONE) {
      status.SetError(
        cmStrCat(R"(SBOM given invalid format ")"_s, this->Format, R"(".)"_s));
      return false;
    }
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

std::string cmSbomArguments::GetPackageDirName() const
{
  return this->PackageName;
}

cmSbomArguments::SbomFormat cmSbomArguments::GetFormat() const
{
  if (this->Format.empty()) {
    return SbomFormat::SPDX_3_0_JSON;
  }
  return ParseSbomFormat(this->Format);
}

std::string cmSbomArguments::GetPackageFileName() const
{
  std::string const pkgNameOnDisk = this->GetPackageDirName();
  std::string format = GetSbomFileExtension(this->GetFormat());
  std::transform(format.begin(), format.end(), format.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return cmStrCat(pkgNameOnDisk, format);
}
