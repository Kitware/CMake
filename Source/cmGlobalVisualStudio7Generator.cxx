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
#include "cmGlobalVisualStudio7Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "windows.h"


cmGlobalVisualStudio7Generator::cmGlobalVisualStudio7Generator()
{
  m_FindMakeProgramFile = "CMakeVS7FindMake.cmake";
}


void cmGlobalVisualStudio7Generator::EnableLanguage(const char* lang, 
                                                    cmMakefile *mf)
{
  mf->AddDefinition("CMAKE_GENERATOR_CC", "cl");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "cl");
  this->cmGlobalGenerator::EnableLanguage(lang, mf);
}



int cmGlobalVisualStudio7Generator::TryCompile(const char *, 
                                               const char *bindir, 
                                               const char *projectName,
                                               const char *targetName,
                                               std::string *output)
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
  makeCommand += ".sln /build Debug /project ";
  if (targetName)
    {
    makeCommand += targetName;
    }
  else
    {
    makeCommand += "ALL_BUILD";
    }
  
  int retVal;
  if (!cmSystemTools::RunCommand(makeCommand.c_str(), *output, retVal, 
                                 0, false))
    {
    cmSystemTools::Error("Generator: execution of devenv failed.");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return retVal;
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio7Generator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalVisualStudio7Generator;
  lg->SetGlobalGenerator(this);
  return lg;
}


void cmGlobalVisualStudio7Generator::SetupTests()
{
  std::string ctest = 
    m_LocalGenerators[0]->GetMakefile()->GetDefinition("CMAKE_COMMAND");
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

void cmGlobalVisualStudio7Generator::GenerateConfigurations()
{
  // process the configurations
  std::string configTypes = 
    m_CMakeInstance->GetCacheDefinition("CMAKE_CONFIGURATION_TYPES");
  std::string::size_type start = 0;
  std::string::size_type endpos = 0;
  while(endpos != std::string::npos)
    {
    endpos = configTypes.find(' ', start);
    std::string config;
    std::string::size_type len;
    if(endpos != std::string::npos)
      {
      len = endpos - start;
      }
    else
      {
      len = configTypes.size() - start;
      }
    config = configTypes.substr(start, len);
    if(config == "Debug" || config == "Release" ||
       config == "MinSizeRel" || config == "RelWithDebInfo")
      {
      // only add unique configurations
      if(std::find(m_Configurations.begin(),
                   m_Configurations.end(), config) == m_Configurations.end())
        {
        m_Configurations.push_back(config);
        }
      }
    else
      {
      cmSystemTools::Error(
        "Invalid configuration type in CMAKE_CONFIGURATION_TYPES: ",
        config.c_str(),
        " (Valid types are Debug,Release,MinSizeRel,RelWithDebInfo)");
      }
    start = endpos+1;
    }
  if(m_Configurations.size() == 0)
    {
    m_Configurations.push_back("Debug");
    m_Configurations.push_back("Release");
    }
}

void cmGlobalVisualStudio7Generator::Generate()
{
  // Generate the possible configuraitons
  this->GenerateConfigurations();
  
  // add a special target that depends on ALL projects for easy build
  // of Debug only
  m_LocalGenerators[0]->GetMakefile()->
    AddUtilityCommand("ALL_BUILD", "echo","\"Build all projects\"",false);
  
  // add the Run Tests command
  this->SetupTests();
  
  // first do the superclass method
  this->cmGlobalGenerator::Generate();
  
  // Now write out the DSW
  this->OutputSLNFile();
}

// output the SLN file
void cmGlobalVisualStudio7Generator::OutputSLNFile()
{ 
  // if this is an out of source build, create the output directory
  if(strcmp(m_CMakeInstance->GetStartOutputDirectory(),
            m_CMakeInstance->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_CMakeInstance->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating output directory for SLN file",
                           m_CMakeInstance->GetStartOutputDirectory());
      }
    }
  // create the dsw file name
  std::string fname;
  fname = m_CMakeInstance->GetStartOutputDirectory();
  fname += "/";
  if(strlen(m_LocalGenerators[0]->GetMakefile()->GetProjectName()) == 0)
    {
    m_LocalGenerators[0]->GetMakefile()->SetProjectName("Project");
    }
  fname += m_LocalGenerators[0]->GetMakefile()->GetProjectName();
  fname += ".sln";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open SLN file for write: "
                         ,fname.c_str());
    return;
    }
  this->WriteSLNFile(fout);
}


