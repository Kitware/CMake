/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio15Generator.h"

#include "cmAlgorithms.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"

static const char vs15generatorName[] = "Visual Studio 15 2017";

// Map generator name without year to name with year.
static const char* cmVS15GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs15generatorName,
              sizeof(vs15generatorName) - 6) != 0) {
    return 0;
  }
  const char* p = name.c_str() + sizeof(vs15generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2017")) {
    p += 5;
  }
  genName = std::string(vs15generatorName) + p;
  return p;
}

class cmGlobalVisualStudio15Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator* CreateGlobalGenerator(const std::string& name,
                                                   cmake* cm) const
  {
    std::string genName;
    const char* p = cmVS15GenName(name, genName);
    if (!p) {
      return 0;
    }
    if (!*p) {
      return new cmGlobalVisualStudio15Generator(cm, genName, "");
    }
    if (*p++ != ' ') {
      return 0;
    }
    if (strcmp(p, "Win64") == 0) {
      return new cmGlobalVisualStudio15Generator(cm, genName, "x64");
    }
    if (strcmp(p, "ARM") == 0) {
      return new cmGlobalVisualStudio15Generator(cm, genName, "ARM");
    }
    return 0;
  }

  virtual void GetDocumentation(cmDocumentationEntry& entry) const
  {
    entry.Name = std::string(vs15generatorName) + " [arch]";
    entry.Brief = "Generates Visual Studio 2016 project files.  "
                  "Optional [arch] can be \"Win64\" or \"ARM\".";
  }

  virtual void GetGenerators(std::vector<std::string>& names) const
  {
    names.push_back(vs15generatorName);
    names.push_back(vs15generatorName + std::string(" ARM"));
    names.push_back(vs15generatorName + std::string(" Win64"));
  }

  virtual bool SupportsToolset() const { return true; }
};

cmGlobalGeneratorFactory* cmGlobalVisualStudio15Generator::NewFactory()
{
  return new Factory;
}

cmGlobalVisualStudio15Generator::cmGlobalVisualStudio15Generator(
  cmake* cm, const std::string& name, const std::string& platformName)
  : cmGlobalVisualStudio14Generator(cm, name, platformName)
{
  std::string vc15Express;
  this->ExpressEdition = false;
  this->DefaultPlatformToolset = "v141";
  this->Version = VS15;
  this->MSBuildCommandInitialized = false;
}

bool cmGlobalVisualStudio15Generator::MatchesGeneratorName(
  const std::string& name) const
{
  std::string genName;
  if (cmVS15GenName(name, genName)) {
    return genName == this->GetName();
  }
  return false;
}

bool cmGlobalVisualStudio15Generator::SelectWindows10SDK(cmMakefile* mf,
                                                         bool required)
{
  // Find the default version of the Windows 10 SDK.
  this->WindowsTargetPlatformVersion = this->GetWindows10SDKVersion();
  if (required && this->WindowsTargetPlatformVersion.empty()) {
    std::ostringstream e;
    e << "Could not find an appropriate version of the Windows 10 SDK"
      << " installed on this machine";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
  }
  mf->AddDefinition("CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION",
                    this->WindowsTargetPlatformVersion.c_str());
  return true;
}

bool cmGlobalVisualStudio15Generator::SelectWindowsStoreToolset(
  std::string& toolset) const
{
  if (cmHasLiteralPrefix(this->SystemVersion, "10.0")) {
    if (this->IsWindowsStoreToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = "v141";
      return true;
    } else {
      return false;
    }
  }
  return this->cmGlobalVisualStudio12Generator::SelectWindowsStoreToolset(
    toolset);
}

void cmGlobalVisualStudio15Generator::WriteSLNHeader(std::ostream& fout)
{
  // Visual Studio 15 writes .sln format 12.00
  fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
  if (this->ExpressEdition) {
    fout << "# Visual Studio Express 15 for Windows Desktop\n";
  } else {
    fout << "# Visual Studio 15\n";
  }
}

bool cmGlobalVisualStudio15Generator::IsWindowsDesktopToolsetInstalled() const
{
  return true;
}

bool cmGlobalVisualStudio15Generator::IsWindowsStoreToolsetInstalled() const
{
  return true;
}

std::string const& cmGlobalVisualStudio15Generator::GetMSBuildCommand()
{
    if (!this->MSBuildCommandInitialized) {
        this->MSBuildCommandInitialized = true;
        this->MSBuildCommand = this->FindMSBuildCommand();
    }
    return this->MSBuildCommand;
}

