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
#include "cmExtraKateGenerator.h"
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

//----------------------------------------------------------------------------
void cmExtraKateGenerator
::GetDocumentation(cmDocumentationEntry& entry, const char*) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Kate project files.";
}

cmExtraKateGenerator::cmExtraKateGenerator()
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


void cmExtraKateGenerator::Generate()
{
  const cmMakefile* mf
     = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();
  this->ProjectName = this->GenerateProjectName(mf->GetProjectName(),
                          mf->GetSafeDefinition("CMAKE_BUILD_TYPE"),
                          this->GetPathBasename(mf->GetHomeOutputDirectory()));
  this->CreateKateProjectFile(mf);
  this->CreateDummyKateProjectFile(mf);
}


void cmExtraKateGenerator::CreateKateProjectFile(const cmMakefile* mf) const
{
  std::string filename = mf->GetHomeOutputDirectory();
  filename += "/.kateproject";
  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string args = mf->GetRequiredDefinition("CMAKE_KATE_MAKE_ARGUMENTS");

  fout <<
    "{\n"
    "\t\"name\": \"" << this->ProjectName << "\",\n"
    "\t\"directory\": \"" << mf->GetHomeDirectory() << "\",\n"
    "\t\"files\": [ { " << this->GenerateFilesString(mf) << "} ],\n";
  this->WriteTargets(mf, fout);
  fout << "}\n";
}