// Write a SLN file to the stream
void cmGlobalVisualStudio7Generator::WriteSLNFile(std::ostream& fout)
{
  // Write out the header for a SLN file
  this->WriteSLNHeader(fout);
  
  // Get the home directory with the trailing slash
  std::string homedir = m_CMakeInstance->GetHomeDirectory();
  homedir += "/";
    
  // For each cmMakefile, create a VCProj for it, and
  // add it to this SLN file
  unsigned int i;
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = m_LocalGenerators[i]->GetMakefile();

    // Get the source directory from the makefile
    std::string dir = mf->GetStartDirectory();
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");

    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      static_cast<cmLocalVisualStudio7Generator *>(m_LocalGenerators[i])
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
                  l->second.AddLinkLibrary(al->first,cmTarget::GENERAL);
                  }
                }
              }
            }
          }
        }
      // Write the project into the SLN file
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
        {
        cmCustomCommand cc = l->second.GetCustomCommands()[0];
        
        // dodgy use of the cmCustomCommand's members to store the 
        // arguments from the INCLUDE_EXTERNAL_MSPROJECT command
        std::vector<std::string> stuff = cc.GetDepends();
        std::vector<std::string> depends = cc.GetOutputs();
        this->WriteExternalProject(fout, stuff[0].c_str(), 
                                   stuff[1].c_str(), depends);
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
  fout << "Global\n"
       << "\tGlobalSection(SolutionConfiguration) = preSolution\n";
  
  int c = 0;
  for(std::vector<std::string>::iterator i = m_Configurations.begin();
      i != m_Configurations.end(); ++i)
    {
    fout << "\t\tConfigName." << c << " = " << *i << "\n";
    c++;
    }
  fout << "\tEndGlobalSection\n"
       << "\tGlobalSection(ProjectDependencies) = postSolution\n";

  // loop over again and compute the depends
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = m_LocalGenerators[i]->GetMakefile();
    cmLocalVisualStudio7Generator* pg =  
      static_cast<cmLocalVisualStudio7Generator*>(m_LocalGenerators[i]);
    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProjectDepends(fout, si->c_str(), dir.c_str(),l->second);
        ++si;
        }
      }
    }
  fout << "\tEndGlobalSection\n";
  fout << "\tGlobalSection(ProjectConfiguration) = postSolution\n";
  // loop over again and compute the depends
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = m_LocalGenerators[i]->GetMakefile();
    cmLocalVisualStudio7Generator* pg =  
      static_cast<cmLocalVisualStudio7Generator*>(m_LocalGenerators[i]);
    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProjectConfigurations(fout, si->c_str(), l->second.IsInAll());
        ++si;
        }
      }
    }
  fout << "\tEndGlobalSection\n";

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmGlobalVisualStudio7Generator::WriteProject(std::ostream& fout, 
                               const char* dspname,
                               const char* dir,
                               const cmTarget&)
{
  std::string d = cmSystemTools::ConvertToOutputPath(dir);
  fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\" = \"" 
       << dspname << "\", \""
       << d << "\\" << dspname << ".vcproj\", \"{"
       << this->CreateGUID(dspname) << "}\"\nEndProject\n";
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmGlobalVisualStudio7Generator::WriteProjectDepends(std::ostream& fout, 
                                      const char* dspname,
                                      const char* ,
                                      const cmTarget& target
  )
{
  int depcount = 0;
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
        // is the library part of this SLN ? If so add dependency
        std::string libPath = j->first + "_CMAKE_PATH";
        const char* cacheValue
          = m_CMakeInstance->GetCacheDefinition(libPath.c_str());
        if(cacheValue)
          {
          fout << "\t\t{" << this->CreateGUID(dspname) << "}." << depcount << " = {"
               << this->CreateGUID(j->first.c_str()) << "}\n";
          depcount++;
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
      fout << "\t\t{" << this->CreateGUID(dspname) << "}." << depcount << " = {"
           << this->CreateGUID(i->c_str()) << "}\n";
      depcount++;
      }
    }
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void 
cmGlobalVisualStudio7Generator::WriteProjectConfigurations(std::ostream& fout, 
                                                           const char* name, 
                                                           bool in_all_build)
{
  std::string guid = this->CreateGUID(name);
  for(std::vector<std::string>::iterator i = m_Configurations.begin();
      i != m_Configurations.end(); ++i)
    {
    fout << "\t\t{" << guid << "}." << *i << ".ActiveCfg = " << *i << "|Win32\n";
    if (in_all_build)
      {
      fout << "\t\t{" << guid << "}." << *i << ".Build.0 = " << *i << "|Win32\n";
      }
    }
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmGlobalVisualStudio7Generator::WriteExternalProject(std::ostream& , 
                               const char* ,
                               const char* ,
                               const std::vector<std::string>& )
{
  cmSystemTools::Error("WriteExternalProject not implemented");
//  fout << "#########################################################"
//     "######################\n\n";
//   fout << "Project: \"" << name << "\"=" 
//        << location << " - Package Owner=<4>\n\n";
//   fout << "Package=<5>\n{{{\n}}}\n\n";
//   fout << "Package=<4>\n";
//   fout << "{{{\n";

  
//   std::vector<std::string>::const_iterator i, end;
//   // write dependencies.
//   i = dependencies.begin();
//   end = dependencies.end();
//   for(;i!= end; ++i)
//   {
//     fout << "Begin Project Dependency\n";
//     fout << "Project_Dep_Name " << *i << "\n";
//     fout << "End Project Dependency\n";
//   }
//   fout << "}}}\n\n";
}



// Standard end of dsw file
void cmGlobalVisualStudio7Generator::WriteSLNFooter(std::ostream& fout)
{
  fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "EndGlobal\n";
}

  
// ouput standard header for dsw file
void cmGlobalVisualStudio7Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 7.00\n";
}


std::string cmGlobalVisualStudio7Generator::CreateGUID(const char* name)
{
  std::map<cmStdString, cmStdString>::iterator i = m_GUIDMap.find(name);
  if(i != m_GUIDMap.end())
    {
    return i->second;
    }
  std::string ret;
  UUID uid;
  unsigned char *uidstr;
  UuidCreate(&uid);
  UuidToString(&uid,&uidstr);
  ret = reinterpret_cast<char*>(uidstr);
  RpcStringFree(&uidstr);
  ret = cmSystemTools::UpperCase(ret);
  m_GUIDMap[name] = ret;
  return ret;
}

void cmGlobalVisualStudio7Generator::LocalGenerate()
{
  // load the possible configuraitons
  this->GenerateConfigurations();
  this->cmGlobalGenerator::LocalGenerate();
}

std::vector<std::string> *cmGlobalVisualStudio7Generator::GetConfigurations()
{
  return &m_Configurations;
};
