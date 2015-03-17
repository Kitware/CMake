/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio9Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmVisualStudioWCEPlatformParser.h"
#include "cmake.h"

static const char vs9generatorName[] = "Visual Studio 9 2008";

class cmGlobalVisualStudio9Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator* CreateGlobalGenerator(
                                              const std::string& name) const {
    if(strncmp(name.c_str(), vs9generatorName,
               sizeof(vs9generatorName) - 1) != 0)
      {
      return 0;
      }

    const char* p = name.c_str() + sizeof(vs9generatorName) - 1;
    if(p[0] == '\0')
      {
      return new cmGlobalVisualStudio9Generator(
        name, "");
      }

    if(p[0] != ' ')
      {
      return 0;
      }

    ++p;

    if(!strcmp(p, "IA64"))
      {
      return new cmGlobalVisualStudio9Generator(
        name, "Itanium");
      }

    if(!strcmp(p, "Win64"))
      {
      return new cmGlobalVisualStudio9Generator(
        name, "x64");
      }

    cmVisualStudioWCEPlatformParser parser(p);
    parser.ParseVersion("9.0");
    if (!parser.Found())
      {
      return 0;
      }

    cmGlobalVisualStudio9Generator* ret = new cmGlobalVisualStudio9Generator(
      name, p);
    ret->WindowsCEVersion = parser.GetOSVersion();
    return ret;
  }

  virtual void GetDocumentation(cmDocumentationEntry& entry) const {
    entry.Name = vs9generatorName;
    entry.Brief = "Generates Visual Studio 9 2008 project files.";
  }

  virtual void GetGenerators(std::vector<std::string>& names) const {
    names.push_back(vs9generatorName);
    names.push_back(vs9generatorName + std::string(" Win64"));
    names.push_back(vs9generatorName + std::string(" IA64"));
    cmVisualStudioWCEPlatformParser parser;
    parser.ParseVersion("9.0");
    const std::vector<std::string>& availablePlatforms =
      parser.GetAvailablePlatforms();
    for(std::vector<std::string>::const_iterator i =
        availablePlatforms.begin(); i != availablePlatforms.end(); ++i)
      {
      names.push_back("Visual Studio 9 2008 " + *i);
      }
  }
};

//----------------------------------------------------------------------------
cmGlobalGeneratorFactory* cmGlobalVisualStudio9Generator::NewFactory()
{
  return new Factory;
}

//----------------------------------------------------------------------------
cmGlobalVisualStudio9Generator::cmGlobalVisualStudio9Generator(
  const std::string& name, const std::string& platformName)
  : cmGlobalVisualStudio8Generator(name, platformName)
{
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio9Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 10.00\n";
  fout << "# Visual Studio 2008\n";
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio9Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg
    = new cmLocalVisualStudio7Generator(cmLocalVisualStudioGenerator::VS9);
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS8());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio9Generator::GetUserMacrosDirectory()
{
  std::string base;
  std::string path;

  // base begins with the VisualStudioProjectsLocation reg value...
  if (cmSystemTools::ReadRegistryValue(
    "HKEY_CURRENT_USER\\Software\\Microsoft\\VisualStudio\\9.0;"
    "VisualStudioProjectsLocation",
    base))
    {
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

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio9Generator::GetUserMacrosRegKeyBase()
{
  return "Software\\Microsoft\\VisualStudio\\9.0\\vsmacros";
}
