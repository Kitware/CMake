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
#include "cmGlobalVisualStudio10Generator.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmVisualStudioSlnData.h"
#include "cmVisualStudioSlnParser.h"
#include "cmake.h"

static const char vs10Win32generatorName[] = "Visual Studio 10";
static const char vs10Win64generatorName[] = "Visual Studio 10 Win64";
static const char vs10IA64generatorName[] = "Visual Studio 10 IA64";

class cmGlobalVisualStudio10Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  virtual cmGlobalGenerator* CreateGlobalGenerator(const char* name) const {
    if(!strcmp(name, vs10Win32generatorName))
      {
      return new cmGlobalVisualStudio10Generator(
        vs10Win32generatorName, NULL, NULL);
      }
    if(!strcmp(name, vs10Win64generatorName))
      {
      return new cmGlobalVisualStudio10Generator(
        vs10Win64generatorName, "x64", "CMAKE_FORCE_WIN64");
      }
    if(!strcmp(name, vs10IA64generatorName))
      {
      return new cmGlobalVisualStudio10Generator(
        vs10IA64generatorName, "Itanium", "CMAKE_FORCE_IA64");
      }
    return 0;
  }

  virtual void GetDocumentation(cmDocumentationEntry& entry) const {
    entry.Name = "Visual Studio 10";
    entry.Brief = "Generates Visual Studio 10 (Visual Studio 2010)\
                  project files.";
    entry.Full =
      "It is possible to append a space followed by the platform name "
      "to create project files for a specific target platform. E.g. "
      "\"Visual Studio 10 Win64\" will create project files for "
      "the x64 processor; \"Visual Studio 10 IA64\" for Itanium.";
  }

  virtual void GetGenerators(std::vector<std::string>& names) const {
    names.push_back(vs10Win32generatorName);
    names.push_back(vs10Win64generatorName);
    names.push_back(vs10IA64generatorName); }
};

//----------------------------------------------------------------------------
cmGlobalGeneratorFactory* cmGlobalVisualStudio10Generator::NewFactory()
{
  return new Factory;
}

//----------------------------------------------------------------------------
cmGlobalVisualStudio10Generator::cmGlobalVisualStudio10Generator(
  const char* name, const char* architectureId,
  const char* additionalPlatformDefinition)
  : cmGlobalVisualStudio8Generator(name, architectureId,
                                   additionalPlatformDefinition)
{
  this->FindMakeProgramFile = "CMakeVS10FindMake.cmake";
  std::string vc10Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\10.0\\Setup\\VC;"
    "ProductDir", vc10Express, cmSystemTools::KeyWOW64_32);
}

