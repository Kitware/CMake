/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio11Generator.h"

#include <cstring>
#include <utility>
#include <vector>

#include <cmext/string_view>

#include "cmGlobalGenerator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmGlobalVisualStudio11Generator::cmGlobalVisualStudio11Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio10Generator(cm, name, platformInGeneratorName)
{
}

void cmGlobalVisualStudio11Generator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  for (std::string const& it : lang) {
    if (it == "ASM_MARMASM"_s) {
      this->MarmasmEnabled = true;
    }
  }
  this->AddPlatformDefinitions(mf);
  cmGlobalVisualStudio10Generator::EnableLanguage(lang, mf, optional);
}

bool cmGlobalVisualStudio11Generator::InitializeWindowsPhone(cmMakefile* mf)
{
  if (!this->SelectWindowsPhoneToolset(this->DefaultPlatformToolset)) {
    std::string e;
    if (this->DefaultPlatformToolset.empty()) {
      e = cmStrCat(this->GetName(), " supports Windows Phone '8.0', but not '",
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

bool cmGlobalVisualStudio11Generator::InitializeWindowsStore(cmMakefile* mf)
{
  if (!this->SelectWindowsStoreToolset(this->DefaultPlatformToolset)) {
    std::string e;
    if (this->DefaultPlatformToolset.empty()) {
      e = cmStrCat(this->GetName(), " supports Windows Store '8.0', but not '",
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

bool cmGlobalVisualStudio11Generator::SelectWindowsPhoneToolset(
  std::string& toolset) const
{
  if (this->SystemVersion == "8.0"_s) {
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
  if (this->SystemVersion == "8.0"_s) {
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
