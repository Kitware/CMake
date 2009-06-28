/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf neundorf@kde.org, All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmExtraCodeBlocksGenerator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"
#include "cmSystemTools.h"

#include <cmsys/SystemTools.hxx>

/* Some useful URLs:
Homepage: 
http://www.codeblocks.org

File format docs:
http://wiki.codeblocks.org/index.php?title=File_formats_description
http://wiki.codeblocks.org/index.php?title=Workspace_file
http://wiki.codeblocks.org/index.php?title=Project_file

Discussion:
http://forums.codeblocks.org/index.php/topic,6789.0.html
*/

//----------------------------------------------------------------------------
void cmExtraCodeBlocksGenerator
::GetDocumentation(cmDocumentationEntry& entry, const char*) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates CodeBlocks project files.";
  entry.Full =
    "Project files for CodeBlocks will be created in the top directory "
    "and in every subdirectory which features a CMakeLists.txt file "
    "containing a PROJECT() call. "
    "Additionally a hierarchy of makefiles is generated into the "
    "build tree.  The appropriate make program can build the project through "
    "the default make target.  A \"make install\" target is also provided.";
}

cmExtraCodeBlocksGenerator::cmExtraCodeBlocksGenerator()
:cmExternalMakefileProjectGenerator()
{
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
// disable until somebody actually tests it:
//  this->SupportedGlobalGenerators.push_back("MSYS Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}


void cmExtraCodeBlocksGenerator::SetGlobalGenerator(
                                                  cmGlobalGenerator* generator)
{
  cmExternalMakefileProjectGenerator::SetGlobalGenerator(generator);
  cmGlobalUnixMakefileGenerator3* mf = (cmGlobalUnixMakefileGenerator3*)
                                                                     generator;
  mf->SetToolSupportsColor(false);
  mf->SetForceVerboseMakefiles(true);
}

void cmExtraCodeBlocksGenerator::Generate()
{
  // for each sub project in the project create a codeblocks project
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
void cmExtraCodeBlocksGenerator::CreateProjectFile(
                                     const std::vector<cmLocalGenerator*>& lgs)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  std::string outputDir=mf->GetStartOutputDirectory();
  std::string projectName=mf->GetProjectName();

  std::string filename=outputDir+"/";
  filename+=projectName+".cbp";
  std::string sessionFilename=outputDir+"/";
  sessionFilename+=projectName+".layout";

  this->CreateNewProjectFile(lgs, filename);
}


void cmExtraCodeBlocksGenerator
  ::CreateNewProjectFile(const std::vector<cmLocalGenerator*>& lgs,
                         const std::string& filename)
{
  const cmMakefile* mf=lgs[0]->GetMakefile();
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  // figure out the compiler
  std::string compiler = this->GetCBCompilerId(mf);
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");

  fout<<"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n"
        "<CodeBlocks_project_file>\n"
        "   <FileVersion major=\"1\" minor=\"6\" />\n"
        "   <Project>\n"
        "      <Option title=\"" << mf->GetProjectName()<<"\" />\n"
        "      <Option makefile_is_custom=\"1\" />\n"
        "      <Option compiler=\"" << compiler << "\" />\n"
        "      <Build>\n";

  bool installTargetCreated = false;
  bool installStripTargetCreated = false;
  bool testTargetCreated = false;
  bool experimentalTargetCreated = false;
  bool nightlyTargetCreated = false;
  bool packageTargetCreated = false;
  bool packageSourceTargetCreated = false;
  bool rebuildCacheTargetCreated = false;

  this->AppendTarget(fout, "all", 0, make.c_str(), mf, compiler.c_str());

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
          case cmTarget::UTILITY:
          case cmTarget::GLOBAL_TARGET:
            // only add these targets once
            if ((ti->first=="install") && (installTargetCreated==false)) 
              {
              installTargetCreated=true;
              }
            else if ((ti->first=="install/strip") 
                      && (installStripTargetCreated==false)) 
              {
              installStripTargetCreated=true;
              }
            else if ((ti->first=="test") && (testTargetCreated==false)) 
              {
              testTargetCreated=true;
              }
            else if ((ti->first=="Experimental") 
                      && (experimentalTargetCreated==false)) 
              {
              experimentalTargetCreated=true;
              }
            else if ((ti->first=="Nightly") && (nightlyTargetCreated==false)) 
              {
              nightlyTargetCreated=true;
              }
            else if ((ti->first=="package") && (packageTargetCreated==false)) 
              {
              packageTargetCreated=true;
              }
            else if ((ti->first=="package_source") 
                      && (packageSourceTargetCreated==false)) 
              {
              packageSourceTargetCreated=true;
              }
            else if ((ti->first=="rebuild_cache") 
                      && (rebuildCacheTargetCreated==false)) 
              {
              rebuildCacheTargetCreated=true;
              }
            else
              {
              break;
              }
            this->AppendTarget(fout, ti->first.c_str(), 0, 
                               make.c_str(), makefile, compiler.c_str());
            break;
          case cmTarget::EXECUTABLE:
          case cmTarget::STATIC_LIBRARY:
          case cmTarget::SHARED_LIBRARY:
          case cmTarget::MODULE_LIBRARY:
            {
            this->AppendTarget(fout, ti->first.c_str(), &ti->second, 
                               make.c_str(), makefile, compiler.c_str());
            std::string fastTarget = ti->first;
            fastTarget += "/fast";
            this->AppendTarget(fout, fastTarget.c_str(), &ti->second, 
                               make.c_str(), makefile, compiler.c_str());
            }
            break;
          // ignore these:
          case cmTarget::INSTALL_FILES:
          case cmTarget::INSTALL_PROGRAMS:
          case cmTarget::INSTALL_DIRECTORY:
          default:
            break;
        }
      }
    }

  fout<<"      </Build>\n";


  // Collect all used source files in the project
  // Sort them into two containers, one for C/C++ implementation files
  // which may have an acompanying header, one for all other files
  std::map<std::string, cmSourceFile*> cFiles;
  std::set<std::string> otherFiles;
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
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
          {
          const std::vector<cmSourceFile*>&sources=ti->second.GetSourceFiles();
          for (std::vector<cmSourceFile*>::const_iterator si=sources.begin();
               si!=sources.end(); si++)
            {
            // check whether it is a C/C++ implementation file
            bool isCFile = false;
            if ((*si)->GetLanguage() && (*(*si)->GetLanguage() == 'C'))
              {
              for(std::vector<std::string>::const_iterator
                  ext = mf->GetSourceExtensions().begin();
                  ext !=  mf->GetSourceExtensions().end(); 
                  ++ext)
                {
                if ((*si)->GetExtension() == *ext)
                  {
                  isCFile = true;
                  break;
                  }
                }
              }

            // then put it accordingly into one of the two containers
            if (isCFile)
              {
              cFiles[(*si)->GetFullPath()] = *si ;
              }
            else
              {
              otherFiles.insert((*si)->GetFullPath());
              }
            }
          }
        default:  // intended fallthrough
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
  for (std::map<std::string, cmSourceFile*>::const_iterator 
       sit=cFiles.begin();
       sit!=cFiles.end();
       ++sit)
    {
    std::string headerBasename=cmSystemTools::GetFilenamePath(sit->first);
    headerBasename+="/";
    headerBasename+=cmSystemTools::GetFilenameWithoutExtension(sit->first);

    // check if there's a matching header around
    for(std::vector<std::string>::const_iterator
        ext = mf->GetHeaderExtensions().begin();
        ext !=  mf->GetHeaderExtensions().end(); 
        ++ext)
      {
      std::string hname=headerBasename;
      hname += ".";
      hname += *ext;
      // if it's already in the set, don't check if it exists on disk
      std::set<std::string>::const_iterator headerIt=otherFiles.find(hname);
      if (headerIt != otherFiles.end())
        {
        break;
        }

      if(cmSystemTools::FileExists(hname.c_str()))
        {
        otherFiles.insert(hname);
        break;
        }
      }
    }

  // insert all source files in the CodeBlocks project
  // first the C/C++ implementation files, then all others
  for (std::map<std::string, cmSourceFile*>::const_iterator 
       sit=cFiles.begin();
       sit!=cFiles.end();
       ++sit)
    {
    fout<<"      <Unit filename=\""<< sit->first <<"\">\n"
          "      </Unit>\n";
    }
  for (std::set<std::string>::const_iterator 
       sit=otherFiles.begin();
       sit!=otherFiles.end();
       ++sit)
    {
    fout<<"      <Unit filename=\""<< sit->c_str() <<"\">\n"
          "      </Unit>\n";
    }

  fout<<"   </Project>\n"
        "</CodeBlocks_project_file>\n";
}