//----------------------------------------------------------------------------
bool
cmGlobalVisualStudio10Generator::SetGeneratorToolset(std::string const& ts)
{
  this->PlatformToolset = ts;
  return true;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  cmGlobalVisualStudio8Generator::AddPlatformDefinitions(mf);
  if(!this->PlatformToolset.empty())
    {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET",
                      this->PlatformToolset.c_str());
    }
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 11.00\n";
  if (this->ExpressEdition)
    {
    fout << "# Visual C++ Express 2010\n";
    }
  else
    {
    fout << "# Visual Studio 2010\n";
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio10Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio10Generator* lg =
    new cmLocalVisualStudio10Generator(cmLocalVisualStudioGenerator::VS10);
  lg->SetPlatformName(this->GetPlatformName());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Generator::Generate()
{
  this->LongestSource = LongestSourcePath();
  this->cmGlobalVisualStudio8Generator::Generate();
  if(this->LongestSource.Length > 0)
    {
    cmMakefile* mf = this->LongestSource.Target->GetMakefile();
    cmOStringStream e;
    e <<
      "The binary and/or source directory paths may be too long to generate "
      "Visual Studio 10 files for this project.  "
      "Consider choosing shorter directory names to build this project with "
      "Visual Studio 10.  "
      "A more detailed explanation follows."
      "\n"
      "There is a bug in the VS 10 IDE that renders property dialog fields "
      "blank for files referenced by full path in the project file.  "
      "However, CMake must reference at least one file by full path:\n"
      "  " << this->LongestSource.SourceFile->GetFullPath() << "\n"
      "This is because some Visual Studio tools would append the relative "
      "path to the end of the referencing directory path, as in:\n"
      "  " << mf->GetCurrentOutputDirectory() << "/"
      << this->LongestSource.SourceRel << "\n"
      "and then incorrectly complain that the file does not exist because "
      "the path length is too long for some internal buffer or API.  "
      "To avoid this problem CMake must use a full path for this file "
      "which then triggers the VS 10 property dialog bug.";
    mf->IssueMessage(cmake::WARNING, e.str().c_str());
    }
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Generator
::EnableLanguage(std::vector<std::string>const &  lang,
                 cmMakefile *mf, bool optional)
{
  if(this->ArchitectureId == "Itanium" || this->ArchitectureId == "x64")
    {
    if(this->IsExpressEdition() && !this->Find64BitTools(mf))
      {
      return;
      }
    }
  cmGlobalVisualStudio8Generator::EnableLanguage(lang, mf, optional);
}

//----------------------------------------------------------------------------
const char* cmGlobalVisualStudio10Generator::GetPlatformToolset()
{
  if(!this->PlatformToolset.empty())
    {
    return this->PlatformToolset.c_str();
    }
  return 0;
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio10Generator::GetUserMacrosDirectory()
{
  std::string base;
  std::string path;

  // base begins with the VisualStudioProjectsLocation reg value...
  if (cmSystemTools::ReadRegistryValue(
    "HKEY_CURRENT_USER\\Software\\Microsoft\\VisualStudio\\10.0;"
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
std::string cmGlobalVisualStudio10Generator::GetUserMacrosRegKeyBase()
{
  return "Software\\Microsoft\\VisualStudio\\10.0\\vsmacros";
}


std::string cmGlobalVisualStudio10Generator
::GenerateBuildCommand(const char* makeProgram,
                       const char *projectName, const char *projectDir,
                       const char* additionalOptions, const char *targetName,
                       const char* config, bool ignoreErrors, bool fast)
{
  // now build the test
  std::string makeCommand
    = cmSystemTools::ConvertToOutputPath(makeProgram);
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  // If makeProgram is devenv, parent class knows how to generate command:
  if (lowerCaseCommand.find("devenv") != std::string::npos ||
      lowerCaseCommand.find("VCExpress") != std::string::npos)
    {
    return cmGlobalVisualStudio7Generator::GenerateBuildCommand(makeProgram,
      projectName, projectDir, additionalOptions, targetName, config,
      ignoreErrors, fast);
    }

  // Otherwise, assume MSBuild command line, and construct accordingly.

  // if there are spaces in the makeCommand, assume a full path
  // and convert it to a path with no spaces in it as the
  // RunSingleCommand does not like spaces
  if(makeCommand.find(' ') != std::string::npos)
    {
    cmSystemTools::GetShortPath(makeCommand.c_str(), makeCommand);
    }
  // msbuild.exe CxxOnly.sln /t:Build /p:Configuration=Debug /target:ALL_BUILD
  if(!targetName || strlen(targetName) == 0)
    {
    targetName = "ALL_BUILD";
    }
  bool clean = false;
  if ( targetName && strcmp(targetName, "clean") == 0 )
    {
    clean = true;
    makeCommand += " ";
    makeCommand += projectName;
    makeCommand += ".sln ";
    makeCommand += "/t:Clean ";
    }
  else
    {
    std::string targetProject(targetName);
    targetProject += ".vcxproj";
    if (targetProject.find('/') == std::string::npos)
      {
      // it might be in a subdir
      cmVisualStudioSlnParser parser;
      cmSlnData slnData;
      std::string slnFile;
      if (projectDir && *projectDir)
        {
        slnFile = projectDir;
        slnFile += '/';
        slnFile += projectName;
        }
      else
        {
        slnFile = projectName;
        }
      if (parser.ParseFile(slnFile + ".sln", slnData,
                           cmVisualStudioSlnParser::DataGroupProjects))
        {
        if (cmSlnProjectEntry const* proj =
            slnData.GetProjectByName(targetName))
          {
          targetProject = proj->GetRelativePath();
          cmSystemTools::ConvertToUnixSlashes(targetProject);
          }
        }
      }
    makeCommand += " ";
    makeCommand += targetProject;
    makeCommand += " ";
    }
  makeCommand += "/p:Configuration=";
  if(config && strlen(config))
    {
    makeCommand += config;
    }
  else
    {
    makeCommand += "Debug";
    }
  makeCommand += " /p:VisualStudioVersion=";
  makeCommand += this->GetIDEVersion();
  if ( additionalOptions )
    {
    makeCommand += " ";
    makeCommand += additionalOptions;
    }
  return makeCommand;
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudio10Generator::Find64BitTools(cmMakefile* mf)
{
  if(!this->PlatformToolset.empty())
    {
    return true;
    }
  // This edition does not come with 64-bit tools.  Look for them.
  //
  // TODO: Detect available tools?  x64\v100 exists but does not work?
  // KHLM\\SOFTWARE\\Microsoft\\MSBuild\\ToolsVersions\\4.0;VCTargetsPath
  // c:/Program Files (x86)/MSBuild/Microsoft.Cpp/v4.0/Platforms/
  //   {Itanium,Win32,x64}/PlatformToolsets/{v100,v90,Windows7.1SDK}
  std::string winSDK_7_1;
  if(cmSystemTools::ReadRegistryValue(
       "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\"
       "Windows\\v7.1;InstallationFolder", winSDK_7_1))
    {
    cmOStringStream m;
    m << "Found Windows SDK v7.1: " << winSDK_7_1;
    mf->DisplayStatus(m.str().c_str(), -1);
    this->PlatformToolset = "Windows7.1SDK";
    return true;
    }
  else
    {
    cmOStringStream e;
    e << "Cannot enable 64-bit tools with Visual Studio 2010 Express.\n"
      << "Install the Microsoft Windows SDK v7.1 to get 64-bit tools:\n"
      << "  http://msdn.microsoft.com/en-us/windows/bb980924.aspx";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }
}

//----------------------------------------------------------------------------
std::string
cmGlobalVisualStudio10Generator
::GenerateRuleFile(std::string const& output) const
{
  // The VS 10 generator needs to create the .rule files on disk.
  // Hide them away under the CMakeFiles directory.
  std::string ruleDir = this->GetCMakeInstance()->GetHomeOutputDirectory();
  ruleDir += cmake::GetCMakeFilesDirectory();
  ruleDir += "/";
  ruleDir += cmSystemTools::ComputeStringMD5(
    cmSystemTools::GetFilenamePath(output).c_str());
  std::string ruleFile = ruleDir + "/";
  ruleFile += cmSystemTools::GetFilenameName(output);
  ruleFile += ".rule";
  return ruleFile;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Generator::PathTooLong(
  cmTarget* target, cmSourceFile* sf, std::string const& sfRel)
{
  size_t len = (strlen(target->GetMakefile()->GetCurrentOutputDirectory()) +
                1 + sfRel.length());
  if(len > this->LongestSource.Length)
    {
    this->LongestSource.Length = len;
    this->LongestSource.Target = target;
    this->LongestSource.SourceFile = sf;
    this->LongestSource.SourceRel = sfRel;
    }
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudio10Generator::UseFolderProperty()
{
  return IsExpressEdition() ? false : cmGlobalGenerator::UseFolderProperty();
}
