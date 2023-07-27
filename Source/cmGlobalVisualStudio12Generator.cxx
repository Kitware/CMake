/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio12Generator.h"

#include <cstring>
#include <sstream>
#include <vector>

#include <cmext/string_view>

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static const char vs12generatorName[] = "Visual Studio 12 2013";

// Map generator name without year to name with year.
static const char* cmVS12GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs12generatorName,
              sizeof(vs12generatorName) - 6) != 0) {
    return nullptr;
  }
  const char* p = name.c_str() + sizeof(vs12generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2013")) {
    p += 5;
  }
  genName = std::string(vs12generatorName) + p;
  return p;
}

class cmGlobalVisualStudio12Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool allowArch, cmake* cm) const override
  {
    std::string genName;
    const char* p = cmVS12GenName(name, genName);
    if (!p) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (!*p) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio12Generator(cm, genName, ""));
    }
    if (!allowArch || *p++ != ' ') {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (strcmp(p, "Win64") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio12Generator(cm, genName, "x64"));
    }
    if (strcmp(p, "ARM") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio12Generator(cm, genName, "ARM"));
    }
    return std::unique_ptr<cmGlobalGenerator>();
  }

  cmDocumentationEntry GetDocumentation() const override
  {
    return { cmStrCat(vs12generatorName, " [arch]"),
             "Deprecated.  Generates Visual Studio 2013 project files.  "
             "Optional [arch] can be \"Win64\" or \"ARM\"." };
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs12generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    std::vector<std::string> names;
    names.emplace_back(cmStrCat(vs12generatorName, " ARM"));
    names.emplace_back(cmStrCat(vs12generatorName, " Win64"));
    return names;
  }

  bool SupportsToolset() const override { return true; }
  bool SupportsPlatform() const override { return true; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    std::vector<std::string> platforms;
    platforms.emplace_back("x64");
    platforms.emplace_back("Win32");
    platforms.emplace_back("ARM");
    return platforms;
  }

  std::string GetDefaultPlatformName() const override { return "Win32"; }
};

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalVisualStudio12Generator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory);
}

cmGlobalVisualStudio12Generator::cmGlobalVisualStudio12Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio11Generator(cm, name, platformInGeneratorName)
{
  std::string vc12Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\12.0\\Setup\\VC;"
    "ProductDir",
    vc12Express, cmSystemTools::KeyWOW64_32);
  this->DefaultPlatformToolset = "v120";
  this->DefaultCLFlagTableName = "v12";
  this->DefaultCSharpFlagTableName = "v12";
  this->DefaultLibFlagTableName = "v12";
  this->DefaultLinkFlagTableName = "v12";
  this->DefaultMasmFlagTableName = "v12";
  this->DefaultRCFlagTableName = "v12";
  this->Version = VSVersion::VS12;
}

bool cmGlobalVisualStudio12Generator::MatchesGeneratorName(
  const std::string& name) const
{
  std::string genName;
  if (cmVS12GenName(name, genName)) {
    return genName == this->GetName();
  }
  return false;
}

bool cmGlobalVisualStudio12Generator::ProcessGeneratorToolsetField(
  std::string const& key, std::string const& value)
{
  if (key == "host"_s &&
      (value == "x64"_s || value == "x86"_s || value == "ARM64"_s)) {
    this->GeneratorToolsetHostArchitecture = value;
    return true;
  }
  return this->cmGlobalVisualStudio11Generator::ProcessGeneratorToolsetField(
    key, value);
}

bool cmGlobalVisualStudio12Generator::InitializeWindowsPhone(cmMakefile* mf)
{
  if (!this->SelectWindowsPhoneToolset(this->DefaultPlatformToolset)) {
    std::string e;
    if (this->DefaultPlatformToolset.empty()) {
      e = cmStrCat(this->GetName(),
                   " supports Windows Phone '8.0' and '8.1', but "
                   "not '",
                   this->SystemVersion, "'.  Check CMAKE_SYSTEM_VERSION.");
    } else {
      e = cmStrCat(
        "A Windows Phone component with CMake requires both the Windows "
        "Desktop SDK as well as the Windows Phone '",
        this->SystemVersion,
        "' SDK. Please make sure that you have both installed");
    }
    mf->IssueMessage(MessageType::FATAL_ERROR, e);
    return false;
  }
  return true;
}

bool cmGlobalVisualStudio12Generator::InitializeWindowsStore(cmMakefile* mf)
{
  if (!this->SelectWindowsStoreToolset(this->DefaultPlatformToolset)) {
    std::string e;
    if (this->DefaultPlatformToolset.empty()) {
      e = cmStrCat(this->GetName(),
                   " supports Windows Store '8.0' and '8.1', but "
                   "not '",
                   this->SystemVersion, "'.  Check CMAKE_SYSTEM_VERSION.");
    } else {
      e = cmStrCat(
        "A Windows Store component with CMake requires both the Windows "
        "Desktop SDK as well as the Windows Store '",
        this->SystemVersion,
        "' SDK. Please make sure that you have both installed");
    }
    mf->IssueMessage(MessageType::FATAL_ERROR, e);
    return false;
  }
  return true;
}

bool cmGlobalVisualStudio12Generator::SelectWindowsPhoneToolset(
  std::string& toolset) const
{
  if (this->SystemVersion == "8.1"_s) {
    if (this->IsWindowsPhoneToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = "v120_wp81";
      return true;
    }
    return false;
  }
  return this->cmGlobalVisualStudio11Generator::SelectWindowsPhoneToolset(
    toolset);
}

bool cmGlobalVisualStudio12Generator::SelectWindowsStoreToolset(
  std::string& toolset) const
{
  if (this->SystemVersion == "8.1"_s) {
    if (this->IsWindowsStoreToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = "v120";
      return true;
    }
    return false;
  }
  return this->cmGlobalVisualStudio11Generator::SelectWindowsStoreToolset(
    toolset);
}

bool cmGlobalVisualStudio12Generator::IsWindowsDesktopToolsetInstalled() const
{
  const char desktop81Key[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                              "VisualStudio\\12.0\\VC\\LibraryDesktop";

  std::vector<std::string> subkeys;
  return cmSystemTools::GetRegistrySubKeys(desktop81Key, subkeys,
                                           cmSystemTools::KeyWOW64_32);
}

bool cmGlobalVisualStudio12Generator::IsWindowsPhoneToolsetInstalled() const
{
  const char wp81Key[] =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
    "Microsoft SDKs\\WindowsPhone\\v8.1\\Install Path;Install Path";

  std::string path;
  cmSystemTools::ReadRegistryValue(wp81Key, path, cmSystemTools::KeyWOW64_32);
  return !path.empty();
}

bool cmGlobalVisualStudio12Generator::IsWindowsStoreToolsetInstalled() const
{
  const char win81Key[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                          "VisualStudio\\12.0\\VC\\Libraries\\Core\\Arm";

  std::vector<std::string> subkeys;
  return cmSystemTools::GetRegistrySubKeys(win81Key, subkeys,
                                           cmSystemTools::KeyWOW64_32);
}
