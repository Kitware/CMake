/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalVisualStudio6Generator.h"
#include "cmLocalVisualStudio6Generator.h"
#include "cmMakefile.h"
#include "cmake.h"

cmGlobalVisualStudio6Generator::cmGlobalVisualStudio6Generator()
{
  m_FindMakeProgramFile = "CMakeVS6FindMake.cmake";
}

void cmGlobalVisualStudio6Generator::EnableLanguage(std::vector<std::string>const& lang,
                                                    cmMakefile *mf)
{
  mf->AddDefinition("CMAKE_CFG_INTDIR","$(IntDir)");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "cl");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "cl");
  mf->AddDefinition("CMAKE_GENERATOR_RC", "rc"); 
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  mf->AddDefinition("CMAKE_GENERATOR_Fortran", "ifort");
  this->GenerateConfigurations(mf);
  this->cmGlobalGenerator::EnableLanguage(lang, mf);
}

void cmGlobalVisualStudio6Generator::GenerateConfigurations(cmMakefile* mf)
{
  std::string fname= mf->GetRequiredDefinition("CMAKE_ROOT");
  const char* def= mf->GetDefinition( "MSPROJECT_TEMPLATE_DIRECTORY");
  if(def)
    {
    fname = def;
    }
  else
    {
    fname += "/Templates";
    }
  fname += "/CMakeVisualStudio6Configurations.cmake";
  if(!mf->ReadListFile(mf->GetCurrentListFile(), fname.c_str()))
    {
    cmSystemTools::Error("Cannot open ", fname.c_str(),
                         ".  Please copy this file from the main "
                         "CMake/Templates directory and edit it for "
                         "your build configurations.");
    }
  else if(!mf->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::Error("CMAKE_CONFIGURATION_TYPES not set by ",
                         fname.c_str(),
                         ".  Please copy this file from the main "
                         "CMake/Templates directory and edit it for "
                         "your build configurations.");
    }
}

int cmGlobalVisualStudio6Generator::TryCompile(const char *, 
                                               const char *bindir, 
                                               const char *projectName,
                                               const char *targetName,
                                               std::string *output,
                                               cmMakefile* mf)
{
  // now build the test
  std::string makeCommand = 
    m_CMakeInstance->GetCacheManager()->GetCacheValue("CMAKE_MAKE_PROGRAM");
  std::vector<std::string> mp;
  mp.push_back("[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\6.0\\Setup;VsCommonDir]/MSDev98/Bin");
  cmSystemTools::ExpandRegistryValues(mp[0]);
  std::string originalCommand = makeCommand;
  makeCommand = cmSystemTools::FindProgram(makeCommand.c_str(), mp);
  if(makeCommand.size() == 0)
    {
    std::string e = "Generator cannot find Visual Studio 6 msdev program \"";
    e += originalCommand;
    e += "\" specified by CMAKE_MAKE_PROGRAM cache entry.  ";
    e += "Please fix the setting.";
    cmSystemTools::Error(e.c_str());
    return 1;
    }
  makeCommand = cmSystemTools::ConvertToOutputPath(makeCommand.c_str());

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);
  // if there are spaces in the makeCommand, assume a full path
  // and convert it to a path with no spaces in it as the
  // RunSingleCommand does not like spaces
#if defined(_WIN32) && !defined(__CYGWIN__)      
  if(makeCommand.find(' ') != std::string::npos)
    {
    cmSystemTools::GetShortPath(makeCommand.c_str(), makeCommand);
    }
#endif
  makeCommand += " ";
  makeCommand += projectName;
  makeCommand += ".dsw /MAKE \"";
  if (targetName)
    {
    makeCommand += targetName;
    }
  else
    {
    makeCommand += "ALL_BUILD";
    }
  makeCommand += " - ";
  if(const char* config = mf->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION"))
    {
    makeCommand += config;
    }
  else
    {
    makeCommand += "Debug";
    }
  makeCommand += "\"";
  int retVal;
  int timeout = cmGlobalGenerator::s_TryCompileTimeout;
  if (!cmSystemTools::RunSingleCommand(makeCommand.c_str(), output, 
      &retVal, 0, false, timeout))
    {
    std::string e = "Error executing make program \"";
    e += originalCommand;
    e += "\" specified by CMAKE_MAKE_PROGRAM cache entry.  ";
    e += "The command string used was \"";
    e += makeCommand.c_str();
    e += "\".";
    cmSystemTools::Error(e.c_str());
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return retVal;
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio6Generator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalVisualStudio6Generator;
  lg->SetGlobalGenerator(this);
  return lg;
}


void cmGlobalVisualStudio6Generator::Generate()
{
  // collect sub-projects
  this->CollectSubprojects();
  
  // add a special target that depends on ALL projects for easy build
  // of one configuration only.  
  std::vector<std::string> srcs;
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = m_SubProjectMap.begin(); it!= m_SubProjectMap.end(); ++it)
    {
    std::vector<cmLocalGenerator*>& gen = it->second;
    // add the ALL_BUILD to the first local generator of each project
    if(gen.size())
      {
      gen[0]->GetMakefile()->
        AddUtilityCommand("ALL_BUILD", "echo","\"Build all projects\"",false,srcs);
      std::string cmake_command = 
        m_LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
      gen[0]->GetMakefile()->
        AddUtilityCommand("INSTALL", cmake_command.c_str(),
          "-DBUILD_TYPE=$(IntDir) -P cmake_install.cmake",false,srcs);
      }
    }
  
  // add the Run Tests command
  this->SetupTests();
  
  // first do the superclass method
  this->cmGlobalGenerator::Generate();
  
  // Now write out the DSW
  this->OutputDSWFile();
}

