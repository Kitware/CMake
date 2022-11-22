/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio11Generator.h"

#include <cstring>
#include <sstream>
#include <utility>
#include <vector>

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static const char vs11generatorName[] = "Visual Studio 11 2012";

// Map generator name without year to name with year.
static const char* cmVS11GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs11generatorName,
              sizeof(vs11generatorName) - 6) != 0) {
    return nullptr;
  }
  const char* p = name.c_str() + sizeof(vs11generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2012")) {
    p += 5;
  }
  genName = std::string(vs11generatorName) + p;
  return p;
}

class cmGlobalVisualStudio11Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool allowArch, cmake* cm) const override
  {
    std::string genName;
    const char* p = cmVS11GenName(name, genName);
    if (!p) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (!*p) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio11Generator(cm, genName, ""));
    }
    if (!allowArch || *p++ != ' ') {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (strcmp(p, "Win64") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio11Generator(cm, genName, "x64"));
    }
    if (strcmp(p, "ARM") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio11Generator(cm, genName, "ARM"));
    }

    std::set<std::string> installedSDKs =
      cmGlobalVisualStudio11Generator::GetInstalledWindowsCESDKs();

    if (installedSDKs.find(p) == installedSDKs.end()) {
      return std::unique_ptr<cmGlobalGenerator>();
    }

    auto ret = std::unique_ptr<cmGlobalVisualStudio11Generator>(
      new cmGlobalVisualStudio11Generator(cm, name, p));
    ret->WindowsCEVersion = "8.00";
    return std::unique_ptr<cmGlobalGenerator>(std::move(ret));
  }

  cmDocumentationEntry GetDocumentation() const override
  {
    return { std::string(vs11generatorName) + " [arch]",
             "Deprecated.  Generates Visual Studio 2012 project files.  "
             "Optional [arch] can be \"Win64\" or \"ARM\"." };
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs11generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    std::vector<std::string> names;
    names.push_back(vs11generatorName + std::string(" ARM"));
    names.push_back(vs11generatorName + std::string(" Win64"));

    std::set<std::string> installedSDKs =
      cmGlobalVisualStudio11Generator::GetInstalledWindowsCESDKs();
    for (std::string const& i : installedSDKs) {
      names.push_back(std::string(vs11generatorName) + " " + i);
    }

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

    std::set<std::string> installedSDKs =
      cmGlobalVisualStudio11Generator::GetInstalledWindowsCESDKs();
    for (std::string const& i : installedSDKs) {
      platforms.emplace_back(i);
    }

    return platforms;
  }

  std::string GetDefaultPlatformName() const override { return "Win32"; }
};

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalVisualStudio11Generator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory);
}

cmGlobalVisualStudio11Generator::cmGlobalVisualStudio11Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio10Generator(cm, name, platformInGeneratorName)
{
  std::string vc11Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\11.0\\Setup\\VC;"
    "ProductDir",
    vc11Express, cmSystemTools::KeyWOW64_32);
  this->DefaultPlatformToolset = "v110";
  this->DefaultCLFlagTableName = "v11";
  this->DefaultCSharpFlagTableName = "v11";
  this->DefaultLibFlagTableName = "v11";
  this->DefaultLinkFlagTableName = "v11";
  this->DefaultMasmFlagTableName = "v11";
  this->DefaultRCFlagTableName = "v11";
  this->Version = VSVersion::VS11;
}

bool cmGlobalVisualStudio11Generator::MatchesGeneratorName(
  const std::string& name) const
{
  std::string genName;
  if (cmVS11GenName(name, genName)) {
    return genName == this->GetName();
  }
  return false;
}

void cmGlobalVisualStudio11Generator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  for (std::string const& it : lang) {
    if (it == "ASM_MARMASM") {
      this->MarmasmEnabled = true;
    }
  }
  this->AddPlatformDefinitions(mf);
  cmGlobalVisualStudio10Generator::EnableLanguage(lang, mf, optional);
}

bool cmGlobalVisualStudio11Generator::InitializeWindowsPhone(cmMakefile* mf)
{
  if (!this->SelectWindowsPhoneToolset(this->DefaultPlatformToolset)) {
    std::ostringstream e;
    if (this->DefaultPlatformToolset.empty()) {
      e << this->GetName() << " supports Windows Phone '8.0', but not '"
        << this->SystemVersion << "'.  Check CMAKE_SYSTEM_VERSION.";
    } else {
      e << "A Windows Phone component with CMake requires both the Windows "
        << "Desktop SDK as well as the Windows Phone '" << this->SystemVersion
        << "' SDK. Please make sure that you have both installed";
    }
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }
  return true;
}

