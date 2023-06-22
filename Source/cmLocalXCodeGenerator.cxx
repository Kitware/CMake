/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalXCodeGenerator.h"

#include <memory>
#include <ostream>
#include <utility>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalXCodeGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmGlobalGenerator;

cmLocalXCodeGenerator::cmLocalXCodeGenerator(cmGlobalGenerator* gg,
                                             cmMakefile* mf)
  : cmLocalGenerator(gg, mf)
{
  // the global generator does this, so do not
  // put these flags into the language flags
  this->EmitUniversalBinaryFlags = false;
}

cmLocalXCodeGenerator::~cmLocalXCodeGenerator() = default;

std::string cmLocalXCodeGenerator::GetTargetDirectory(
  cmGeneratorTarget const*) const
{
  // No per-target directory for this generator (yet).
  return "";
}

void cmLocalXCodeGenerator::AppendFlagEscape(std::string& flags,
                                             const std::string& rawFlag) const
{
  const cmGlobalXCodeGenerator* gg =
    static_cast<const cmGlobalXCodeGenerator*>(this->GlobalGenerator);
  gg->AppendFlag(flags, rawFlag);
}

void cmLocalXCodeGenerator::Generate()
{
  cmLocalGenerator::Generate();

  for (const auto& target : this->GetGeneratorTargets()) {
    target->HasMacOSXRpathInstallNameDir("");
  }
}

void cmLocalXCodeGenerator::AddGeneratorSpecificInstallSetup(std::ostream& os)
{
  // First check if we need to warn about incompatible settings
  for (const auto& target : this->GetGeneratorTargets()) {
    target->HasMacOSXRpathInstallNameDir("");
  }

  // CMakeIOSInstallCombined.cmake needs to know the location of the top of
  // the build directory
  os << "set(CMAKE_BINARY_DIR \"" << this->GetBinaryDirectory() << "\")\n\n";

  if (this->Makefile->PlatformIsAppleEmbedded()) {
    std::string platformName;
    switch (this->Makefile->GetAppleSDKType()) {
      case cmMakefile::AppleSDK::IPhoneOS:
        platformName = "iphoneos";
        break;
      case cmMakefile::AppleSDK::IPhoneSimulator:
        platformName = "iphonesimulator";
        break;
      case cmMakefile::AppleSDK::AppleTVOS:
        platformName = "appletvos";
        break;
      case cmMakefile::AppleSDK::AppleTVSimulator:
        platformName = "appletvsimulator";
        break;
      case cmMakefile::AppleSDK::WatchOS:
        platformName = "watchos";
        break;
      case cmMakefile::AppleSDK::WatchSimulator:
        platformName = "watchsimulator";
        break;
      case cmMakefile::AppleSDK::XROS:
        platformName = "xros";
        break;
      case cmMakefile::AppleSDK::XRSimulator:
        platformName = "xrsimulator";
        break;
      case cmMakefile::AppleSDK::MacOS:
        break;
    }
    if (!platformName.empty()) {
      // The effective platform name is just the platform name with a hyphen
      // prepended. We can get the SUPPORTED_PLATFORMS from the project file
      // at runtime, so we don't need to compute that here.
      /* clang-format off */
      os <<
        "if(NOT PLATFORM_NAME)\n"
        "  if(NOT \"$ENV{PLATFORM_NAME}\" STREQUAL \"\")\n"
        "    set(PLATFORM_NAME \"$ENV{PLATFORM_NAME}\")\n"
        "  endif()\n"
        "  if(NOT PLATFORM_NAME)\n"
        "    set(PLATFORM_NAME " << platformName << ")\n"
        "  endif()\n"
        "endif()\n\n"
        "if(NOT EFFECTIVE_PLATFORM_NAME)\n"
        "  if(NOT \"$ENV{EFFECTIVE_PLATFORM_NAME}\" STREQUAL \"\")\n"
        "    set(EFFECTIVE_PLATFORM_NAME \"$ENV{EFFECTIVE_PLATFORM_NAME}\")\n"
        "  endif()\n"
        "  if(NOT EFFECTIVE_PLATFORM_NAME)\n"
        "    set(EFFECTIVE_PLATFORM_NAME -" << platformName << ")\n"
        "  endif()\n"
        "endif()\n\n";
      /* clang-format off */
    }
  }
}

void cmLocalXCodeGenerator::ComputeObjectFilenames(
  std::map<cmSourceFile const*, std::string>& mapping,
  cmGeneratorTarget const*)
{
  // Count the number of object files with each name. Warn about duplicate
  // names since Xcode names them uniquely automatically with a numeric suffix
  // to avoid exact duplicate file names. Note that Mac file names are not
  // typically case sensitive, hence the LowerCase.
  std::map<std::string, int> counts;
  for (auto& si : mapping) {
    cmSourceFile const* sf = si.first;
    std::string objectName = cmStrCat(
      cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath()), ".o");

    std::string objectNameLower = cmSystemTools::LowerCase(objectName);
    counts[objectNameLower] += 1;
    if (2 == counts[objectNameLower]) {
      // TODO: emit warning about duplicate name?
    }
    si.second = objectName;
  }
}

void cmLocalXCodeGenerator::AddXCConfigSources(cmGeneratorTarget* target)
{
  auto xcconfig = target->GetProperty("XCODE_XCCONFIG");
  if (!xcconfig) {
    return;
  }
  auto configs = target->Makefile->GetGeneratorConfigs(
                          cmMakefile::IncludeEmptyConfig);

  for (auto& config : configs) {
    auto file = cmGeneratorExpression::Evaluate(
      *xcconfig,
      this, config);
    if (!file.empty()) {
      target->AddSource(file);
    }
  }
}
