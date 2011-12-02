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
#include "cmGlobalVisualStudio8Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"

//----------------------------------------------------------------------------
cmGlobalVisualStudio8Generator::cmGlobalVisualStudio8Generator()
{
  this->FindMakeProgramFile = "CMakeVS8FindMake.cmake";
  this->ProjectConfigurationSectionName = "ProjectConfigurationPlatforms";
  this->ArchitectureId = "X86";
}

//----------------------------------------------------------------------------
///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio8Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg =
    new cmLocalVisualStudio7Generator(cmLocalVisualStudioGenerator::VS8);
  lg->SetPlatformName(this->GetPlatformName());
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS8());
  lg->SetGlobalGenerator(this);
  return lg;
}
  
//----------------------------------------------------------------------------
// ouput standard header for dsw file
void cmGlobalVisualStudio8Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 9.00\n";
  fout << "# Visual Studio 2005\n";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio .NET 2005 project files.";
  entry.Full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  mf->AddDefinition("MSVC_C_ARCHITECTURE_ID", this->ArchitectureId);
  mf->AddDefinition("MSVC_CXX_ARCHITECTURE_ID", this->ArchitectureId);
  mf->AddDefinition("MSVC80", "1");
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::Configure()
{
  this->cmGlobalVisualStudio7Generator::Configure();
  this->CreateGUID(CMAKE_CHECK_BUILD_SYSTEM_TARGET);
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio8Generator::GetUserMacrosDirectory()
{
  // Some VS8 sp0 versions cannot run macros.
  // See http://support.microsoft.com/kb/928209
  const char* vc8sp1Registry =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\"
    "InstalledProducts\\KB926601;";
  const char* vc8exSP1Registry =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\"
    "InstalledProducts\\KB926748;";
  std::string vc8sp1;
  if (!cmSystemTools::ReadRegistryValue(vc8sp1Registry, vc8sp1) &&
      !cmSystemTools::ReadRegistryValue(vc8exSP1Registry, vc8sp1))
    {
    return "";
    }

  std::string base;
  std::string path;

  // base begins with the VisualStudioProjectsLocation reg value...
  if (cmSystemTools::ReadRegistryValue(
    "HKEY_CURRENT_USER\\Software\\Microsoft\\VisualStudio\\8.0;"
    "VisualStudioProjectsLocation",
    base))
    {
    cmSystemTools::ConvertToUnixSlashes(base);

    // 8.0 macros folder:
    path = base + "/VSMacros80";
    }

  // path is (correctly) still empty if we did not read the base value from
  // the Registry value
  return path;
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio8Generator::GetUserMacrosRegKeyBase()
{
  return "Software\\Microsoft\\VisualStudio\\8.0\\vsmacros";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::AddCheckTarget()
{
  // Add a special target on which all other targets depend that
  // checks the build system and optionally re-runs CMake.
  const char* no_working_directory = 0;
  std::vector<std::string> no_depends;
  std::vector<cmLocalGenerator*> const& generators = this->LocalGenerators;
  cmLocalVisualStudio7Generator* lg =
    static_cast<cmLocalVisualStudio7Generator*>(generators[0]);
  cmMakefile* mf = lg->GetMakefile();

  // Skip the target if no regeneration is to be done.
  if(mf->IsOn("CMAKE_SUPPRESS_REGENERATION"))
    {
    return;
    }

  std::string cmake_command = mf->GetRequiredDefinition("CMAKE_COMMAND");
  cmCustomCommandLines noCommandLines;
  cmTarget* tgt =
    mf->AddUtilityCommand(CMAKE_CHECK_BUILD_SYSTEM_TARGET, false,
                          no_working_directory, no_depends,
                          noCommandLines);

  // Organize in the "predefined targets" folder:
  //
  if (this->UseFolderProperty())
    {
    tgt->SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
    }

  // Create a list of all stamp files for this project.
  std::vector<std::string> stamps;
  std::string stampList = cmake::GetCMakeFilesDirectoryPostSlash();
  stampList += "generate.stamp.list";
  {
  std::string stampListFile =
    generators[0]->GetMakefile()->GetCurrentOutputDirectory();
  stampListFile += "/";
  stampListFile += stampList;
  std::string stampFile;
  cmGeneratedFileStream fout(stampListFile.c_str());
  for(std::vector<cmLocalGenerator*>::const_iterator
        gi = generators.begin(); gi != generators.end(); ++gi)
    {
    stampFile = (*gi)->GetMakefile()->GetCurrentOutputDirectory();
    stampFile += "/";
    stampFile += cmake::GetCMakeFilesDirectoryPostSlash();
    stampFile += "generate.stamp";
    fout << stampFile << "\n";
    stamps.push_back(stampFile);
    }
  }

  // Add a custom rule to re-run CMake if any input files changed.
  {
  // Collect the input files used to generate all targets in this
  // project.
  std::vector<std::string> listFiles;
  for(unsigned int j = 0; j < generators.size(); ++j)
    {
    cmMakefile* lmf = generators[j]->GetMakefile();
    listFiles.insert(listFiles.end(), lmf->GetListFiles().begin(),
                     lmf->GetListFiles().end());
    }
  // Sort the list of input files and remove duplicates.
  std::sort(listFiles.begin(), listFiles.end(),
            std::less<std::string>());
  std::vector<std::string>::iterator new_end =
    std::unique(listFiles.begin(), listFiles.end());
  listFiles.erase(new_end, listFiles.end());

  // Create a rule to re-run CMake.
  std::string stampName = cmake::GetCMakeFilesDirectoryPostSlash();
  stampName += "generate.stamp";
  const char* dsprule = mf->GetRequiredDefinition("CMAKE_COMMAND");
  cmCustomCommandLine commandLine;
  commandLine.push_back(dsprule);
  std::string argH = "-H";
  argH += lg->Convert(mf->GetHomeDirectory(),
                      cmLocalGenerator::START_OUTPUT,
                      cmLocalGenerator::UNCHANGED, true);
  commandLine.push_back(argH);
  std::string argB = "-B";
  argB += lg->Convert(mf->GetHomeOutputDirectory(),
                      cmLocalGenerator::START_OUTPUT,
                      cmLocalGenerator::UNCHANGED, true);
  commandLine.push_back(argB);
  commandLine.push_back("--check-stamp-list");
  commandLine.push_back(stampList.c_str());
  commandLine.push_back("--vs-solution-file");
  commandLine.push_back("\"$(SolutionPath)\"");
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Add the rule.  Note that we cannot use the CMakeLists.txt
  // file as the main dependency because it would get
  // overwritten by the CreateVCProjBuildRule.
  // (this could be avoided with per-target source files)
  const char* no_main_dependency = 0;
  const char* no_working_directory = 0;
  mf->AddCustomCommandToOutput(
    stamps, listFiles,
    no_main_dependency, commandLines, "Checking Build System",
    no_working_directory, true);
  std::string ruleName = stamps[0];
  ruleName += ".rule";
  if(cmSourceFile* file = mf->GetSource(ruleName.c_str()))
    {
    tgt->AddSourceFile(file);
    }
  else
    {
    cmSystemTools::Error("Error adding rule for ", stamps[0].c_str());
    }
  }
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::Generate()
{
  this->AddCheckTarget();

  // All targets depend on the build-system check target.
  for(std::map<cmStdString,cmTarget *>::const_iterator
        ti = this->TotalTargets.begin();
      ti != this->TotalTargets.end(); ++ti)
    {
    if(ti->first != CMAKE_CHECK_BUILD_SYSTEM_TARGET)
      {
      ti->second->AddUtility(CMAKE_CHECK_BUILD_SYSTEM_TARGET);
      }
    }

  // Now perform the main generation.
  this->cmGlobalVisualStudio7Generator::Generate();
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio8Generator
::WriteSolutionConfigurations(std::ostream& fout)
{
  fout << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\t" << *i << "|" << this->GetPlatformName()
         << " = "  << *i << "|" << this->GetPlatformName() << "\n";
    }
  fout << "\tEndGlobalSection\n";
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio8Generator
::WriteProjectConfigurations(std::ostream& fout, const char* name,
                             bool partOfDefaultBuild)
{
  std::string guid = this->GetGUID(name);
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\t{" << guid << "}." << *i
         << "|" << this->GetPlatformName() << ".ActiveCfg = "
         << *i << "|" << this->GetPlatformName() << "\n";
    if(partOfDefaultBuild)
      {
      fout << "\t\t{" << guid << "}." << *i
           << "|" << this->GetPlatformName() << ".Build.0 = "
           << *i << "|" << this->GetPlatformName() << "\n";
      }
    }
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudio8Generator::ComputeTargetDepends()
{
  // Skip over the cmGlobalVisualStudioGenerator implementation!
  // We do not need the support that VS <= 7.1 needs.
  return this->cmGlobalGenerator::ComputeTargetDepends();
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::WriteProjectDepends(
  std::ostream& fout, const char*, const char*, cmTarget& t)
{
  TargetDependSet const& unordered = this->GetTargetDirectDepends(t);
  OrderedTargetDependSet depends(unordered);
  for(OrderedTargetDependSet::const_iterator i = depends.begin();
      i != depends.end(); ++i)
    {
    std::string guid = this->GetGUID((*i)->GetName());
    fout << "\t\t{" << guid << "} = {" << guid << "}\n";
    }
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudio8Generator::NeedLinkLibraryDependencies(
  cmTarget& target)
{
  // Look for utility dependencies that magically link.
  for(std::set<cmStdString>::const_iterator ui =
        target.GetUtilities().begin();
      ui != target.GetUtilities().end(); ++ui)
    {
    if(cmTarget* depTarget = this->FindTarget(0, ui->c_str()))
      {
      if(depTarget->GetProperty("EXTERNAL_MSPROJECT"))
        {
        // This utility dependency names an external .vcproj target.
        // We use LinkLibraryDependencies="true" to link to it without
        // predicting the .lib file location or name.
        return true;
        }
      }
    }
  return false;
}

//----------------------------------------------------------------------------
static cmVS7FlagTable cmVS8ExtraFlagTable[] =
{ 
  {"CallingConvention", "Gd", "cdecl", "0", 0 },
  {"CallingConvention", "Gr", "fastcall", "1", 0 },
  {"CallingConvention", "Gz", "stdcall", "2", 0 },

  {"Detect64BitPortabilityProblems", "Wp64",
   "Detect 64Bit Portability Problems", "true", 0 },
  {"ErrorReporting", "errorReport:prompt", "Report immediately", "1", 0 },
  {"ErrorReporting", "errorReport:queue", "Queue for next login", "2", 0 },
  // Precompiled header and related options.  Note that the
  // UsePrecompiledHeader entries are marked as "Continue" so that the
  // corresponding PrecompiledHeaderThrough entry can be found.
  {"UsePrecompiledHeader", "Yu", "Use Precompiled Header", "2",
   cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue},
  {"PrecompiledHeaderThrough", "Yu", "Precompiled Header Name", "",
   cmVS7FlagTable::UserValueRequired},
  // There is no YX option in the VS8 IDE.

  // Exception handling mode.  If no entries match, it will be FALSE.
  {"ExceptionHandling", "GX", "enable c++ exceptions", "1", 0},
  {"ExceptionHandling", "EHsc", "enable c++ exceptions", "1", 0},
  {"ExceptionHandling", "EHa", "enable SEH exceptions", "2", 0},

  {"EnablePREfast", "analyze", "", "true", 0},
  {"EnablePREfast", "analyze-", "", "false", 0},

  // Language options
  {"TreatWChar_tAsBuiltInType", "Zc:wchar_t",
   "wchar_t is a built-in type", "true", 0},
  {"TreatWChar_tAsBuiltInType", "Zc:wchar_t-",
   "wchar_t is not a built-in type", "false", 0},

  {0,0,0,0,0}
};
cmIDEFlagTable const* cmGlobalVisualStudio8Generator::GetExtraFlagTableVS8()
{
  return cmVS8ExtraFlagTable;
}
