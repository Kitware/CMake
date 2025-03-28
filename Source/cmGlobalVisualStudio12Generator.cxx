/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio12Generator.h"

#include <cstring>
#include <sstream>
#include <vector>

#include <cmext/string_view>

#include "cmGlobalGenerator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmGlobalVisualStudio12Generator::cmGlobalVisualStudio12Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio11Generator(cm, name, platformInGeneratorName)
{
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