// Generate the xml code for one target.
void cmExtraCodeBlocksGenerator::AppendTarget(cmGeneratedFileStream& fout,
                                              const char* targetName,
                                              cmTarget* target,
                                              const char* make,
                                              const cmMakefile* makefile,
                                              const char* compiler)
{
  std::string makefileName = makefile->GetStartOutputDirectory();
  makefileName += "/Makefile";
  makefileName = cmSystemTools::ConvertToOutputPath(makefileName.c_str());

  fout<<"      <Target title=\"" << targetName << "\">\n";
  if (target!=0)
    {
    int cbTargetType = this->GetCBTargetType(target);
    fout<<"         <Option output=\"" << target->GetLocation(0) 
                            << "\" prefix_auto=\"0\" extension_auto=\"0\" />\n"
          "         <Option working_dir=\"" 
                            << makefile->GetStartOutputDirectory() << "\" />\n"
          "         <Option object_output=\"./\" />\n"
          "         <Option type=\"" << cbTargetType << "\" />\n"
          "         <Option compiler=\"" << compiler << "\" />\n"
          "         <Compiler>\n";
      // the include directories for this target
      const std::vector<std::string>& incDirs = 
          target->GetMakefile()->GetIncludeDirectories();
      for(std::vector<std::string>::const_iterator dirIt=incDirs.begin();
          dirIt != incDirs.end();
          ++dirIt)
        {
        fout <<"            <Add directory=\"" << dirIt->c_str() << "\" />\n";
        }
      fout<<"         </Compiler>\n";
      }
    else // e.g. all and the GLOBAL and UTILITY targets
    {
    fout<<"         <Option working_dir=\"" 
                            << makefile->GetStartOutputDirectory() << "\" />\n"
        <<"         <Option type=\"" << 4 << "\" />\n";
    }

  fout<<"         <MakeCommands>\n"
        "            <Build command=\"" 
      << this->BuildMakeCommand(make, makefileName.c_str(), targetName)
      << "\" />\n"
        "            <CompileFile command=\"" 
      << this->BuildMakeCommand(make, makefileName.c_str(),"&quot;$file&quot;")
      << "\" />\n"
        "            <Clean command=\"" 
      << this->BuildMakeCommand(make, makefileName.c_str(), "clean") 
      << "\" />\n"
        "            <DistClean command=\"" 
      << this->BuildMakeCommand(make, makefileName.c_str(), "clean") 
      << "\" />\n"
        "         </MakeCommands>\n"
        "      </Target>\n";
  
}


