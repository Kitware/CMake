/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalVisualStudio6Generator.h"
#include "cmLocalVisualStudio6Generator.h"
#include "cmMakefile.h"
#include "cmake.h"

void cmGlobalVisualStudio6Generator::EnableLanguage(const char*, 
                                                    cmMakefile *mf)
{
  // now load the settings
  if(!mf->GetDefinition("CMAKE_ROOT"))
    {
    cmSystemTools::Error(
      "CMAKE_ROOT has not been defined, bad GUI or driver program");
    return;
    }
  if(!this->GetLanguageEnabled("CXX"))
    {
    std::string fpath = 
      mf->GetDefinition("CMAKE_ROOT");
    fpath += "/Templates/CMakeWindowsSystemConfig.cmake";
    mf->ReadListFile(NULL,fpath.c_str());
    this->SetLanguageEnabled("CXX");
    }
}

int cmGlobalVisualStudio6Generator::TryCompile(const char *, 
                                               const char *bindir, 
                                               const char *projectName,
                                               const char *targetName)
{
  // now build the test
  std::string makeCommand = 
    m_CMakeInstance->GetCacheManager()->GetCacheValue("CMAKE_MAKE_PROGRAM");
  if(makeCommand.size() == 0)
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return 1;
    }
  makeCommand = cmSystemTools::ConvertToOutputPath(makeCommand.c_str());
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string output;

  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  // if there are spaces in the makeCommand, assume a full path
  // and convert it to a path with no spaces in it as the
  // RunCommand does not like spaces
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
  makeCommand += " - Debug\" /REBUILD";
  
  int retVal;
  if (!cmSystemTools::RunCommand(makeCommand.c_str(), output, retVal))
    {
    cmSystemTools::Error("Generator: execution of msdev failed.");
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
  // add a special target that depends on ALL projects for easy build
  // of Debug only
  m_LocalGenerators[0]->GetMakefile()->
    AddUtilityCommand("ALL_BUILD", "echo","\"Build all projects\"",false);

  // add the Run Tests command
  this->SetupTests();
  
  // first do the superclass method
  this->cmGlobalGenerator::Generate();
  
  // Now write out the DSW
  this->OutputDSWFile();
}

// output the DSW file
void cmGlobalVisualStudio6Generator::OutputDSWFile()
{ 
  // create the dsw file name
  std::string fname;
  fname = m_CMakeInstance->GetStartOutputDirectory();
  fname += "/";
  if(strlen(m_LocalGenerators[0]->GetMakefile()->GetProjectName()))
    {
    fname += m_LocalGenerators[0]->GetMakefile()->GetProjectName();
    }
  else
    {
    fname += "Project";
    }
  fname += ".dsw";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open DSW file for write: "
                         ,fname.c_str());
    return;
    }
  this->WriteDSWFile(fout);
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
    m_LocalGenerators[0]->GetMakefile()->GetDefinition("CMAKE_COMMAND");
  ctest = removeQuotes(ctest);
  ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
  ctest += "/";
  ctest += "ctest";
  ctest += cmSystemTools::GetExecutableExtension();
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      m_LocalGenerators[0]->GetMakefile()->GetDefinition("CMAKE_COMMAND");
    ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
    ctest += "/Debug/";
    ctest += "ctest";
    ctest += cmSystemTools::GetExecutableExtension();
    }
  if(!cmSystemTools::FileExists(ctest.c_str()))
    {
    ctest =     
      m_LocalGenerators[0]->GetMakefile()->GetDefinition("CMAKE_COMMAND");
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
      m_LocalGenerators[0]->GetMakefile()->
        AddUtilityCommand("RUN_TESTS", ctest.c_str(), "-D $(IntDir)",false);
      }
    }
}

// Write a DSW file to the stream
void cmGlobalVisualStudio6Generator::WriteDSWFile(std::ostream& fout)
{
  // Write out the header for a DSW file
  this->WriteDSWHeader(fout);
  
  
  // Get the home directory with the trailing slash
  std::string homedir = m_CMakeInstance->GetHomeDirectory();
  homedir += "/";
    
  unsigned int i;
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = m_LocalGenerators[i]->GetMakefile();
    
    // Get the source directory from the makefile
    std::string dir = mf->GetStartDirectory();
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");

    // Get the list of create dsp files names from the LocalGenerator, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      static_cast<cmLocalVisualStudio6Generator *>(m_LocalGenerators[i])
      ->GetCreatedProjectNames();
    cmTargets &tgts = m_LocalGenerators[i]->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      // special handling for the current makefile
      if(mf == m_LocalGenerators[0]->GetMakefile())
        {
        dir = "."; // no subdirectory for project generated
        // if this is the special ALL_BUILD utility, then
        // make it depend on every other non UTILITY project.
        // This is done by adding the names to the GetUtilities
        // vector on the makefile
        if(l->first == "ALL_BUILD")
          {
          unsigned int j;
          for(j = 0; j < m_LocalGenerators.size(); ++j)
            {
            const cmTargets &atgts = 
              m_LocalGenerators[j]->GetMakefile()->GetTargets();
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
        cmCustomCommand cc = l->second.GetCustomCommands()[0];
        
        // dodgy use of the cmCustomCommand's members to store the 
        // arguments from the INCLUDE_EXTERNAL_MSPROJECT command
        std::vector<std::string> stuff = cc.GetDepends();
        std::vector<std::string> depends = cc.GetOutputs();
        this->WriteExternalProject(fout, stuff[0].c_str(), stuff[1].c_str(), depends);
        ++si;
        }
      else 
        {
        if ((l->second.GetType() != cmTarget::INSTALL_FILES)
            && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
          {
          this->WriteProject(fout, si->c_str(), dir.c_str(),l->second);
          ++si;
          }
        }
      }
    }
  
  // Write the footer for the DSW file
  this->WriteDSWFooter(fout);
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
        if(cacheValue)
          {
          fout << "Begin Project Dependency\n";
          fout << "Project_Dep_Name " << j->first << "\n";
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
      fout << "Begin Project Dependency\n";
      fout << "Project_Dep_Name " << *i << "\n";
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
