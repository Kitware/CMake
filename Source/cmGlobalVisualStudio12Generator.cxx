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

static const char vs12generatorName[] = "Visual Studio 12 2013";

// Map generator name without year to name with year.
static const char* cmVS12GenName(const char* name, std::string& genName)
{
  if(strncmp(name, vs12generatorName, sizeof(vs12generatorName)-6) != 0)
    {
    return 0;
    }
  const char* p = name + sizeof(vs12generatorName) - 6;
  if(cmHasLiteralPrefix(p, " 2013"))
    {
    p += 5;
    }
  genName = std::string(vs12generatorName) + p;
  return p;
}

class cmGlobalVisualStudio12Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator* CreateGlobalGenerator(const char* name) const
    {
    std::string genName;
    const char* p = cmVS12GenName(name, genName);
    if(!p)
      { return 0; }
    name = genName.c_str();
    if(strcmp(p, "") == 0)
      {
      return new cmGlobalVisualStudio12Generator(
        name, NULL, NULL);
      }
    if(strcmp(p, " Win64") == 0)
      {
      return new cmGlobalVisualStudio12Generator(
        name, "x64", "CMAKE_FORCE_WIN64");
      }
    if(strcmp(p, " ARM") == 0)
      {
      return new cmGlobalVisualStudio12Generator(
        name, "ARM", NULL);
      }
    return 0;
    }

  virtual void GetDocumentation(cmDocumentationEntry& entry) const
    {
    entry.Name = vs12generatorName;
    entry.Brief = "Generates Visual Studio 12 (VS 2013) project files.";
    }

  virtual void GetGenerators(std::vector<std::string>& names) const
    {
    names.push_back(vs12generatorName);
    names.push_back(vs12generatorName + std::string(" ARM"));
    names.push_back(vs12generatorName + std::string(" Win64"));
    }
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
  std::string vc12Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\12.0\\Setup\\VC;"
    "ProductDir", vc12Express, cmSystemTools::KeyWOW64_32);
  this->PlatformToolset = "v120";
}

//----------------------------------------------------------------------------
bool
cmGlobalVisualStudio12Generator::MatchesGeneratorName(const char* name) const
{
  std::string genName;
  if(cmVS12GenName(name, genName))
    {
    return genName == this->GetName();
    }
  return false;
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