std::string cmGlobalVisualStudio15Generator::FindMSBuildCommand()
{
    std::string MSBuildLocation = "";
    std::string cmakeCommandLocation = cmSystemTools::GetCMakeCommand();
    // For VS2017, cmake is deployed at <VSRoot>\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
    // Use relative path to find the MSBuild location
    std::transform(cmakeCommandLocation.begin(), cmakeCommandLocation.end(), cmakeCommandLocation.begin(), ::tolower);
    std::size_t found = cmakeCommandLocation.find("/common7/ide/commonextensions/microsoft/cmake/cmake/bin");
    if ( found == std::string::npos)
    {
        return MSBuildLocation;
    }

    std::string VSLocation = cmakeCommandLocation.substr(0, found);
    VSLocation += "/MSBuild/15.0/Bin/MSBuild.exe";

    if (cmSystemTools::FileExists(VSLocation.c_str()) != true)
    {
        return MSBuildLocation;
    }

    MSBuildLocation = VSLocation;
    return MSBuildLocation;
}

std::string cmGlobalVisualStudio15Generator::FindDevEnvCommand()
{
    std::string DevEnvLocation = "";
    std::string cmakeCommandLocation = cmSystemTools::GetCMakeCommand();
    // For VS2017, cmake is deployed at <VSRoot>\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
    // Use relative path to find the Devenv location
    std::transform(cmakeCommandLocation.begin(), cmakeCommandLocation.end(), cmakeCommandLocation.begin(), ::tolower);
    std::size_t found = cmakeCommandLocation.find("/common7/ide/commonextensions/microsoft/cmake/cmake/bin");
    if (found == std::string::npos)
    {
        return DevEnvLocation;
    }

    std::string VSLocation = cmakeCommandLocation.substr(0, found);
    VSLocation += "/Common7/IDE/devenv.exe";

    if (cmSystemTools::FileExists(VSLocation.c_str()) != true)
    {
        return DevEnvLocation;
    }

    DevEnvLocation = VSLocation;
    return DevEnvLocation;
}

void cmGlobalVisualStudio15Generator::FindMakeProgram(cmMakefile* mf)
{
    mf->AddDefinition("CMAKE_VS_MSBUILD_COMMAND",
        this->GetVSMakeProgram().c_str());
}

#if defined(_WIN32) && !defined(__CYGWIN__)
struct NoWindowsH
{
  bool operator()(std::string const& p)
  {
    return !cmSystemTools::FileExists(p + "/um/windows.h", true);
  }
};
#endif

std::string cmGlobalVisualStudio15Generator::GetWindows10SDKVersion()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  // This logic is taken from the vcvarsqueryregistry.bat file from VS2015
  // Try HKLM and then HKCU.
  std::string win10Root;
  if (!cmSystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
        "Windows Kits\\Installed Roots;KitsRoot10",
        win10Root, cmSystemTools::KeyWOW64_32) &&
      !cmSystemTools::ReadRegistryValue(
        "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\"
        "Windows Kits\\Installed Roots;KitsRoot10",
        win10Root, cmSystemTools::KeyWOW64_32)) {
    return std::string();
  }

  std::vector<std::string> sdks;
  std::string path = win10Root + "Include/*";
  // Grab the paths of the different SDKs that are installed
  cmSystemTools::GlobDirs(path, sdks);

  // Skip SDKs that do not contain <um/windows.h> because that indicates that
  // only the UCRT MSIs were installed for them.
  sdks.erase(std::remove_if(sdks.begin(), sdks.end(), NoWindowsH()),
             sdks.end());

  if (!sdks.empty()) {
    // Only use the filename, which will be the SDK version.
    for (std::vector<std::string>::iterator i = sdks.begin(); i != sdks.end();
         ++i) {
      *i = cmSystemTools::GetFilenameName(*i);
    }

    // Sort the results to make sure we select the most recent one.
    std::sort(sdks.begin(), sdks.end(), cmSystemTools::VersionCompareGreater);

    // Look for a SDK exactly matching the requested target version.
    for (std::vector<std::string>::iterator i = sdks.begin(); i != sdks.end();
         ++i) {
      if (cmSystemTools::VersionCompareEqual(*i, this->SystemVersion)) {
        return *i;
      }
    }

    // Use the latest Windows 10 SDK since the exact version is not available.
    return sdks.at(0);
  }
#endif
  // Return an empty string
  return std::string();
}
