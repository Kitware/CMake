/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio15Generator.h"

#include "cmAlgorithms.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"

static const char vs15generatorName[] = "Visual Studio 15";

// Map generator name without year to name with year.
static const char* cmVS15GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs15generatorName,
              sizeof(vs15generatorName) - 1) != 0) {
    return 0;
  }
  const char* p = name.c_str() + sizeof(vs15generatorName) - 1;
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
    entry.Brief = "Generates Visual Studio 15 project files.  "
                  "Optional [arch] can be \"Win64\" or \"ARM\".";
  }

  virtual void GetGenerators(std::vector<std::string>& names) const
  {
    names.push_back(vs15generatorName);
    names.push_back(vs15generatorName + std::string(" ARM"));
    names.push_back(vs15generatorName + std::string(" Win64"));
  }

  bool SupportsToolset() const CM_OVERRIDE { return true; }
  bool SupportsPlatform() const CM_OVERRIDE { return true; }
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
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\15.0\\Setup\\VC;"
    "ProductDir",
    vc15Express, cmSystemTools::KeyWOW64_32);
  this->DefaultPlatformToolset = "v140";
  this->Version = VS15;
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

bool cmGlobalVisualStudio15Generator::SelectWindowsStoreToolset(
  std::string& toolset) const
{
  if (cmHasLiteralPrefix(this->SystemVersion, "10.0")) {
    if (this->IsWindowsStoreToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = "v140"; // VS 15 uses v140 toolset
      return true;
    } else {
      return false;
    }
  }
  return this->cmGlobalVisualStudio14Generator::SelectWindowsStoreToolset(
    toolset);
}

bool cmGlobalVisualStudio15Generator::IsWindowsDesktopToolsetInstalled() const
{
  const char desktop10Key[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
                              "VisualStudio\\15.0\\VC\\Runtimes";

  std::vector<std::string> vc15;
  return cmSystemTools::GetRegistrySubKeys(desktop10Key, vc15,
                                           cmSystemTools::KeyWOW64_32);
}

bool cmGlobalVisualStudio15Generator::IsWindowsStoreToolsetInstalled() const
{
  const char universal10Key[] =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
    "VisualStudio\\15.0\\Setup\\Build Tools for Windows 10;SrcPath";

  std::string win10SDK;
  return cmSystemTools::ReadRegistryValue(universal10Key, win10SDK,
                                          cmSystemTools::KeyWOW64_32);
}