// populate the m_SubProjectMap 
void cmGlobalVisualStudio6Generator::CollectSubprojects()
{
  unsigned int i;
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    std::string name = m_LocalGenerators[i]->GetMakefile()->GetProjectName();
    m_SubProjectMap[name].push_back(m_LocalGenerators[i]);
    std::vector<std::string> const& pprojects 
      = m_LocalGenerators[i]->GetMakefile()->GetParentProjects();
    for(unsigned int k =0; k < pprojects.size(); ++k)
      {
      m_SubProjectMap[pprojects[k]].push_back(m_LocalGenerators[i]);
      }
    }
}


// Write a DSW file to the stream
void cmGlobalVisualStudio6Generator::WriteDSWFile(std::ostream& fout,
                                                  cmLocalGenerator* root,
                                                  std::vector<cmLocalGenerator*>& generators)
{
  // Write out the header for a DSW file
  this->WriteDSWHeader(fout);
  
  // Get the home directory with the trailing slash
  std::string homedir = root->GetMakefile()->GetCurrentDirectory();
  homedir += "/";
    
  unsigned int i;
  bool doneAllBuild = false;
  bool doneRunTests = false;
  bool doneInstall = false;

  for(i = 0; i < generators.size(); ++i)
    {
    if(this->IsExcluded(root, generators[i]))
      {
      continue;
      }
    cmMakefile* mf = generators[i]->GetMakefile();
    
    // Get the source directory from the makefile
    std::string dir = mf->GetStartDirectory();
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");

    // Get the list of create dsp files names from the LocalGenerator, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      static_cast<cmLocalVisualStudio6Generator *>(generators[i])
      ->GetCreatedProjectNames();
    cmTargets &tgts = generators[i]->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      // special handling for the current makefile
      if(mf == generators[0]->GetMakefile())
        {
        dir = "."; // no subdirectory for project generated
        // if this is the special ALL_BUILD utility, then
        // make it depend on every other non UTILITY project.
        // This is done by adding the names to the GetUtilities
        // vector on the makefile
        if(l->first == "ALL_BUILD" && !doneAllBuild)
          {
          unsigned int j;
          for(j = 0; j < generators.size(); ++j)
            {
            const cmTargets &atgts = 
              generators[j]->GetMakefile()->GetTargets();
            for(cmTargets::const_iterator al = atgts.begin();
                al != atgts.end(); ++al)
              {
              if (al->second.IsInAll())
                {
                if (al->second.GetType() == cmTarget::UTILITY)
                  {
                  l->second.AddUtility(al->first.c_str());
                  }
                else
                  {
                  l->second.AddLinkLibrary(al->first, cmTarget::GENERAL);
                  }
                }
              }
            }
          }
        }
      // Write the project into the DSW file
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
        {
        cmCustomCommand cc = l->second.GetPostBuildCommands()[0];
        std::string project = cc.GetCommand();
        std::string location = cc.GetArguments();
        this->WriteExternalProject(fout, project.c_str(), location.c_str(), cc.GetDepends());
        }
      else 
        {
        if ((l->second.GetType() != cmTarget::INSTALL_FILES)
            && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
          {
          bool skip = false;
          // skip ALL_BUILD and RUN_TESTS if they have already been added
          if(l->first == "ALL_BUILD" )
            {
            if(doneAllBuild)
              {
              skip = true;
              }
            else
              {
              doneAllBuild = true;
              }
            }
          if(l->first == "INSTALL")
            {
            if(doneInstall)
              {
              skip = true;
              }
            else
              {
              doneInstall = true;
              }
            }
          if(l->first == "RUN_TESTS")
            {
            if(doneRunTests)
              {
              skip = true;
              }
            else
              {
              doneRunTests = true;
              }
            }
          if(!skip)
            {
            this->WriteProject(fout, si->c_str(), dir.c_str(),l->second);
            }
          ++si;
          }
        }
      }
    }
  
  // Write the footer for the DSW file
  this->WriteDSWFooter(fout);
}

void cmGlobalVisualStudio6Generator::OutputDSWFile(cmLocalGenerator* root, 
                                                   std::vector<cmLocalGenerator*>& generators)
{
  if(generators.size() == 0)
    {
    return;
    }
  std::string fname = root->GetMakefile()->GetStartOutputDirectory();
  fname += "/";
  fname += root->GetMakefile()->GetProjectName();
  fname += ".dsw";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open DSW file for write: ",
                         fname.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }
  this->WriteDSWFile(fout, root, generators);
}

