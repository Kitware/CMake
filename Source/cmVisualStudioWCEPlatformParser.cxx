/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmVisualStudioWCEPlatformParser.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "cmGlobalVisualStudioGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

int cmVisualStudioWCEPlatformParser::ParseVersion(char const* version)
{
  std::string const registryBase =
    cmGlobalVisualStudioGenerator::GetRegistryBase(version);
  std::string const vckey = cmStrCat(registryBase, "\\Setup\\VC;ProductDir");
  std::string const vskey = cmStrCat(registryBase, "\\Setup\\VS;ProductDir");

  if (!cmSystemTools::ReadRegistryValue(vckey, this->VcInstallDir,
                                        cmSystemTools::KeyWOW64_32) ||
      !cmSystemTools::ReadRegistryValue(vskey, this->VsInstallDir,
                                        cmSystemTools::KeyWOW64_32)) {
    return 0;
  }
  cmSystemTools::ConvertToUnixSlashes(this->VcInstallDir);
  cmSystemTools::ConvertToUnixSlashes(this->VsInstallDir);
  this->VcInstallDir.append("//");

  std::string const configFilename =
    cmStrCat(this->VcInstallDir, "vcpackages/WCE.VCPlatform.config");

  return this->ParseFile(configFilename.c_str());
}

std::string cmVisualStudioWCEPlatformParser::GetOSVersion() const
{
  if (this->OSMinorVersion.empty()) {
    return OSMajorVersion;
  }

  return cmStrCat(OSMajorVersion, '.', OSMinorVersion);
}

char const* cmVisualStudioWCEPlatformParser::GetArchitectureFamily() const
{
  auto it = this->Macros.find("ARCHFAM");
  if (it != this->Macros.end()) {
    return it->second.c_str();
  }

  return nullptr;
}

void cmVisualStudioWCEPlatformParser::StartElement(std::string const& name,
                                                   char const** attributes)
{
  if (this->FoundRequiredName) {
    return;
  }

  this->CharacterData.clear();

  if (name == "PlatformData"_s) {
    this->PlatformName.clear();
    this->OSMajorVersion.clear();
    this->OSMinorVersion.clear();
    this->Macros.clear();
  }

  if (name == "Macro"_s) {
    std::string macroName;
    std::string macroValue;

    for (char const** attr = attributes; *attr; attr += 2) {
      if (strcmp(attr[0], "Name") == 0) {
        macroName = attr[1];
      } else if (strcmp(attr[0], "Value") == 0) {
        macroValue = attr[1];
      }
    }

    if (!macroName.empty()) {
      this->Macros[macroName] = macroValue;
    }
  } else if (name == "Directories"_s) {
    for (char const** attr = attributes; *attr; attr += 2) {
      if (strcmp(attr[0], "Include") == 0) {
        this->Include = attr[1];
      } else if (strcmp(attr[0], "Library") == 0) {
        this->Library = attr[1];
      } else if (strcmp(attr[0], "Path") == 0) {
        this->Path = attr[1];
      }
    }
  }
}

void cmVisualStudioWCEPlatformParser::EndElement(std::string const& name)
{
  if (!this->RequiredName) {
    if (name == "PlatformName"_s) {
      this->AvailablePlatforms.push_back(this->CharacterData);
    }
    return;
  }

  if (this->FoundRequiredName) {
    return;
  }

  if (name == "PlatformName"_s) {
    this->PlatformName = this->CharacterData;
  } else if (name == "OSMajorVersion"_s) {
    this->OSMajorVersion = this->CharacterData;
  } else if (name == "OSMinorVersion"_s) {
    this->OSMinorVersion = this->CharacterData;
  } else if (name == "Platform"_s) {
    if (this->PlatformName == this->RequiredName) {
      this->FoundRequiredName = true;
    }
  }
}

void cmVisualStudioWCEPlatformParser::CharacterDataHandler(char const* data,
                                                           int length)
{
  this->CharacterData.append(data, length);
}

std::string cmVisualStudioWCEPlatformParser::FixPaths(
  std::string const& paths) const
{
  std::string ret = paths;
  cmSystemTools::ReplaceString(ret, "$(PATH)", "%PATH%");
  cmSystemTools::ReplaceString(ret, "$(VCInstallDir)", VcInstallDir.c_str());
  cmSystemTools::ReplaceString(ret, "$(VSInstallDir)", VsInstallDir.c_str());
  std::replace(ret.begin(), ret.end(), '\\', '/');
  cmSystemTools::ReplaceString(ret, "//", "/");
  std::replace(ret.begin(), ret.end(), '/', '\\');
  return ret;
}
