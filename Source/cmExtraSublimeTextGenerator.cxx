/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExtraSublimeTextGenerator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"
#include "cmSystemTools.h"
#include "cmXMLSafe.h"

#include <cmsys/SystemTools.hxx>

/*
Sublime Text 2 Generator
Author: Morné Chamberlain
This generator was initially based off of the CodeBlocks generator.

Some useful URLs:
Homepage:
http://www.sublimetext.com/

File format docs:
http://www.sublimetext.com/docs/2/projects.html
http://sublimetext.info/docs/en/reference/build_systems.html
*/

//----------------------------------------------------------------------------
void cmExtraSublimeTextGenerator
::GetDocumentation(cmDocumentationEntry& entry, const char*) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Sublime Text 2 project files.";
  entry.Full =
    "Project files for Sublime Text 2 will be created in the top directory "
    "and in every subdirectory which features a CMakeLists.txt file "
    "containing a PROJECT() call. "
    "Additionally Makefiles (or build.ninja files) are generated into the "
    "build tree.  The appropriate make program can build the project through "
    "the default make target.  A \"make install\" target is also provided.";
}

cmExtraSublimeTextGenerator::cmExtraSublimeTextGenerator()
:cmExternalMakefileProjectGenerator()
{
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
// disable until somebody actually tests it:
//  this->SupportedGlobalGenerators.push_back("MSYS Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Ninja");
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}


void cmExtraSublimeTextGenerator::Generate()
{
  // for each sub project in the project create a sublime text 2 project
  for (std::map<cmStdString, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
      it!= this->GlobalGenerator->GetProjectMap().end();
      ++it)
    {
    // create a project file
    this->CreateProjectFile(it->second);
    }
}


void cmExtraSublimeTextGenerator::CreateProjectFile(
                                     const std::vector<cmLocalGenerator*>& lgs)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  std::string outputDir=mf->GetStartOutputDirectory();
  std::string projectName=mf->GetProjectName();

  std::string filename=outputDir+"/";
  filename+=projectName+".sublime-project";

  this->CreateNewProjectFile(lgs, filename);
}

void cmExtraSublimeTextGenerator
  ::CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                         const std::string& filename)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  const std::string &sourceRootRelativeToOutput = cmSystemTools::RelativePath(
                     lgs[0]->GetMakefile()->GetHomeOutputDirectory(),
                     lgs[0]->GetMakefile()->GetHomeDirectory());
  const std::string &outputRelativeToSourceRoot = cmSystemTools::RelativePath(
                     lgs[0]->GetMakefile()->GetHomeDirectory(),
                     lgs[0]->GetMakefile()->GetHomeOutputDirectory());
  // Write the folder entries to the project file
  fout << "{\n";
  fout << "\t\"folders\":\n\t[\n\t";
  fout << "\t{\n\t\t\t\"path\": \"" << sourceRootRelativeToOutput << "\",\n";
  fout << "\t\t\t\"folder_exclude_patterns\": [\"" <<
          outputRelativeToSourceRoot << "\"],\n";
  fout << "\t\t\t\"file_exclude_patterns\": []\n";
  fout << "\t\t},\n\t";
  // In order for SublimeClang's path resolution to work, the directory that
  // contains the sublime-project file must be included here. We just ensure
  // that no files or subfolders are included
  fout << "\t{\n\t\t\t\"path\": \"./\",\n";
  fout << "\t\t\t\"folder_exclude_patterns\": [\"*\"],\n";
  fout << "\t\t\t\"file_exclude_patterns\": [\"*\"]\n";
  fout << "\t\t}\n\t";
  // End of the folders section
  fout << "]";

  // Write the beginning of the build systems section to the project file
  fout << ",\n\t\"build_systems\":\n\t[\n\t";

  // Set of include directories over all targets (sublime text/sublimeclang
  // doesn't currently support these settings per build system, only project
  // wide
  std::set<std::string> includeDirs;
  std::set<std::string> defines;
  AppendAllTargets(lgs, mf, fout, includeDirs, defines);
  // End of build_systems
  fout << "\n\t]";

  // Write the settings section with sublimeclang options
  fout << ",\n\t\"settings\":\n\t{\n\t";
  // Check if the CMAKE_SUBLIMECLANG_DISABLED flag has been set. If it has
  // disable sublimeclang for this project.
  const char* sublimeclangDisabledValue =
    lgs[0]->GetMakefile()->GetSafeDefinition("CMAKE_SUBLIMECLANG_DISABLED");
  bool sublimeclangEnabled = cmSystemTools::IsOff(sublimeclangDisabledValue);
  // Per project sublimeclang settings override default and user settings,
  // so we only write the sublimeclang_enabled setting to the project file
  // if it is set to be disabled.
  if (!sublimeclangEnabled)
    {
      fout << "\t\"sublimeclang_enabled\": false,\n\t";
    }
  fout << "\t\"sublimeclang_options\":\n\t\t[\n\t\t";
  std::set<std::string>::const_iterator stringSetIter = includeDirs.begin();
  while (stringSetIter != includeDirs.end())
    {
    const std::string &includeDir = *stringSetIter;
    const std::string &relative = cmSystemTools::RelativePath(
                       lgs[0]->GetMakefile()->GetHomeOutputDirectory(),
                       includeDir.c_str());
    // It appears that a relative path to the sublime-project file doesn't
    // always work. So we use ${folder:${project_path:<project_filename>}}
    // that SublimeClang will expand to the correct path
    fout << "\t\"-I${folder:${project_path:" << mf->GetProjectName() <<
            ".sublime-project}}/" << relative << "\"";
    stringSetIter++;
    if ((stringSetIter != includeDirs.end()) || (!defines.empty()))
      {
      fout << ",";
      }
    fout << "\n\t\t";
  }
  stringSetIter = defines.begin();
  while (stringSetIter != defines.end())
    {
    fout << "\t\"-D" << *stringSetIter << "\"";
    stringSetIter++;
    if (stringSetIter != defines.end())
      {
      fout << ",";
      }
    fout << "\n\t\t";
  }
  // End of the sublimeclang_options section
  fout << "]\n\t";
  // End of the settings section
  fout << "}\n";

  // End of file
  fout << "}";
}