// output the DSW file
void cmGlobalVisualStudio6Generator::OutputDSWFile()
{ 
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = m_SubProjectMap.begin(); it!= m_SubProjectMap.end(); ++it)
    {
    this->OutputDSWFile(it->second[0], it->second);
    }
}


inline std::string removeQuotes(const std::string& s)
{
  if(s[0] == '\"' && s[s.size()-1] == '\"')
    {
    return s.substr(1, s.size()-2);
    }
  return s;
}


void cmGlobalVisualStudio6Generator::SetupTests()
{
  std::string ctest = 
    m_LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
  ctest = removeQuotes(ctest);
  ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
  ctest += "/";
  ctest += "ctest";
  ctest += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      m_LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Debug/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      m_LocalGenerators[0]->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Release/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  // if we found ctest
  if (cmSystemTools::FileExists(ctest.c_str()))
    {
    // Create a full path filename for output Testfile
    std::string fname;
    fname = m_CMakeInstance->GetStartOutputDirectory();
    fname += "/";
    fname += "DartTestfile.txt";
    
    // If the file doesn't exist, then ENABLE_TESTING hasn't been run
    if (cmSystemTools::FileExists(fname.c_str()))
      {
      std::vector<std::string> srcs;
      std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
      for(it = m_SubProjectMap.begin(); it!= m_SubProjectMap.end(); ++it)
        {
        std::vector<cmLocalGenerator*>& gen = it->second;
        // add the ALL_BUILD to the first local generator of each project
        if(gen.size())
          {
          gen[0]->GetMakefile()->
            AddUtilityCommand("RUN_TESTS", ctest.c_str(), "-C $(IntDir)",false,srcs);
          }
        }
      }
    }
}


// Write a dsp file into the DSW file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmGlobalVisualStudio6Generator::WriteProject(std::ostream& fout, 
                                                  const char* dspname,
                                                  const char* dir,
                                                  const cmTarget& target)
{
  fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << dspname << "\"=" 
       << dir << "\\" << dspname << ".dsp - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";

  // insert Begin Project Dependency  Project_Dep_Name project stuff here 
  if (target.GetType() != cmTarget::STATIC_LIBRARY)
    {
    cmTarget::LinkLibraries::const_iterator j, jend;
    j = target.GetLinkLibraries().begin();
    jend = target.GetLinkLibraries().end();
    for(;j!= jend; ++j)
      {
      if(j->first != dspname)
        {
        // is the library part of this DSW ? If so add dependency
        std::string libPath = j->first + "_CMAKE_PATH";
        const char* cacheValue
          = m_CMakeInstance->GetCacheDefinition(libPath.c_str());
        if(cacheValue && *cacheValue)
          { 
          fout << "Begin Project Dependency\n";
          fout << "Project_Dep_Name " << j->first.c_str() << "\n";
          fout << "End Project Dependency\n";
          }
        }
      }
    }

  std::set<cmStdString>::const_iterator i, end;
  // write utility dependencies.
  i = target.GetUtilities().begin();
  end = target.GetUtilities().end();
  for(;i!= end; ++i)
    {
    if(*i != dspname)
      {
      std::string depName = *i;
      if(strncmp(depName.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
        {
        depName.erase(depName.begin(), depName.begin() + 27);
        }
          
      fout << "Begin Project Dependency\n";
      fout << "Project_Dep_Name " << depName << "\n";
      fout << "End Project Dependency\n";
      }
    }
  fout << "}}}\n\n";
}


// Write a dsp file into the DSW file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmGlobalVisualStudio6Generator::WriteExternalProject(std::ostream& fout, 
                               const char* name,
                               const char* location,
                               const std::vector<std::string>& dependencies)
{
 fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << name << "\"=" 
       << location << " - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";

  
  std::vector<std::string>::const_iterator i, end;
  // write dependencies.
  i = dependencies.begin();
  end = dependencies.end();
  for(;i!= end; ++i)
  {
    fout << "Begin Project Dependency\n";
    fout << "Project_Dep_Name " << *i << "\n";
    fout << "End Project Dependency\n";
  }
  fout << "}}}\n\n";
}



// Standard end of dsw file
void cmGlobalVisualStudio6Generator::WriteDSWFooter(std::ostream& fout)
{
  fout << "######################################################"
    "#########################\n\n";
  fout << "Global:\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<3>\n{{{\n}}}\n\n";
  fout << "#####################################################"
    "##########################\n\n";
}

  
// ouput standard header for dsw file
void cmGlobalVisualStudio6Generator::WriteDSWHeader(std::ostream& fout)
{
  fout << "Microsoft Developer Studio Workspace File, Format Version 6.00\n";
  fout << "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n\n";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio6Generator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Visual Studio 6 project files.";
  entry.full = "";
}
