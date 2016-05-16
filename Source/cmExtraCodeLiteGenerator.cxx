/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2013 Eran Ifrah (eran.ifrah@gmail.com)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExtraCodeLiteGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmake.h"

#include "cmStandardIncludes.h"
#include "cmXMLWriter.h"
#include <cmsys/Directory.hxx>
#include <cmsys/SystemInformation.hxx>
#include <cmsys/SystemTools.hxx>

void cmExtraCodeLiteGenerator::GetDocumentation(cmDocumentationEntry& entry,
                                                const std::string&) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates CodeLite project files.";
}

cmExtraCodeLiteGenerator::cmExtraCodeLiteGenerator()
  : cmExternalMakefileProjectGenerator()
  , ConfigName("NoConfig")
  , CpuCount(2)
{
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Ninja");
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}

void cmExtraCodeLiteGenerator::Generate()
{
  // Hold root tree information for creating the workspace
  std::string workspaceProjectName;
  std::string workspaceOutputDir;
  std::string workspaceFileName;
  std::string workspaceSourcePath;

  const std::map<std::string, std::vector<cmLocalGenerator*> >& projectMap =
    this->GlobalGenerator->GetProjectMap();

  // loop projects and locate the root project.
  // and extract the information for creating the worspace
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
         it = projectMap.begin();
       it != projectMap.end(); ++it) {
    const cmMakefile* mf = it->second[0]->GetMakefile();
    this->ConfigName = GetConfigurationName(mf);

    if (strcmp(it->second[0]->GetCurrentBinaryDirectory(),
               it->second[0]->GetBinaryDirectory()) == 0) {
      workspaceOutputDir = it->second[0]->GetCurrentBinaryDirectory();
      workspaceProjectName = it->second[0]->GetProjectName();
      workspaceSourcePath = it->second[0]->GetSourceDirectory();
      workspaceFileName = workspaceOutputDir + "/";
      workspaceFileName += workspaceProjectName + ".workspace";
      this->WorkspacePath = it->second[0]->GetCurrentBinaryDirectory();
      ;
      break;
    }
  }

  cmGeneratedFileStream fout(workspaceFileName.c_str());
  cmXMLWriter xml(fout);

  xml.StartDocument("utf-8");
  xml.StartElement("CodeLite_Workspace");
  xml.Attribute("Name", workspaceProjectName);

  // for each sub project in the workspace create a codelite project
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
         it = projectMap.begin();
       it != projectMap.end(); ++it) {
    // retrive project information
    std::string outputDir = it->second[0]->GetCurrentBinaryDirectory();
    std::string projectName = it->second[0]->GetProjectName();
    std::string filename = outputDir + "/" + projectName + ".project";

    // Make the project file relative to the workspace
    filename = cmSystemTools::RelativePath(this->WorkspacePath.c_str(),
                                           filename.c_str());

    // create a project file
    this->CreateProjectFile(it->second);
    xml.StartElement("Project");
    xml.Attribute("Name", projectName);
    xml.Attribute("Path", filename);
    xml.Attribute("Active", "No");
    xml.EndElement();
  }

  xml.StartElement("BuildMatrix");
  xml.StartElement("WorkspaceConfiguration");
  xml.Attribute("Name", this->ConfigName);
  xml.Attribute("Selected", "yes");

  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
         it = projectMap.begin();
       it != projectMap.end(); ++it) {
    // retrive project information
    std::string projectName = it->second[0]->GetProjectName();

    xml.StartElement("Project");
    xml.Attribute("Name", projectName);
    xml.Attribute("ConfigName", this->ConfigName);
    xml.EndElement();
  }

  xml.EndElement(); // WorkspaceConfiguration
  xml.EndElement(); // BuildMatrix
  xml.EndElement(); // CodeLite_Workspace
}

/* create the project file */
void cmExtraCodeLiteGenerator::CreateProjectFile(
  const std::vector<cmLocalGenerator*>& lgs)
{
  std::string outputDir = lgs[0]->GetCurrentBinaryDirectory();
  std::string projectName = lgs[0]->GetProjectName();
  std::string filename = outputDir + "/";

  filename += projectName + ".project";
  this->CreateNewProjectFile(lgs, filename);
}

