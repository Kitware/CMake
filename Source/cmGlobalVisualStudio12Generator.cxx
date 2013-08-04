/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio12Generator.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"

static const char vs12Win32generatorName[] = "Visual Studio 12";
static const char vs12Win64generatorName[] = "Visual Studio 12 Win64";
static const char vs12ARMgeneratorName[] = "Visual Studio 12 ARM";

class cmGlobalVisualStudio12Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator* CreateGlobalGenerator(const char* name) const {
    if(!strcmp(name, vs12Win32generatorName))
      {
      return new cmGlobalVisualStudio12Generator(
        name, NULL, NULL);
      }
    if(!strcmp(name, vs12Win64generatorName))
      {
      return new cmGlobalVisualStudio12Generator(
        name, "x64", "CMAKE_FORCE_WIN64");
      }
    if(!strcmp(name, vs12ARMgeneratorName))
      {
      return new cmGlobalVisualStudio12Generator(
        name, "ARM", NULL);
      }
    return 0;
  }

  virtual void GetDocumentation(cmDocumentationEntry& entry) const {
    entry.Name = "Visual Studio 12";
    entry.Brief = "Generates Visual Studio 12 (2013) project files.";
    entry.Full =
      "It is possible to append a space followed by the platform name "
      "to create project files for a specific target platform. E.g. "
      "\"Visual Studio 12 Win64\" will create project files for "
      "the x64 processor; \"Visual Studio 12 ARM\" for ARM.";
  }

  virtual void GetGenerators(std::vector<std::string>& names) const {
    names.push_back(vs12Win32generatorName);
    names.push_back(vs12Win64generatorName);
    names.push_back(vs12ARMgeneratorName); }
};

//----------------------------------------------------------------------------
cmGlobalGeneratorFactory* cmGlobalVisualStudio12Generator::NewFactory()
{
  return new Factory;
}

//----------------------------------------------------------------------------
cmGlobalVisualStudio12Generator::cmGlobalVisualStudio12Generator(
  const char* name, const char* platformName,
  const char* additionalPlatformDefinition)
  : cmGlobalVisualStudio11Generator(name, platformName,
                                   additionalPlatformDefinition)
{
  this->FindMakeProgramFile = "CMakeVS12FindMake.cmake";
  std::string vc12Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\12.0\\Setup\\VC;"
    "ProductDir", vc12Express, cmSystemTools::KeyWOW64_32);
  this->PlatformToolset = "v120";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio12Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
  if (this->ExpressEdition)
    {
    fout << "# Visual Studio Express 2013 for Windows Desktop\n";
    }
  else
    {
    fout << "# Visual Studio 2013\n";
    }
}

//----------------------------------------------------------------------------
cmLocalGenerator *cmGlobalVisualStudio12Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio10Generator* lg =
    new cmLocalVisualStudio10Generator(cmLocalVisualStudioGenerator::VS12);
  lg->SetPlatformName(this->GetPlatformName());
  lg->SetGlobalGenerator(this);
  return lg;
}