void
cmExtraKateGenerator::WriteTargets(const cmMakefile* mf,
                                   cmGeneratedFileStream& fout) const
{
  fout <<
  "\t\"build\": {\n"
  "\t\t\"directory\": \"" << mf->GetHomeOutputDirectory() << "\",\n"
  "\t\t\"default_target\": \"all\",\n"
  "\t\t\"prev_target\": \"all\",\n"
  "\t\t\"clean_target\": \"clean\",\n"
  "\t\t\"targets\":[\n";

  const std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  const std::string makeArgs = mf->GetSafeDefinition(
                                              "CMAKE_KATE_MAKE_ARGUMENTS");

  this->AppendTarget(fout, "all", make, makeArgs,
                     mf->GetHomeOutputDirectory());
  this->AppendTarget(fout, "clean", make, makeArgs,
                     mf->GetHomeOutputDirectory());

  // add all executable and library targets and some of the GLOBAL
  // and UTILITY targets
  for (std::vector<cmLocalGenerator*>::const_iterator
       it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {
    const cmTargets& targets = (*it)->GetMakefile()->GetTargets();
    cmMakefile* makefile=(*it)->GetMakefile();
    std::string currentDir = makefile->GetCurrentOutputDirectory();
    bool topLevel = (currentDir == makefile->GetHomeOutputDirectory());

    for(cmTargets::const_iterator ti=targets.begin(); ti!=targets.end(); ++ti)
      {
      switch(ti->second.GetType())
        {
        case cmTarget::GLOBAL_TARGET:
          {
          bool insertTarget = false;
          // Only add the global targets from CMAKE_BINARY_DIR,
          // not from the subdirs
          if (topLevel)
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
            this->AppendTarget(fout, ti->first, make, makeArgs, currentDir);
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

            this->AppendTarget(fout, ti->first, make, makeArgs, currentDir);
          break;
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
        {
          this->AppendTarget(fout, ti->first, make, makeArgs, currentDir);
          std::string fastTarget = ti->first;
          fastTarget += "/fast";
          this->AppendTarget(fout, fastTarget, make, makeArgs, currentDir);

        }
        break;
        default:
          break;
      }
    }

    //insert rules for compiling, preprocessing and assembling individual files
    std::vector<std::string> objectFileTargets;
    (*it)->GetIndividualFileTargets(objectFileTargets);
    for(std::vector<std::string>::const_iterator fit=objectFileTargets.begin();
        fit != objectFileTargets.end();
        ++fit)
      {
      this->AppendTarget(fout, *fit, make, makeArgs, currentDir);
      }
  }

  fout <<
  "\t] }\n";
}


void
cmExtraKateGenerator::AppendTarget(cmGeneratedFileStream& fout,
                                   const std::string&     target,
                                   const std::string&     make,
                                   const std::string&     makeArgs,
                                   const std::string&     path) const
{
  static char JsonSep = ' ';

  fout <<
    "\t\t\t" << JsonSep << "{\"name\":\"" << target << "\", "
    "\"build_cmd\":\"" << make << " -C " << path << " " << makeArgs << " "
                       << target << "\"}\n";

  JsonSep = ',';
}



void
cmExtraKateGenerator::CreateDummyKateProjectFile(const cmMakefile* mf) const
{
  std::string filename = mf->GetHomeOutputDirectory();
  filename += "/";
  filename += this->ProjectName;
  filename += ".kateproject";
  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  fout << "#Generated by cmake, do not edit.\n";
}


std::string
cmExtraKateGenerator::GenerateFilesString(const cmMakefile* mf) const
{
  std::string s = mf->GetHomeDirectory();
  s += "/.git";
  if(cmSystemTools::FileExists(s.c_str()))
  {
    return std::string("\"git\": 1 ");
  }

  s = mf->GetHomeDirectory();
  s += "/.svn";
  if(cmSystemTools::FileExists(s.c_str()))
  {
    return std::string("\"svn\": 1 ");
  }

  s = mf->GetHomeDirectory();
  s += "/";

  std::set<std::string> files;
  std::string tmp;
  const std::vector<cmLocalGenerator *>& lgs =
                                   this->GlobalGenerator->GetLocalGenerators();

  for (std::vector<cmLocalGenerator*>::const_iterator it=lgs.begin();
       it!=lgs.end(); it++)
    {
    cmMakefile* makefile=(*it)->GetMakefile();
    const std::vector<std::string>& listFiles=makefile->GetListFiles();
    for (std::vector<std::string>::const_iterator lt=listFiles.begin();
         lt!=listFiles.end(); lt++)
      {
      tmp=*lt;
        {
        files.insert(tmp);
        }
      }

    const std::vector<cmSourceFile*>& sources = makefile->GetSourceFiles();
    for (std::vector<cmSourceFile*>::const_iterator sfIt = sources.begin();
         sfIt != sources.end(); sfIt++)
      {
      cmSourceFile* sf = *sfIt;
      if (sf->GetPropertyAsBool("GENERATED"))
        {
        continue;
        }

      tmp = sf->GetFullPath();
      files.insert(tmp);
      }
    }

  const char* sep = "";
  tmp = "\"list\": [";
  for(std::set<std::string>::const_iterator it = files.begin();
      it != files.end(); ++it)
    {
    tmp += sep;
    tmp += " \"";
    tmp += *it;
    tmp += "\"";
    sep = ",";
    }
  tmp += "] ";

  return tmp;
}


std::string cmExtraKateGenerator::GenerateProjectName(const std::string& name,
                                                 const std::string& type,
                                                 const std::string& path) const
{
  return name + (type.empty() ? "" : "-") + type + "@" + path;
}


std::string cmExtraKateGenerator::GetPathBasename(const std::string& path)const
{
  std::string outputBasename = path;
  while (outputBasename.size() > 0 &&
         (outputBasename[outputBasename.size() - 1] == '/' ||
          outputBasename[outputBasename.size() - 1] == '\\'))
    {
    outputBasename.resize(outputBasename.size() - 1);
    }
  std::string::size_type loc = outputBasename.find_last_of("/\\");
  if (loc != std::string::npos)
    {
    outputBasename = outputBasename.substr(loc + 1);
    }

  return outputBasename;
}


// Create the command line for building the given target using the selected
// make
std::string cmExtraKateGenerator::BuildMakeCommand(const std::string& make,
                                const char* makefile, const char* target) const
{
  std::string command = make;
  if (strcmp(this->GlobalGenerator->GetName(), "NMake Makefiles")==0)
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += " /NOLOGO /f &quot;";
    command += makefileName;
    command += "&quot; ";
    command += " VERBOSE=1 ";
    command += target;
    }
  else if (strcmp(this->GlobalGenerator->GetName(), "MinGW Makefiles")==0)
    {
    // no escaping of spaces in this case, see
    // http://public.kitware.com/Bug/view.php?id=10014
    std::string makefileName = makefile;
    command += " -f &quot;";
    command += makefileName;
    command += "&quot; ";
    command += " VERBOSE=1 ";
    command += target;
    }
  else if (strcmp(this->GlobalGenerator->GetName(), "Ninja")==0)
    {
    command += " -v ";
    command += target;
    }
  else
    {
    std::string makefileName = cmSystemTools::ConvertToOutputPath(makefile);
    command += " -f &quot;";
    command += makefileName;
    command += "&quot; ";
    command += " VERBOSE=1 ";
    command += target;
    }
  return command;
}