void cmExtraSublimeTextGenerator::
  AppendAllTargets(const std::vector<cmLocalGenerator*>& lgs,
                   const cmMakefile* mf,
                   cmGeneratedFileStream& fout,
                   std::set<std::string>& includeDirs,
                   std::set<std::string>& defines)
{
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string compiler = "";
  this->AppendTarget(fout, "all", 0, make.c_str(), mf, compiler.c_str(),
                     includeDirs, defines, true);
  this->AppendTarget(fout, "clean", 0, make.c_str(), mf, compiler.c_str(),
                     includeDirs, defines, false);

  // add all executable and library targets and some of the GLOBAL
  // and UTILITY targets
  for (std::vector<cmLocalGenerator*>::const_iterator lg=lgs.begin();
       lg!=lgs.end(); lg++)
    {
    cmMakefile* makefile=(*lg)->GetMakefile();
    cmTargets& targets=makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {
      switch(ti->second.GetType())
        {
        case cmTarget::GLOBAL_TARGET:
          {
          bool insertTarget = false;
          // Only add the global targets from CMAKE_BINARY_DIR,
          // not from the subdirs
          if (strcmp(makefile->GetStartOutputDirectory(),
                     makefile->GetHomeOutputDirectory())==0)
            {
            insertTarget = true;
            // only add the "edit_cache" target if it's not ccmake, because
            // this will not work within the IDE
            if (ti->first == "edit_cache")
              {
              const char* editCommand = makefile->GetDefinition
                                                        ("CMAKE_EDIT_COMMAND");
              if (editCommand == 0)
                {
                insertTarget = false;
                }
              else if (strstr(editCommand, "ccmake")!=NULL)
                {
                insertTarget = false;
                }
              }
            }
          if (insertTarget)
            {
            this->AppendTarget(fout, ti->first.c_str(), 0,
                               make.c_str(), makefile, compiler.c_str(),
                               includeDirs, defines, false);
            }
          }
          break;
        case cmTarget::UTILITY:
          // Add all utility targets, except the Nightly/Continuous/
          // Experimental-"sub"targets as e.g. NightlyStart
          if (((ti->first.find("Nightly")==0)   &&(ti->first!="Nightly"))
             || ((ti->first.find("Continuous")==0)&&(ti->first!="Continuous"))
             || ((ti->first.find("Experimental")==0)
                                               && (ti->first!="Experimental")))
            {
            break;
            }

          this->AppendTarget(fout, ti->first.c_str(), 0,
                             make.c_str(), makefile, compiler.c_str(),
                             includeDirs, defines, false);
          break;
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
          {
          this->AppendTarget(fout, ti->first.c_str(), &ti->second,
                             make.c_str(), makefile, compiler.c_str(),
                             includeDirs, defines, false);
          std::string fastTarget = ti->first;
          fastTarget += "/fast";
          this->AppendTarget(fout, fastTarget.c_str(), &ti->second,
                             make.c_str(), makefile, compiler.c_str(),
                             includeDirs, defines, false);
          }
          break;
        default:
          break;
        }
      }
    }
}