// Translate the cmake compiler id into the CodeBlocks compiler id
std::string cmExtraCodeBlocksGenerator::GetCBCompilerId(const cmMakefile* mf)
{
  // figure out which language to use
  // for now care only for C and C++
  std::string compilerIdVar = "CMAKE_CXX_COMPILER_ID";
  if (this->GlobalGenerator->GetLanguageEnabled("CXX") == false)
    {
    compilerIdVar = "CMAKE_C_COMPILER_ID";
    }

  std::string hostSystemName = mf->GetSafeDefinition("CMAKE_HOST_SYSTEM_NAME");
  std::string systemName = mf->GetSafeDefinition("CMAKE_SYSTEM_NAME");
  std::string compilerId = mf->GetRequiredDefinition(compilerIdVar.c_str());
  std::string compiler = "gcc";
  if (compilerId == "MSVC")
    {
    compiler = "msvc8";
    }
  else if (compilerId == "Borland")
    {
    compiler = "bcc";
    }
  else if (compilerId == "SDCC")
    {
    compiler = "sdcc";
    }
  else if (compilerId == "Intel")
    {
    compiler = "icc";
    }
  else if (compilerId == "Watcom")
    {
    compiler = "ow";
    }
  else if (compilerId == "GNU")
    {
    compiler = "gcc";
    }
  return compiler;
}


// Translate the cmake target type into the CodeBlocks target type id
int cmExtraCodeBlocksGenerator::GetCBTargetType(cmTarget* target)
{
  if ( target->GetType()==cmTarget::EXECUTABLE)
    {
    if ((target->GetPropertyAsBool("WIN32_EXECUTABLE"))
        || (target->GetPropertyAsBool("MACOSX_BUNDLE")))
      {
      return 0;
      }
    else
      {
      return 1;
      }
    }
  else if ( target->GetType()==cmTarget::STATIC_LIBRARY)
    {
    return 2;
    }
  else if ((target->GetType()==cmTarget::SHARED_LIBRARY) 
     || (target->GetType()==cmTarget::MODULE_LIBRARY))
    {
    return 3;
    }
  return 4;
}

// Create the command line for building the given target using the selected
// make
std::string cmExtraCodeBlocksGenerator::BuildMakeCommand(
             const std::string& make, const char* makefile, const char* target)
{
  std::string command = make;
  if (strcmp(this->GlobalGenerator->GetName(), "NMake Makefiles")==0)
    {
    command += " /NOLOGO /f &quot;";
    command += makefile;
    command += "&quot; ";
    command += target;
    }
  else if (strcmp(this->GlobalGenerator->GetName(), "MinGW Makefiles")==0)
    {
    command += " -f ";
    command += makefile;
    command += " ";
    command += target;
    }
  else
    {
    command += " -f &quot;";
    command += makefile;
    command += "&quot; ";
    command += target;
    }
  return command;
}
