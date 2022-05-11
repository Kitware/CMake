/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio14Generator.h"

#include <cstring>
#include <sstream>

#include <cm/vector>

#include "cmDocumentationEntry.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

static const char vs14generatorName[] = "Visual Studio 14 2015";

// Map generator name without year to name with year.
static const char* cmVS14GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs14generatorName,
              sizeof(vs14generatorName) - 6) != 0) {
    return 0;
  }
  const char* p = name.c_str() + sizeof(vs14generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2015")) {
    p += 5;
  }
  genName = std::string(vs14generatorName) + p;
  return p;
}

class cmGlobalVisualStudio14Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool allowArch, cmake* cm) const override
  {
    std::string genName;
    const char* p = cmVS14GenName(name, genName);
    if (!p) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (!*p) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio14Generator(cm, genName, ""));
    }
    if (!allowArch || *p++ != ' ') {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (strcmp(p, "Win64") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio14Generator(cm, genName, "x64"));
    }
    if (strcmp(p, "ARM") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio14Generator(cm, genName, "ARM"));
    }
    return std::unique_ptr<cmGlobalGenerator>();
  }

  void GetDocumentation(cmDocumentationEntry& entry) const override
  {
    entry.Name = std::string(vs14generatorName) + " [arch]";
    entry.Brief = "Generates Visual Studio 2015 project files.  "
                  "Optional [arch] can be \"Win64\" or \"ARM\".";
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs14generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    std::vector<std::string> names;
    names.push_back(vs14generatorName + std::string(" ARM"));
    names.push_back(vs14generatorName + std::string(" Win64"));
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
cmGlobalVisualStudio14Generator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory);
}

cmGlobalVisualStudio14Generator::cmGlobalVisualStudio14Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio12Generator(cm, name, platformInGeneratorName)
{
  std::string vc14Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\14.0\\Setup\\VC;"
    "ProductDir",
    vc14Express, cmSystemTools::KeyWOW64_32);
  this->DefaultPlatformToolset = "v140";
  this->DefaultAndroidToolset = "Clang_3_8";
  this->DefaultCLFlagTableName = "v140";
  this->DefaultCSharpFlagTableName = "v140";
  this->DefaultLibFlagTableName = "v14";
  this->DefaultLinkFlagTableName = "v140";
  this->DefaultMasmFlagTableName = "v14";
  this->DefaultRCFlagTableName = "v14";
  this->Version = VSVersion::VS14;
}

bool cmGlobalVisualStudio14Generator::MatchesGeneratorName(
  const std::string& name) const
{
  std::string genName;
  if (cmVS14GenName(name, genName)) {
    return genName == this->GetName();
  }
  return false;
}

bool cmGlobalVisualStudio14Generator::InitializeWindows(cmMakefile* mf)
{
  if (cmHasLiteralPrefix(this->SystemVersion, "10.0")) {
    return this->SelectWindows10SDK(mf, false);
  }
  return true;
}

bool cmGlobalVisualStudio14Generator::InitializeWindowsStore(cmMakefile* mf)
{
  std::ostringstream e;
  if (!this->SelectWindowsStoreToolset(this->DefaultPlatformToolset)) {
    if (this->DefaultPlatformToolset.empty()) {
      e << this->GetName()
        << " supports Windows Store '8.0', '8.1' and "
           "'10.0', but not '"
        << this->SystemVersion << "'.  Check CMAKE_SYSTEM_VERSION.";
    } else {
      e << "A Windows Store component with CMake requires both the Windows "
        << "Desktop SDK as well as the Windows Store '" << this->SystemVersion
        << "' SDK. Please make sure that you have both installed";
    }
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }
  if (cmHasLiteralPrefix(this->SystemVersion, "10.0")) {
    return this->SelectWindows10SDK(mf, true);
  }
  return true;
}

bool cmGlobalVisualStudio14Generator::InitializeAndroid(cmMakefile*)
{
  return true;
}

bool cmGlobalVisualStudio14Generator::SelectWindows10SDK(cmMakefile* mf,
                                                         bool required)
{
  // Find the default version of the Windows 10 SDK.
  std::string const version = this->GetWindows10SDKVersion(mf);

  if (required && version.empty()) {
    std::ostringstream e;
    e << "Could not find an appropriate version of the Windows 10 SDK"
      << " installed on this machine";
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }
  this->SetWindowsTargetPlatformVersion(version, mf);
  return true;
}

void cmGlobalVisualStudio14Generator::SetWindowsTargetPlatformVersion(
  std::string const& version, cmMakefile* mf)
{
  this->WindowsTargetPlatformVersion = version;
  if (!cmSystemTools::VersionCompareEqual(this->WindowsTargetPlatformVersion,
                                          this->SystemVersion)) {
    std::ostringstream e;
    e << "Selecting Windows SDK version " << this->WindowsTargetPlatformVersion
      << " to target Windows " << this->SystemVersion << ".";
    mf->DisplayStatus(e.str(), -1);
  }
  mf->AddDefinition("CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION",
                    this->WindowsTargetPlatformVersion);
}

bool cmGlobalVisualStudio14Generator::SelectWindowsStoreToolset(
  std::string& toolset) const
{
  if (cmHasLiteralPrefix(this->SystemVersion, "10.0")) {
    if (this->IsWindowsStoreToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = "v140";
      return true;
    } else {
      return false;
    }
  }
  return this->cmGlobalVisualStudio12Generator::SelectWindowsStoreToolset(
    toolset);
}