// Generate the build_system entry for one target
void cmExtraSublimeTextGenerator::AppendTarget(cmGeneratedFileStream& fout,
                                              const char* targetName,
                                              cmTarget* target,
                                              const char* make,
                                              const cmMakefile* makefile,
                                              const char* compiler,
                                              std::set<std::string>&
                                                              includeDirs,
                                              std::set<std::string>& defines,
                                              bool firstTarget)
{
  if (target != 0)
    {
      // the compilerdefines for this target
      cmGeneratorTarget *gtgt = this->GlobalGenerator
                                    ->GetGeneratorTarget(target);
      std::string cdefs = gtgt->GetCompileDefinitions();

      if(!cdefs.empty())
        {
        // Expand the list.
        std::vector<std::string> defs;
        cmSystemTools::ExpandListArgument(cdefs.c_str(), defs);
        for(std::vector<std::string>::const_iterator di = defs.begin();
            di != defs.end(); ++di)
          {
          cmXMLSafe safedef(di->c_str());
          defines.insert(safedef.str());
          }
        }

      // the include directories for this target
      std::vector<std::string> includes;
      target->GetMakefile()->GetLocalGenerator()->
        GetIncludeDirectories(includes, gtgt);
      for(std::vector<std::string>::const_iterator dirIt=includes.begin();
          dirIt != includes.end();
          ++dirIt)
        {
        includeDirs.insert(*dirIt);
        }

      std::string systemIncludeDirs = makefile->GetSafeDefinition(
                                "CMAKE_EXTRA_GENERATOR_C_SYSTEM_INCLUDE_DIRS");
      if (!systemIncludeDirs.empty())
        {
        std::vector<std::string> dirs;
        cmSystemTools::ExpandListArgument(systemIncludeDirs.c_str(), dirs);
        for(std::vector<std::string>::const_iterator dirIt=dirs.begin();
            dirIt != dirs.end();
            ++dirIt)
          {
          includeDirs.insert(*dirIt);
          }
        }

      systemIncludeDirs = makefile->GetSafeDefinition(
                            "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS");
      if (!systemIncludeDirs.empty())
        {
        std::vector<std::string> dirs;
        cmSystemTools::ExpandListArgument(systemIncludeDirs.c_str(), dirs);
        for(std::vector<std::string>::const_iterator dirIt=dirs.begin();
            dirIt != dirs.end();
            ++dirIt)
          {
          includeDirs.insert(*dirIt);
          }
        }
    }

  // Ninja uses ninja.build files (look for a way to get the output file name
  // from cmMakefile or something)
  std::string makefileName;
  if (strcmp(this->GlobalGenerator->GetName(), "Ninja")==0)
    {
      makefileName = "build.ninja";
    }
    else
    {
      makefileName = "Makefile";
    }
  if (!firstTarget)
    {
    fout << ",\n\t";
    }
  fout << "\t{\n\t\t\t\"name\": \"" << makefile->GetProjectName() << " - " <<
          targetName << "\",\n";
  fout << "\t\t\t\"cmd\": [" <<
          this->BuildMakeCommand(make, makefileName.c_str(), targetName) <<
          "],\n";
  fout << "\t\t\t\"working_dir\": \"${project_path}\",\n";
  fout << "\t\t\t\"file_regex\": \"^(..[^:]*):([0-9]+):?([0-9]+)?:? (.*)$\"\n";
  fout << "\t\t}";
}

// Create the command line for building the given target using the selected
// make
std::string cmExtraSublimeTextGenerator::BuildMakeCommand(
             const std::string& make, const char* makefile, const char* target)
{
  std::string command = "\"";
  command += make + "\"";
  if (strcmp(this->GlobalGenerator->GetName(), "NMake Makefiles")==0)
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += ", \"/NOLOGO\", \"/f\", \"";
    command += makefileName + "\"";
    command += ", \"VERBOSE=1\", \"";
    command += target;
    command += "\"";
    }
  else if (strcmp(this->GlobalGenerator->GetName(), "Ninja")==0)
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += ", \"-f\", \"";
    command += makefileName + "\"";
    command += ", \"-v\", \"";
    command += target;
    command += "\"";
    }
  else
    {
    std::string makefileName;
    if (strcmp(this->GlobalGenerator->GetName(), "MinGW Makefiles")==0)
      {
        // no escaping of spaces in this case, see
        // http://public.kitware.com/Bug/view.php?id=10014
        makefileName = makefile;
      }
      else
      {
        makefileName = cmSystemTools::ConvertToOutputPath(makefile);
      }
    command += ", \"-f\", \"";
    command += makefileName + "\"";
    command += ", \"VERBOSE=1\", \"";
    command += target;
    command += "\"";
    }
  return command;
}
