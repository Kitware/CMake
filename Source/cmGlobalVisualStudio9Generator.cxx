/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio9Generator.h"

#include <cstring>
#include <utility>
#include <vector>

#include "cmDocumentationEntry.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmSystemTools.h"
#include "cmVisualStudioWCEPlatformParser.h"

class cmake;

static const char vs9generatorName[] = "Visual Studio 9 2008";

class cmGlobalVisualStudio9Generator::Factory : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool allowArch, cmake* cm) const override
  {
    if (strncmp(name.c_str(), vs9generatorName,
                sizeof(vs9generatorName) - 1) != 0) {
      return std::unique_ptr<cmGlobalGenerator>();
    }

    const char* p = name.c_str() + sizeof(vs9generatorName) - 1;
    if (p[0] == '\0') {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio9Generator(cm, name, ""));
    }

    if (!allowArch || p[0] != ' ') {
      return std::unique_ptr<cmGlobalGenerator>();
    }

    ++p;

    if (!strcmp(p, "IA64")) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio9Generator(cm, name, "Itanium"));
    }

    if (!strcmp(p, "Win64")) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio9Generator(cm, name, "x64"));
    }

    cmVisualStudioWCEPlatformParser parser(p);
    parser.ParseVersion("9.0");
    if (!parser.Found()) {
      return std::unique_ptr<cmGlobalGenerator>();
    }

    auto ret = std::unique_ptr<cmGlobalVisualStudio9Generator>(
      new cmGlobalVisualStudio9Generator(cm, name, p));
    ret->WindowsCEVersion = parser.GetOSVersion();
    return std::unique_ptr<cmGlobalGenerator>(std::move(ret));
  }

  void GetDocumentation(cmDocumentationEntry& entry) const override
  {
    entry.Name = std::string(vs9generatorName) + " [arch]";
    entry.Brief = "Generates Visual Studio 2008 project files.  "
                  "Optional [arch] can be \"Win64\" or \"IA64\".";
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs9generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    std::vector<std::string> names;
    names.push_back(vs9generatorName + std::string(" Win64"));
    names.push_back(vs9generatorName + std::string(" IA64"));
    cmVisualStudioWCEPlatformParser parser;
    parser.ParseVersion("9.0");
    const std::vector<std::string>& availablePlatforms =
      parser.GetAvailablePlatforms();
    for (std::string const& i : availablePlatforms) {
      names.push_back("Visual Studio 9 2008 " + i);
    }
    return names;
  }

  bool SupportsToolset() const override { return false; }
  bool SupportsPlatform() const override { return true; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    std::vector<std::string> platforms;
    platforms.emplace_back("x64");
    platforms.emplace_back("Win32");
    platforms.emplace_back("Itanium");
    cmVisualStudioWCEPlatformParser parser;
    parser.ParseVersion("9.0");
    const std::vector<std::string>& availablePlatforms =
      parser.GetAvailablePlatforms();
    for (std::string const& i : availablePlatforms) {
      platforms.emplace_back(i);
    }
    return platforms;
  }

  std::string GetDefaultPlatformName() const override { return "Win32"; }
};

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalVisualStudio9Generator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory);
}

cmGlobalVisualStudio9Generator::cmGlobalVisualStudio9Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio8Generator(cm, name, platformInGeneratorName)
{
  this->Version = VSVersion::VS9;
  std::string vc9Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\9.0\\Setup\\VC;"
    "ProductDir",
    vc9Express, cmSystemTools::KeyWOW64_32);
}

std::string cmGlobalVisualStudio9Generator::GetUserMacrosDirectory()
{
  std::string base;
  std::string path;

  // base begins with the VisualStudioProjectsLocation reg value...
  if (cmSystemTools::ReadRegistryValue(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\VisualStudio\\9.0;"
        "VisualStudioProjectsLocation",
        base)) {
    cmSystemTools::ConvertToUnixSlashes(base);

    // 9.0 macros folder:
    path = base + "/VSMacros80";
    // *NOT* a typo; right now in Visual Studio 2008 beta the macros
    // folder is VSMacros80... They may change it to 90 before final
    // release of 2008 or they may not... we'll have to keep our eyes
    // on it
  }

  // path is (correctly) still empty if we did not read the base value from
  // the Registry value
  return path;
}

std::string cmGlobalVisualStudio9Generator::GetUserMacrosRegKeyBase()
{
  return "Software\\Microsoft\\VisualStudio\\9.0\\vsmacros";
}
