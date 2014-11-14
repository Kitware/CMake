/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio14Generator.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"

static const char vs14generatorName[] = "Visual Studio 14 2015";

// Map generator name without year to name with year.
static const char* cmVS14GenName(const std::string& name, std::string& genName)
{
  if(strncmp(name.c_str(), vs14generatorName,
             sizeof(vs14generatorName)-6) != 0)
    {
    return 0;
    }
  const char* p = name.c_str() + sizeof(vs14generatorName) - 6;
  if(cmHasLiteralPrefix(p, " 2015"))
    {
    p += 5;
    }
  genName = std::string(vs14generatorName) + p;
  return p;
}

class cmGlobalVisualStudio14Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator* CreateGlobalGenerator(
                                                const std::string& name) const
    {
    std::string genName;
    const char* p = cmVS14GenName(name, genName);
    if(!p)
      { return 0; }
    if(!*p)
      {
      return new cmGlobalVisualStudio14Generator(
        genName, "");
      }
    if(*p++ != ' ')
      { return 0; }
    if(strcmp(p, "Win64") == 0)
      {
      return new cmGlobalVisualStudio14Generator(
        genName, "x64");
      }
    if(strcmp(p, "ARM") == 0)
      {
      return new cmGlobalVisualStudio14Generator(
        genName, "ARM");
      }
    return 0;
    }

  virtual void GetDocumentation(cmDocumentationEntry& entry) const
    {
    entry.Name = vs14generatorName;
    entry.Brief = "Generates Visual Studio 14 (VS 2015) project files.";
    }

  virtual void GetGenerators(std::vector<std::string>& names) const
    {
    names.push_back(vs14generatorName);
    names.push_back(vs14generatorName + std::string(" ARM"));
    names.push_back(vs14generatorName + std::string(" Win64"));
    }
};

//----------------------------------------------------------------------------
cmGlobalGeneratorFactory* cmGlobalVisualStudio14Generator::NewFactory()
{
  return new Factory;
}

//----------------------------------------------------------------------------
cmGlobalVisualStudio14Generator::cmGlobalVisualStudio14Generator(
  const std::string& name, const std::string& platformName)
  : cmGlobalVisualStudio12Generator(name, platformName)
{
  std::string vc14Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\14.0\\Setup\\VC;"
    "ProductDir", vc14Express, cmSystemTools::KeyWOW64_32);
  this->DefaultPlatformToolset = "v140";
}

//----------------------------------------------------------------------------
bool
cmGlobalVisualStudio14Generator::MatchesGeneratorName(
                                                const std::string& name) const
{
  std::string genName;
  if(cmVS14GenName(name, genName))
    {
    return genName == this->GetName();
    }
  return false;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio14Generator::WriteSLNHeader(std::ostream& fout)
{
  // Visual Studio 14 writes .sln format 12.00
  fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
  if (this->ExpressEdition)
    {
    fout << "# Visual Studio Express 14 for Windows Desktop\n";
    }
  else
    {
    fout << "# Visual Studio 14\n";
    }
}

//----------------------------------------------------------------------------
cmLocalGenerator *cmGlobalVisualStudio14Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio10Generator* lg =
    new cmLocalVisualStudio10Generator(cmLocalVisualStudioGenerator::VS14);
  lg->SetGlobalGenerator(this);
  return lg;
}