bool cmGlobalVisualStudio14Generator::IsWindowsDesktopToolsetInstalled() const
{
  const char desktop10Key[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                              "VisualStudio\\14.0\\VC\\Runtimes";

  std::vector<std::string> vc14;
  return cmSystemTools::GetRegistrySubKeys(desktop10Key, vc14,
                                           cmSystemTools::KeyWOW64_32);
}

bool cmGlobalVisualStudio14Generator::IsWindowsStoreToolsetInstalled() const
{
  const char universal10Key[] =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
    "VisualStudio\\14.0\\Setup\\Build Tools for Windows 10;SrcPath";

  std::string win10SDK;
  return cmSystemTools::ReadRegistryValue(universal10Key, win10SDK,
                                          cmSystemTools::KeyWOW64_32);
}

std::string cmGlobalVisualStudio14Generator::GetWindows10SDKMaxVersion(
  cmMakefile* mf) const
{
  // if the given value is set, it can either be OFF/FALSE or a valid SDK
  // string
  if (cmValue value = mf->GetDefinition(
        "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION_MAXIMUM")) {

    // If the value is some off/false value, then there is NO maximum set.
    if (cmIsOff(value)) {
      return std::string();
    }
    // If the value is something else, trust that it is a valid SDK value.
    else if (value) {
      return *value;
    }
    // If value is an invalid pointer, leave result unchanged.
  }

  return this->GetWindows10SDKMaxVersionDefault(mf);
}

std::string cmGlobalVisualStudio14Generator::GetWindows10SDKMaxVersionDefault(
  cmMakefile*) const
{
  // The last Windows 10 SDK version that VS 2015 can target is 10.0.14393.0.
  //
  // "VS 2015 Users: The Windows 10 SDK (15063, 16299, 17134, 17763) is
  // officially only supported for VS 2017." From:
  // https://blogs.msdn.microsoft.com/chuckw/2018/10/02/windows-10-october-2018-update/
  return "10.0.14393.0";
}

#if defined(_WIN32) && !defined(__CYGWIN__)
struct NoWindowsH
{
  bool operator()(std::string const& p)
  {
    return !cmSystemTools::FileExists(p + "/um/windows.h", true);
  }
};
class WindowsSDKTooRecent
{
  std::string const& MaxVersion;

public:
  WindowsSDKTooRecent(std::string const& maxVersion)
    : MaxVersion(maxVersion)
  {
  }
  bool operator()(std::string const& v)
  {
    return cmSystemTools::VersionCompareGreater(v, MaxVersion);
  }
};
#endif

std::string cmGlobalVisualStudio14Generator::GetWindows10SDKVersion(
  cmMakefile* mf)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::vector<std::string> win10Roots;

  {
    std::string win10Root;
    if (cmSystemTools::GetEnv("CMAKE_WINDOWS_KITS_10_DIR", win10Root)) {
      cmSystemTools::ConvertToUnixSlashes(win10Root);
      win10Roots.push_back(win10Root);
    }
  }

  {
    // This logic is taken from the vcvarsqueryregistry.bat file from VS2015
    // Try HKLM and then HKCU.
    std::string win10Root;
    if (cmSystemTools::ReadRegistryValue(
          "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
          "Windows Kits\\Installed Roots;KitsRoot10",
          win10Root, cmSystemTools::KeyWOW64_32) ||
        cmSystemTools::ReadRegistryValue(
          "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\"
          "Windows Kits\\Installed Roots;KitsRoot10",
          win10Root, cmSystemTools::KeyWOW64_32)) {
      cmSystemTools::ConvertToUnixSlashes(win10Root);
      win10Roots.push_back(win10Root);
    }
  }

  if (win10Roots.empty()) {
    return std::string();
  }

  std::vector<std::string> sdks;
  // Grab the paths of the different SDKs that are installed
  for (std::string const& i : win10Roots) {
    std::string path = i + "/Include/*";
    cmSystemTools::GlobDirs(path, sdks);
  }

  // Skip SDKs that do not contain <um/windows.h> because that indicates that
  // only the UCRT MSIs were installed for them.
  cm::erase_if(sdks, NoWindowsH());

  // Only use the filename, which will be the SDK version.
  for (std::string& i : sdks) {
    i = cmSystemTools::GetFilenameName(i);
  }

  // Skip SDKs that cannot be used with our toolset, unless the user does not
  // want to limit the highest supported SDK according to the Microsoft
  // documentation.
  std::string maxVersion = this->GetWindows10SDKMaxVersion(mf);
  if (!maxVersion.empty()) {
    cm::erase_if(sdks, WindowsSDKTooRecent(maxVersion));
  }

  // Sort the results to make sure we select the most recent one.
  std::sort(sdks.begin(), sdks.end(), cmSystemTools::VersionCompareGreater);

  // Look for a SDK exactly matching the requested target version.
  for (std::string const& i : sdks) {
    if (cmSystemTools::VersionCompareEqual(i, this->SystemVersion)) {
      return i;
    }
  }

  if (!sdks.empty()) {
    // Use the latest Windows 10 SDK since the exact version is not available.
    return sdks.at(0);
  }
#endif
  // Return an empty string
  return std::string();
}
