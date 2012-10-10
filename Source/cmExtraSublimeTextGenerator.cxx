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

#if defined(_WIN32)
#  define PATH_SEP "\\"
#else
#  define PATH_SEP "/"
#endif

/* Some useful URLs:
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
    "Additionally a hierarchy of makefiles is generated into the "
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


/* create the project file */
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

  // A set of folders to include in the project
  std::set<std::string> folderIncludePatternsSet;
  // Collect all files, this includes source files and list files
  std::vector<std::string> allFiles;
  std::stringstream fileIncludePatternsStream;
  for (std::vector<cmLocalGenerator *>::const_iterator
       it = lgs.begin();
       it != lgs.end();
       ++it)
    {
    cmMakefile* makefile=(*it)->GetMakefile();
    // Add list files
    const std::vector<std::string> & listFiles =
                                            makefile->GetListFiles();
    allFiles.insert(allFiles.end(), listFiles.begin(), listFiles.end());
    // Add source files
    cmTargets& targets=makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {
      switch(ti->second.GetType())
        {
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
        case cmTarget::UTILITY: // can have sources since 2.6.3
          {
          const std::vector<cmSourceFile*>&sources=ti->second.GetSourceFiles();
          for (std::vector<cmSourceFile*>::const_iterator si=sources.begin();
               si!=sources.end(); si++)
            {
            // don't add source files which have the GENERATED property set:
            if ((*si)->GetPropertyAsBool("GENERATED"))
              {
              continue;
              }
              allFiles.push_back((*si)->GetFullPath());
            }
          }
        default:  // intended fallthrough
           break;
        }
      }
    }

  // Convert
  const char* cmakeRoot = mf->GetDefinition("CMAKE_ROOT");
  for (std::vector<std::string>::const_iterator jt = allFiles.begin();
       jt != allFiles.end();
       ++jt)
    {
    // don't put cmake's own files into the project (#12110):
    if (jt->find(cmakeRoot) == 0)
      {
      continue;
      }

    const std::string &relative = cmSystemTools::RelativePath(
                       lgs[0]->GetMakefile()->GetHomeDirectory(),
                       jt->c_str());
    // Split filename from path
    std::string fileName = cmSystemTools::GetFilenameName(relative);
    std::string path = "";
    if (fileName.length() < relative.length())
      {
      path = relative.substr(0, relative.length() - fileName.length() - 1);
      }

    // We don't want paths with CMakeFiles in them
    if (relative.find("CMakeFiles") == std::string::npos)
      {
      if (fileIncludePatternsStream.tellp() > 0)
        {
          fileIncludePatternsStream << ", ";
        }
      fileIncludePatternsStream << "\"" << relative << "\"";
      if ((!path.empty()) && (folderIncludePatternsSet.find(path) ==
                              folderIncludePatternsSet.end()))
        {
        folderIncludePatternsSet.insert(path);
        std::string::size_type splitIndex = path.rfind(PATH_SEP);
        std::string splitPath = path;
        while (splitIndex != std::string::npos)
          {
          splitPath = splitPath.substr(0, splitIndex);
          if ((splitPath.empty()) ||
                  (folderIncludePatternsSet.insert(splitPath).second == false))
            {
            // If the path is already in the set then all of its
            // parents are as well
            break;
            }
          splitIndex = splitPath.rfind(PATH_SEP);
          }
        }
      }
    }
  // Write the folder entries to the project file
  const std::string &homeRelative = cmSystemTools::RelativePath(
                     lgs[0]->GetMakefile()->GetHomeOutputDirectory(),
                     lgs[0]->GetMakefile()->GetHomeDirectory());
  fout << "{\n";
  fout << "\t\"folders\":\n\t[\n\t";
  fout << "\t{\n\t\t\t\"path\": \"" << homeRelative << "\",\n";
  fout << "\t\t\t\"folder_include_patterns\": [";
  std::set<std::string>::const_iterator folderIter =
          folderIncludePatternsSet.begin();
  while (folderIter != folderIncludePatternsSet.end())
    {
    fout << "\"" << *folderIter << "\"";
    folderIter++;
    if (folderIter != folderIncludePatternsSet.end())
      {
        fout << ", ";
      }
    }
  fout << "],\n";
  fout << "\t\t\t\"file_include_patterns\": [" <<
          fileIncludePatternsStream.str() << "]\n";
  fout << "\t\t}\n\t";
  // End of the folders section
  fout << "]";

  // Write the beginning of the build systems section to the project file
  fout << ",\n\t\"build_systems\":\n\t[\n\t";

  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string compiler = "";
  this->AppendTarget(fout, "all", 0, make.c_str(), mf, compiler.c_str(), true);
  this->AppendTarget(fout, "clean", 0, make.c_str(), mf, compiler.c_str(),
                     false);

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
                               false);
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
                             make.c_str(), makefile, compiler.c_str(), false);
          break;
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
          {
          this->AppendTarget(fout, ti->first.c_str(), &ti->second,
                             make.c_str(), makefile, compiler.c_str(), false);
          std::string fastTarget = ti->first;
          fastTarget += "/fast";
          this->AppendTarget(fout, fastTarget.c_str(), &ti->second,
                             make.c_str(), makefile, compiler.c_str(), false);
          }
          break;
        default:
          break;
        }
      }
    }
  // End of build_systems
  fout << "\n\t]\n";

  // End of file
  fout << "}";
}

// Generate the build_system entry for one target
void cmExtraSublimeTextGenerator::AppendTarget(cmGeneratedFileStream& fout,
                                              const char* targetName,
                                              cmTarget* target,
                                              const char* make,
                                              const cmMakefile* makefile,
                                              const char* compiler,
                                              bool firstTarget)
{
  std::string makefileName = makefile->GetStartOutputDirectory();
  makefileName += "/Makefile";
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
  else if (strcmp(this->GlobalGenerator->GetName(), "MinGW Makefiles")==0)
    {
    // no escaping of spaces in this case, see
    // http://public.kitware.com/Bug/view.php?id=10014
    std::string makefileName = makefile;
    command += ", \"-f\", \"";
    command += makefileName + "\"";
    command += ", \"VERBOSE=1\", \"";
    command += target;
    command += "\"";
    }
  else
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += ", \"-f\", \"";
    command += makefileName + "\"";
    command += ", \"VERBOSE=1\", \"";
    command += target;
    command += "\"";
    }
  return command;
}
