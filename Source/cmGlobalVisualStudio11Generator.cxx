/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio11Generator.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"

static const char vs11Win32generatorName[] = "Visual Studio 11";
static const char vs11Win64generatorName[] = "Visual Studio 11 Win64";
static const char vs11WinXP32generatorName[] = "Visual Studio 11 (WinXP)";
static const char vs11WinXP64generatorName[] = "Visual Studio 11 Win64 (WinXP)";
static const char vs11ARMgeneratorName[] = "Visual Studio 11 ARM";

class cmGlobalVisualStudio11Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator* CreateGlobalGenerator(const char* name) const {
    if(!strcmp(name, vs11Win32generatorName))
      {
      return new cmGlobalVisualStudio11Generator(
        vs11Win32generatorName, NULL, NULL, "v110");
      }
    if(!strcmp(name, vs11Win64generatorName))
      {
      return new cmGlobalVisualStudio11Generator(
        vs11Win64generatorName, "x64", "CMAKE_FORCE_WIN64", "v110");
      }
    if(!strcmp(name, vs11WinXP32generatorName))
      {
      return new cmGlobalVisualStudio11Generator(
        vs11Win32generatorName, NULL, NULL, "v110_xp");
      }
    if(!strcmp(name, vs11WinXP64generatorName))
      {
      return new cmGlobalVisualStudio11Generator(
        vs11Win64generatorName, "x64", "CMAKE_FORCE_WIN64", "v110_xp");
      }
    if(!strcmp(name, vs11ARMgeneratorName))
      {
      return new cmGlobalVisualStudio11Generator(
        vs11ARMgeneratorName, "ARM", NULL, "v110");
      }
    return 0;
  }

  virtual void GetDocumentation(cmDocumentationEntry& entry) const {
    entry.Name = "Visual Studio 11";
    entry.Brief = "Generates Visual Studio 11 project files.";
    entry.Full =
      "It is possible to append a space followed by the platform name "
      "to create project files for a specific target platform. E.g. "
      "\"Visual Studio 11 Win64\" will create project files for "
      "the x64 processor; \"Visual Studio 11 ARM\" for ARM.";
  }

  virtual void GetGenerators(std::vector<std::string>& names) const {
    names.push_back(vs11Win32generatorName);
    names.push_back(vs11Win64generatorName);
    names.push_back(vs11WinXP32generatorName);
    names.push_back(vs11WinXP64generatorName);
    names.push_back(vs11ARMgeneratorName); }
};

//----------------------------------------------------------------------------
cmGlobalGeneratorFactory* cmGlobalVisualStudio11Generator::NewFactory()
{
  return new Factory;
}

//----------------------------------------------------------------------------
cmGlobalVisualStudio11Generator::cmGlobalVisualStudio11Generator(
  const char* name, const char* architectureId,
  const char* additionalPlatformDefinition, const char* toolset)
  : cmGlobalVisualStudio10Generator(name, architectureId,
                                   additionalPlatformDefinition)
{
  this->FindMakeProgramFile = "CMakeVS11FindMake.cmake";
  std::string vc11Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\11.0\\Setup\\VC;"
    "ProductDir", vc11Express, cmSystemTools::KeyWOW64_32);
  this->PlatformToolset = toolset;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio11Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
  fout << "# Visual Studio 11\n";
}

//----------------------------------------------------------------------------
cmLocalGenerator *cmGlobalVisualStudio11Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio10Generator* lg =
    new cmLocalVisualStudio10Generator(cmLocalVisualStudioGenerator::VS11);
  lg->SetPlatformName(this->GetPlatformName());
  lg->SetGlobalGenerator(this);
  return lg;
}