bool cmGlobalVisualStudio11Generator::InitializeWindowsStore(cmMakefile* mf)
{
  if (!this->SelectWindowsStoreToolset(this->DefaultPlatformToolset)) {
    std::ostringstream e;
    if (this->DefaultPlatformToolset.empty()) {
      e << this->GetName() << " supports Windows Store '8.0', but not '"
        << this->SystemVersion << "'.  Check CMAKE_SYSTEM_VERSION.";
    } else {
      e << "A Windows Store component with CMake requires both the Windows "
        << "Desktop SDK as well as the Windows Store '" << this->SystemVersion
        << "' SDK. Please make sure that you have both installed";
    }
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }
  return true;
}

bool cmGlobalVisualStudio11Generator::SelectWindowsPhoneToolset(
  std::string& toolset) const
{
  if (this->SystemVersion == "8.0") {
    if (this->IsWindowsPhoneToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = "v110_wp80";
      return true;
    }
    return false;
  }
  return this->cmGlobalVisualStudio10Generator::SelectWindowsPhoneToolset(
    toolset);
}

bool cmGlobalVisualStudio11Generator::SelectWindowsStoreToolset(
  std::string& toolset) const
{
  if (this->SystemVersion == "8.0") {
    if (this->IsWindowsStoreToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = "v110";
      return true;
    }
    return false;
  }
  return this->cmGlobalVisualStudio10Generator::SelectWindowsStoreToolset(
    toolset);
}

bool cmGlobalVisualStudio11Generator::UseFolderProperty() const
{
  // Intentionally skip up to the top-level class implementation.
  // Folders are not supported by the Express editions in VS10 and earlier,
  // but they are in VS11 Express and above.
  // NOLINTNEXTLINE(bugprone-parent-virtual-call)
  return cmGlobalGenerator::UseFolderProperty();
}

std::set<std::string>
cmGlobalVisualStudio11Generator::GetInstalledWindowsCESDKs()
{
  const char sdksKey[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                         "Windows CE Tools\\SDKs";

  std::vector<std::string> subkeys;
  cmSystemTools::GetRegistrySubKeys(sdksKey, subkeys,
                                    cmSystemTools::KeyWOW64_32);

  std::set<std::string> ret;
  for (std::string const& i : subkeys) {
    std::string key = sdksKey;
    key += '\\';
    key += i;
    key += ';';

    std::string path;
    if (cmSystemTools::ReadRegistryValue(key, path,
                                         cmSystemTools::KeyWOW64_32) &&
        !path.empty()) {
      ret.insert(i);
    }
  }

  return ret;
}

bool cmGlobalVisualStudio11Generator::TargetSystemSupportsDeployment() const
{
  return this->SystemIsWindowsPhone || this->SystemIsWindowsStore ||
    cmGlobalVisualStudio10Generator::TargetSystemSupportsDeployment();
}

bool cmGlobalVisualStudio11Generator::IsWindowsDesktopToolsetInstalled() const
{
  const char desktop80Key[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                              "VisualStudio\\11.0\\VC\\Libraries\\Extended";
  const char VS2012DesktopExpressKey[] =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
    "WDExpress\\11.0;InstallDir";

  std::vector<std::string> subkeys;
  std::string path;
  return cmSystemTools::ReadRegistryValue(VS2012DesktopExpressKey, path,
                                          cmSystemTools::KeyWOW64_32) ||
    cmSystemTools::GetRegistrySubKeys(desktop80Key, subkeys,
                                      cmSystemTools::KeyWOW64_32);
}

bool cmGlobalVisualStudio11Generator::IsWindowsPhoneToolsetInstalled() const
{
  const char wp80Key[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                         "Microsoft SDKs\\WindowsPhone\\v8.0\\"
                         "Install Path;Install Path";

  std::string path;
  cmSystemTools::ReadRegistryValue(wp80Key, path, cmSystemTools::KeyWOW64_32);
  return !path.empty();
}

bool cmGlobalVisualStudio11Generator::IsWindowsStoreToolsetInstalled() const
{
  const char win80Key[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                          "VisualStudio\\11.0\\VC\\Libraries\\Core\\Arm";

  std::vector<std::string> subkeys;
  return cmSystemTools::GetRegistrySubKeys(win80Key, subkeys,
                                           cmSystemTools::KeyWOW64_32);
}