void cmExtraCodeLiteGenerator::CreateNewProjectFile(
  const std::vector<cmLocalGenerator*>& lgs, const std::string& filename)
{
  const cmMakefile* mf = lgs[0]->GetMakefile();
  cmGeneratedFileStream fout(filename.c_str());
  if (!fout) {
    return;
  }
  cmXMLWriter xml(fout);

  ////////////////////////////////////
  xml.StartDocument("utf-8");
  xml.StartElement("CodeLite_Project");
  xml.Attribute("Name", lgs[0]->GetProjectName());
  xml.Attribute("InternalType", "");

  // Collect all used source files in the project
  // Sort them into two containers, one for C/C++ implementation files
  // which may have an acompanying header, one for all other files
  std::string projectType;

  std::vector<std::string> srcExts =
    this->GlobalGenerator->GetCMakeInstance()->GetSourceExtensions();
  std::vector<std::string> headerExts =
    this->GlobalGenerator->GetCMakeInstance()->GetHeaderExtensions();

  std::map<std::string, cmSourceFile*> cFiles;
  std::set<std::string> otherFiles;
  for (std::vector<cmLocalGenerator*>::const_iterator lg = lgs.begin();
       lg != lgs.end(); lg++) {
    cmMakefile* makefile = (*lg)->GetMakefile();
    std::vector<cmGeneratorTarget*> targets = (*lg)->GetGeneratorTargets();
    for (std::vector<cmGeneratorTarget*>::iterator ti = targets.begin();
         ti != targets.end(); ti++) {

      switch ((*ti)->GetType()) {
        case cmState::EXECUTABLE: {
          projectType = "Executable";
        } break;
        case cmState::STATIC_LIBRARY: {
          projectType = "Static Library";
        } break;
        case cmState::SHARED_LIBRARY: {
          projectType = "Dynamic Library";
        } break;
        case cmState::MODULE_LIBRARY: {
          projectType = "Dynamic Library";
        } break;
        default: // intended fallthrough
          break;
      }

      switch ((*ti)->GetType()) {
        case cmState::EXECUTABLE:
        case cmState::STATIC_LIBRARY:
        case cmState::SHARED_LIBRARY:
        case cmState::MODULE_LIBRARY: {
          std::vector<cmSourceFile*> sources;
          cmGeneratorTarget* gt = *ti;
          gt->GetSourceFiles(sources,
                             makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
          for (std::vector<cmSourceFile*>::const_iterator si = sources.begin();
               si != sources.end(); si++) {
            // check whether it is a C/C++ implementation file
            bool isCFile = false;
            std::string lang = (*si)->GetLanguage();
            if (lang == "C" || lang == "CXX") {
              std::string srcext = (*si)->GetExtension();
              for (std::vector<std::string>::const_iterator ext =
                     srcExts.begin();
                   ext != srcExts.end(); ++ext) {
                if (srcext == *ext) {
                  isCFile = true;
                  break;
                }
              }
            }

            // then put it accordingly into one of the two containers
            if (isCFile) {
              cFiles[(*si)->GetFullPath()] = *si;
            } else {
              otherFiles.insert((*si)->GetFullPath());
            }
          }
        }
        default: // intended fallthrough
          break;
      }
    }
  }

  // The following loop tries to add header files matching to implementation
  // files to the project. It does that by iterating over all source files,
  // replacing the file name extension with ".h" and checks whether such a
  // file exists. If it does, it is inserted into the map of files.
  // A very similar version of that code exists also in the kdevelop
  // project generator.
  for (std::map<std::string, cmSourceFile*>::const_iterator sit =
         cFiles.begin();
       sit != cFiles.end(); ++sit) {
    std::string headerBasename = cmSystemTools::GetFilenamePath(sit->first);
    headerBasename += "/";
    headerBasename += cmSystemTools::GetFilenameWithoutExtension(sit->first);

    // check if there's a matching header around
    for (std::vector<std::string>::const_iterator ext = headerExts.begin();
         ext != headerExts.end(); ++ext) {
      std::string hname = headerBasename;
      hname += ".";
      hname += *ext;
      // if it's already in the set, don't check if it exists on disk
      std::set<std::string>::const_iterator headerIt = otherFiles.find(hname);
      if (headerIt != otherFiles.end()) {
        break;
      }

      if (cmSystemTools::FileExists(hname.c_str())) {
        otherFiles.insert(hname);
        break;
      }
    }
  }

  // Get the project path ( we need it later to convert files to
  // their relative path)
  std::string projectPath = cmSystemTools::GetFilenamePath(filename);

  // Create 2 virtual folders: src and include
  // and place all the implementation files into the src
  // folder, the rest goes to the include folder
  xml.StartElement("VirtualDirectory");
  xml.Attribute("Name", "src");

  // insert all source files in the codelite project
  // first the C/C++ implementation files, then all others
  for (std::map<std::string, cmSourceFile*>::const_iterator sit =
         cFiles.begin();
       sit != cFiles.end(); ++sit) {
    xml.StartElement("File");
    xml.Attribute("Name", cmSystemTools::RelativePath(projectPath.c_str(),
                                                      sit->first.c_str()));
    xml.EndElement();
  }
  xml.EndElement(); // VirtualDirectory
  xml.StartElement("VirtualDirectory");
  xml.Attribute("Name", "include");
  for (std::set<std::string>::const_iterator sit = otherFiles.begin();
       sit != otherFiles.end(); ++sit) {
    xml.StartElement("File");
    xml.Attribute(
      "Name", cmSystemTools::RelativePath(projectPath.c_str(), sit->c_str()));
    xml.EndElement();
  }
  xml.EndElement(); // VirtualDirectory

  // Get the number of CPUs. We use this information for the make -jN
  // command
  cmsys::SystemInformation info;
  info.RunCPUCheck();

  this->CpuCount =
    info.GetNumberOfLogicalCPU() * info.GetNumberOfPhysicalCPU();

  std::string codeliteCompilerName = this->GetCodeLiteCompilerName(mf);

  xml.StartElement("Settings");
  xml.Attribute("Type", projectType);

  xml.StartElement("Configuration");
  xml.Attribute("Name", this->ConfigName);
  xml.Attribute("CompilerType", this->GetCodeLiteCompilerName(mf));
  xml.Attribute("DebuggerType", "GNU gdb debugger");
  xml.Attribute("Type", projectType);
  xml.Attribute("BuildCmpWithGlobalSettings", "append");
  xml.Attribute("BuildLnkWithGlobalSettings", "append");
  xml.Attribute("BuildResWithGlobalSettings", "append");

  xml.StartElement("Compiler");
  xml.Attribute("Options", "-g");
  xml.Attribute("Required", "yes");
  xml.Attribute("PreCompiledHeader", "");
  xml.StartElement("IncludePath");
  xml.Attribute("Value", ".");
  xml.EndElement(); // IncludePath
  xml.EndElement(); // Compiler

  xml.StartElement("Linker");
  xml.Attribute("Options", "");
  xml.Attribute("Required", "yes");
  xml.EndElement(); // Linker

  xml.StartElement("ResourceCompiler");
  xml.Attribute("Options", "");
  xml.Attribute("Required", "no");
  xml.EndElement(); // ResourceCompiler

  xml.StartElement("General");
  xml.Attribute("OutputFile", "$(IntermediateDirectory)/$(ProjectName)");
  xml.Attribute("IntermediateDirectory", "./");
  xml.Attribute("Command", "./$(ProjectName)");
  xml.Attribute("CommandArguments", "");
  xml.Attribute("WorkingDirectory", "$(IntermediateDirectory)");
  xml.Attribute("PauseExecWhenProcTerminates", "yes");
  xml.EndElement(); // General

  xml.StartElement("Debugger");
  xml.Attribute("IsRemote", "no");
  xml.Attribute("RemoteHostName", "");
  xml.Attribute("RemoteHostPort", "");
  xml.Attribute("DebuggerPath", "");
  xml.Element("PostConnectCommands");
  xml.Element("StartupCommands");
  xml.EndElement(); // Debugger

  xml.Element("PreBuild");
  xml.Element("PostBuild");

  xml.StartElement("CustomBuild");
  xml.Attribute("Enabled", "yes");
  xml.Element("RebuildCommand", GetRebuildCommand(mf));
  xml.Element("CleanCommand", GetCleanCommand(mf));
  xml.Element("BuildCommand", GetBuildCommand(mf));
  xml.Element("SingleFileCommand", GetSingleFileBuildCommand(mf));
  xml.Element("PreprocessFileCommand");
  xml.Element("WorkingDirectory", "$(WorkspacePath)");
  xml.EndElement(); // CustomBuild

  xml.StartElement("AdditionalRules");
  xml.Element("CustomPostBuild");
  xml.Element("CustomPreBuild");
  xml.EndElement(); // AdditionalRules

  xml.EndElement(); // Configuration
  xml.StartElement("GlobalSettings");

  xml.StartElement("Compiler");
  xml.Attribute("Options", "");
  xml.StartElement("IncludePath");
  xml.Attribute("Value", ".");
  xml.EndElement(); // IncludePath
  xml.EndElement(); // Compiler

  xml.StartElement("Linker");
  xml.Attribute("Options", "");
  xml.StartElement("LibraryPath");
  xml.Attribute("Value", ".");
  xml.EndElement(); // LibraryPath
  xml.EndElement(); // Linker

  xml.StartElement("ResourceCompiler");
  xml.Attribute("Options", "");
  xml.EndElement(); // ResourceCompiler

  xml.EndElement(); // GlobalSettings
  xml.EndElement(); // Settings
  xml.EndElement(); // CodeLite_Project
}

std::string cmExtraCodeLiteGenerator::GetCodeLiteCompilerName(
  const cmMakefile* mf) const
{
  // figure out which language to use
  // for now care only for C and C++
  std::string compilerIdVar = "CMAKE_CXX_COMPILER_ID";
  if (this->GlobalGenerator->GetLanguageEnabled("CXX") == false) {
    compilerIdVar = "CMAKE_C_COMPILER_ID";
  }

  std::string compilerId = mf->GetSafeDefinition(compilerIdVar);
  std::string compiler = "gnu g++"; // default to g++

  // Since we need the compiler for parsing purposes only
  // it does not matter if we use clang or clang++, same as
  // "gnu gcc" vs "gnu g++"
  if (compilerId == "MSVC") {
    compiler = "VC++";
  } else if (compilerId == "Clang") {
    compiler = "clang++";
  } else if (compilerId == "GNU") {
    compiler = "gnu g++";
  }
  return compiler;
}

std::string cmExtraCodeLiteGenerator::GetConfigurationName(
  const cmMakefile* mf) const
{
  std::string confName = mf->GetSafeDefinition("CMAKE_BUILD_TYPE");
  // Trim the configuration name from whitespaces (left and right)
  confName.erase(0, confName.find_first_not_of(" \t\r\v\n"));
  confName.erase(confName.find_last_not_of(" \t\r\v\n") + 1);
  if (confName.empty()) {
    confName = "NoConfig";
  }
  return confName;
}

std::string cmExtraCodeLiteGenerator::GetBuildCommand(
  const cmMakefile* mf) const
{
  std::string generator = mf->GetSafeDefinition("CMAKE_GENERATOR");
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string buildCommand = make; // Default
  if (generator == "NMake Makefiles" || generator == "Ninja") {
    buildCommand = make;
  } else if (generator == "MinGW Makefiles" || generator == "Unix Makefiles") {
    std::ostringstream ss;
    ss << make << " -j " << this->CpuCount;
    buildCommand = ss.str();
  }
  return buildCommand;
}

std::string cmExtraCodeLiteGenerator::GetCleanCommand(
  const cmMakefile* mf) const
{
  return GetBuildCommand(mf) + " clean";
}

std::string cmExtraCodeLiteGenerator::GetRebuildCommand(
  const cmMakefile* mf) const
{
  return GetCleanCommand(mf) + " && " + GetBuildCommand(mf);
}

std::string cmExtraCodeLiteGenerator::GetSingleFileBuildCommand(
  const cmMakefile* mf) const
{
  std::string buildCommand;
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string generator = mf->GetSafeDefinition("CMAKE_GENERATOR");
  if (generator == "Unix Makefiles" || generator == "MinGW Makefiles") {
    std::ostringstream ss;
    ss << make << " -f$(ProjectPath)/Makefile $(CurrentFileName).cpp.o";
    buildCommand = ss.str();
  }
  return buildCommand;
}
