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

#include "cmCTestScriptHandler.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

//#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>

// used for sleep
#ifdef _WIN32
#include "windows.h"
#endif

#include <stdlib.h> 
#include <time.h>
#include <math.h>
#include <float.h>

// needed for sleep
#if !defined(_WIN32)
# include <unistd.h>
#endif

//----------------------------------------------------------------------
cmCTestScriptHandler::cmCTestScriptHandler()
{
  m_Verbose = false; 
  m_Backup = false; 
  m_Makefile = 0;
  m_LocalGenerator = 0;
  m_CMake = 0;
  m_GlobalGenerator = 0;
  
  // the *60 is becuase the settings are in minutes but GetTime is seconds
  m_MinimumInterval = 30*60;
}


//----------------------------------------------------------------------
cmCTestScriptHandler::~cmCTestScriptHandler()
{
  // local generator owns the makefile
  m_Makefile = 0;
  if (m_LocalGenerator)
    {
    delete m_LocalGenerator;
    }
  m_LocalGenerator = 0;
  if (m_GlobalGenerator)
    {
    delete m_GlobalGenerator;
    }
  m_GlobalGenerator = 0;
  if (m_CMake)
    {
    delete m_CMake;
    }
  m_CMake = 0;
}


//----------------------------------------------------------------------
// just adds an argument to the vector
void cmCTestScriptHandler::AddConfigurationScript(const char *script)
{
  m_ConfigurationScripts.push_back(script);
}


//----------------------------------------------------------------------
// the generic entry point for handling scripts, this routine will run all
// the scripts provides a -S arguments
int cmCTestScriptHandler::RunConfigurationScript()
{
  int res = 0;
  std::vector<cmStdString>::iterator it;
  for ( it = m_ConfigurationScripts.begin();
        it != m_ConfigurationScripts.end();
        it ++ )
    {
    // for each script run it
    res += this->RunConfigurationScript(
      cmSystemTools::CollapseFullPath(it->c_str()));
    }
  return res;
}


//----------------------------------------------------------------------
// this sets up some variables for thew script to use, creates the required
// cmake instance and generators, and then reads in the script
int cmCTestScriptHandler::ReadInScript(const std::string& total_script_arg)
{
  // if the argument has a , in it then it needs to be broken into the fist
  // argument (which is the script) and the second argument which will be
  // passed into the scripts as S_ARG
  std::string script = total_script_arg;
  std::string script_arg;
  if (total_script_arg.find(",") != std::string::npos)
    {
    script = total_script_arg.substr(0,total_script_arg.find(","));
    script_arg = total_script_arg.substr(total_script_arg.find(",")+1);
    }
  
  // make sure the file exists
  if (!cmSystemTools::FileExists(script.c_str()))
    {
    std::cerr << "Cannot find file: " << script.c_str() << std::endl;
    return 1;
    }

  // create a cmake instance to read the configuration script
  // read in the list file to fill the cache
  if (m_CMake)
    {
    delete m_CMake;
    delete m_GlobalGenerator;
    delete m_LocalGenerator;
    }
  m_CMake = new cmake;
  m_GlobalGenerator = new cmGlobalGenerator;
  m_GlobalGenerator->SetCMakeInstance(m_CMake);

  m_LocalGenerator = m_GlobalGenerator->CreateLocalGenerator();
  m_LocalGenerator->SetGlobalGenerator(m_GlobalGenerator);
  
  // set a variable with the path to the current script
  m_LocalGenerator->GetMakefile()->AddDefinition("CTEST_SCRIPT_DIRECTORY",
                                   cmSystemTools::GetFilenamePath(
                                     script).c_str());
  m_LocalGenerator->GetMakefile()->AddDefinition("CTEST_SCRIPT_NAME",
                                   cmSystemTools::GetFilenameName(
                                     script).c_str());
  // add the script arg if defined
  if (script_arg.size())
    {
    m_LocalGenerator->GetMakefile()->AddDefinition(
      "CTEST_SCRIPT_ARG", script_arg.c_str());
    }
  
  // finally read in the script
  if (!m_LocalGenerator->GetMakefile()->ReadListFile(0, script.c_str()))
    {
    return 2;
    }
  
  return 0;
}


//----------------------------------------------------------------------
// extract variabels from the script to set ivars
int cmCTestScriptHandler::ExtractVariables()
{
  // get some info that should be set
  m_Makefile = m_LocalGenerator->GetMakefile();

  m_SourceDir.clear();
  m_BinaryDir.clear();
  m_CTestCmd.clear();
  
  if (m_Makefile->GetDefinition("CTEST_SOURCE_DIRECTORY"))
    {
    m_SourceDir = m_Makefile->GetDefinition("CTEST_SOURCE_DIRECTORY");
    }
  if (m_Makefile->GetDefinition("CTEST_BINARY_DIRECTORY"))
    {
    m_BinaryDir = m_Makefile->GetDefinition("CTEST_BINARY_DIRECTORY");
    }
  if (m_Makefile->GetDefinition("CTEST_COMMAND"))
    {
    m_CTestCmd  = m_Makefile->GetDefinition("CTEST_COMMAND");
    }
  m_Backup    = m_Makefile->IsOn("CTEST_BACKUP_AND_RESTORE");

  // in order to backup and restore we also must have the cvs root
  m_CVSCheckOut.clear();
  if (m_Makefile->GetDefinition("CTEST_CVS_CHECKOUT"))
    {
    m_CVSCheckOut = m_Makefile->GetDefinition("CTEST_CVS_CHECKOUT");
    }
  if (m_Backup && m_CVSCheckOut.empty())
    {
    cmSystemTools::Error(
      "Backup was requested without specifying CTEST_CVS_CHECKOUT.");    
    return 3;
    }
  
  // make sure the required info is here
  if (this->m_SourceDir.empty() || 
      this->m_BinaryDir.empty() || 
      this->m_CTestCmd.empty())
    {
    std::string message = "CTEST_SOURCE_DIRECTORY = ";
    message += (!m_SourceDir.empty()) ? m_SourceDir.c_str() : "(Null)";
    message += "\nCTEST_BINARY_DIRECTORY = ";
    message += (!m_BinaryDir.empty()) ? m_BinaryDir.c_str() : "(Null)";
    message += "\nCTEST_CMAKE_COMMAND = ";
    message += (!m_CTestCmd.empty()) ? m_CTestCmd.c_str() : "(Null)";
    cmSystemTools::Error(
      "Some required settings in the configuration file were missing:\n",
      message.c_str());
    return 4;
    }
  
  // if the dashboard root isn't specified then we can compute it from the
  // m_SourceDir
  if (m_Makefile->GetDefinition("CTEST_DASHBOARD_ROOT"))
    {
    m_CTestRoot = m_Makefile->GetDefinition("CTEST_DASHBOARD_ROOT");
    }
  else
    {
    m_CTestRoot = cmSystemTools::GetFilenamePath(m_SourceDir).c_str();
    }

  // the script may override the minimum continuous interval
  if (m_Makefile->GetDefinition("CTEST_CONTINUOUS_MINIMUM_INTERVAL"))
    {
    m_MinimumInterval = 60*
      atof(m_Makefile->GetDefinition("CTEST_CONTINUOUS_MINIMUM_INTERVAL"));
    }
  
  m_CVSCmd.clear();
  if (m_Makefile->GetDefinition("CTEST_CVS_COMMAND"))
    {
    m_CVSCmd = m_Makefile->GetDefinition("CTEST_CVS_COMMAND");
    }
  
  return 0;
}

//----------------------------------------------------------------------
void cmCTestScriptHandler::LocalSleep(unsigned int secondsToWait)
{
#if defined(_WIN32)
        Sleep(1000*secondsToWait);
#else
        sleep(secondsToWait);
#endif
}

//----------------------------------------------------------------------
// run a specific script
int cmCTestScriptHandler::RunConfigurationScript(
  const std::string& total_script_arg)
{
  int result;
  
  // read in the script
  result = this->ReadInScript(total_script_arg);
  if (result)
    {
    return result;
    }
  
  // no popup widows
  cmSystemTools::SetRunCommandHideConsole(true);
  
  // extract the vars from the cache and store in ivars
  result = this->ExtractVariables();
  if (result)
    {
    return result;
    }
  
  // set any environment variables
  const char *ctestEnv = m_Makefile->GetDefinition("CTEST_ENVIRONMENT");
  if (ctestEnv)
    {
    std::vector<std::string> envArgs;
    cmSystemTools::ExpandListArgument(ctestEnv,envArgs);
    // for each variable/argument do a putenv
    for (unsigned i = 0; i < envArgs.size(); ++i)
      {
      cmSystemTools::PutEnv(envArgs[i].c_str());
      }
    }

  // now that we have done most of the error checking finally run the
  // dashboard, we may be asked to repeatedly run this dashboard, such as
  // for a continuous, do we ned to run it more than once?
  if (m_Makefile->GetDefinition("CTEST_CONTINUOUS_DURATION"))
    {
    double ending_time  = cmSystemTools::GetTime() + 
      60.0*atof(m_Makefile->GetDefinition("CTEST_CONTINUOUS_DURATION"));
    if (m_Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE"))
      {
      m_Makefile->AddDefinition("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY","1");
      }
    do
      {
      double interval = cmSystemTools::GetTime();
      result = this->RunConfigurationDashboard();
      interval = cmSystemTools::GetTime() - interval;
      if (interval < m_MinimumInterval)
        {
        this->LocalSleep(
          static_cast<unsigned int>(m_MinimumInterval - interval));
        }
      if (m_Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE"))
        {
        m_Makefile->AddDefinition("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY",
                                  "0");
        }
      }
    while (cmSystemTools::GetTime() < ending_time);
    }
  // otherwise just run it once
  else
    {
    result = this->RunConfigurationDashboard();
    }

  return result;
}

//----------------------------------------------------------------------
int cmCTestScriptHandler::CheckOutSourceDir()
{
  std::string command;
  std::string output;
  int retVal;
  bool res; 

  if (!cmSystemTools::FileExists(m_SourceDir.c_str()) && 
      !m_CVSCheckOut.empty())
    {
    // we must now checkout the src dir
    output = "";
    if ( m_Verbose )
      {
      std::cerr << "Run cvs: " << m_CVSCheckOut << std::endl;
      }
    res = cmSystemTools::RunSingleCommand(m_CVSCheckOut.c_str(), &output, 
                                          &retVal, m_CTestRoot.c_str(),
                                          m_Verbose, 0 /*m_TimeOut*/);
    if (!res || retVal != 0)
      {
      cmSystemTools::Error("Unable to perform cvs checkout:\n", 
                           output.c_str());    
      return 6;
      }
    }
  return 0;
}

//----------------------------------------------------------------------
int cmCTestScriptHandler::BackupDirectories()
{
  int retVal;

  // compute the backup names
  m_BackupSourceDir = m_SourceDir;
  m_BackupSourceDir += "_CMakeBackup";
  m_BackupBinaryDir = m_BinaryDir;
  m_BackupBinaryDir += "_CMakeBackup";
  
  // backup the binary and src directories if requested
  if (m_Backup)
    {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(m_BackupSourceDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_BackupSourceDir.c_str());
      }
    if (cmSystemTools::FileExists(m_BackupBinaryDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_BackupBinaryDir.c_str());
      }
    
    // first rename the src and binary directories 
    rename(m_SourceDir.c_str(), m_BackupSourceDir.c_str());
    rename(m_BinaryDir.c_str(), m_BackupBinaryDir.c_str());
    
    // we must now checkout the src dir
    retVal = this->CheckOutSourceDir();
    if (retVal)
      {
      this->RestoreBackupDirectories();
      return retVal;
      }
    }

  return 0;
}


//----------------------------------------------------------------------
int cmCTestScriptHandler::PerformExtraUpdates()
{
  std::string command;
  std::string output;
  int retVal;
  bool res; 

  // do an initial cvs update as required
  if (!m_CVSCmd.empty())
    {
    command = m_CVSCmd;
    char updateVar[40];
    int i;
    for (i = 1; i < 10; ++i)
      {
      sprintf(updateVar,"CTEST_EXTRA_UPDATES_%i",i);
      const char *updateVal = m_Makefile->GetDefinition(updateVar);
      if (updateVal)
        {
        std::vector<std::string> cvsArgs;
        cmSystemTools::ExpandListArgument(updateVal,cvsArgs);
        if (cvsArgs.size() == 2)
          {
          std::string fullCommand = command;
          fullCommand += " update ";
          fullCommand += cvsArgs[1];
          output = "";
          retVal = 0;
          if ( m_Verbose )
            {
            std::cerr << "Run CVS: " << fullCommand.c_str() << std::endl;
            }
          res = cmSystemTools::RunSingleCommand(fullCommand.c_str(), &output, 
            &retVal, cvsArgs[0].c_str(),
            m_Verbose, 0 /*m_TimeOut*/);
          if (!res || retVal != 0)
            {
            cmSystemTools::Error("Unable to perform extra cvs updates:\n", 
                                 output.c_str());
            this->RestoreBackupDirectories();
            return 8;
            }
          }
        }
      }
    }
  return 0;
}


//----------------------------------------------------------------------
// run a single dashboard entry
int cmCTestScriptHandler::RunConfigurationDashboard()
{
  // local variables
  std::string command;
  std::string output;
  int retVal;
  bool res; 

  // make sure the src directory is there, if it isn't then we might be able
  // to check it out from cvs
  retVal = this->CheckOutSourceDir();
  if (retVal)
    {
    return retVal;
    }

  // backup the dirs if requested
  retVal = this->BackupDirectories();
  if (retVal)
    {
    return retVal;
    }
  
  // clear the binary directory?
  if (m_Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY"))
    {
    // try to avoid deleting directories that we shouldn't
    std::string check = m_BinaryDir;
    check += "/CMakeCache.txt";
    if (cmSystemTools::FileExists(check.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_BinaryDir.c_str());
      }
    }
  
  // make sure the binary directory exists if it isn't the srcdir
  if (!cmSystemTools::FileExists(m_BinaryDir.c_str()) && 
      m_SourceDir != m_BinaryDir)
    {
    if (!cmSystemTools::MakeDirectory(m_BinaryDir.c_str()))
      {
      cmSystemTools::Error("Unable to create the binary directory:\n", 
                           m_BinaryDir.c_str());    
      this->RestoreBackupDirectories();
      return 7;
      }
    }

  // if the binary directory and the source directory are the same,
  // and we are starting with an empty binary directory, then that means
  // we must check out the source tree
  if (m_Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY") &&
      m_SourceDir == m_BinaryDir)
    {
    // make sure we have the required info
    if (m_CVSCheckOut.empty())
      {
      cmSystemTools::Error("You have specified the source and binary directories to be the same (an in source build). You have also specified that the binary directory is to be erased. This means that the source will have to be checked out from CVS. But you have not specified CTEST_CVS_CHECKOUT");    
      return 8;
      }
    
    // we must now checkout the src dir
    retVal = this->CheckOutSourceDir();
    if (retVal)
      {
      this->RestoreBackupDirectories();
      return retVal;
      }
    }
  
  // backup the dirs if requested
  retVal = this->PerformExtraUpdates();
  if (retVal)
    {
    return retVal;
    }

  // put the initial cache into the bin dir
  if (m_Makefile->GetDefinition("CTEST_INITIAL_CACHE"))
    {
    const char *initCache = m_Makefile->GetDefinition("CTEST_INITIAL_CACHE");
    std::string cacheFile = m_BinaryDir;
    cacheFile += "/CMakeCache.txt";
    std::ofstream fout(cacheFile.c_str());
    if(!fout)
      {
      this->RestoreBackupDirectories();
      return 9;
      }

    fout.write(initCache, strlen(initCache));

    // Make sure the operating system has finished writing the file
    // before closing it.  This will ensure the file is finished before
    // the check below.
    fout.flush();
    fout.close();
    }

  // do an initial cmake to setup the DartConfig file
  const char *cmakeCmd = m_Makefile->GetDefinition("CTEST_CMAKE_COMMAND");
  int cmakeFailed = 0;
  std::string cmakeFailedOuput;
  if (cmakeCmd)
    {
    command = cmakeCmd;
    command += " \"";
    command += m_SourceDir;
    output = "";
    command += "\"";
    retVal = 0;
    if ( m_Verbose )
      {
      std::cerr << "Run cmake command: " << command.c_str() << std::endl;
      }
    res = cmSystemTools::RunSingleCommand(command.c_str(), &output, 
      &retVal, m_BinaryDir.c_str(),
      m_Verbose, 0 /*m_TimeOut*/);
    if (!res || retVal != 0)
      {
      // even if this fails continue to the next step
      cmakeFailed = 1;
      cmakeFailedOuput = output;
      }
    }

  // run ctest, it may be more than one command in here
  std::vector<std::string> ctestCommands;
  cmSystemTools::ExpandListArgument(m_CTestCmd,ctestCommands);
  // for each variable/argument do a putenv
  for (unsigned i = 0; i < ctestCommands.size(); ++i)
    {
    command = ctestCommands[i];
    output = "";
    retVal = 0;
    if ( m_Verbose )
      {
      std::cerr << "Run ctest command: " << command.c_str() << std::endl;
      }
    res = cmSystemTools::RunSingleCommand(command.c_str(), &output, 
                                          &retVal, m_BinaryDir.c_str(),
                                          m_Verbose, 0 /*m_TimeOut*/);
    
    // did something critical fail in ctest
    if (!res || cmakeFailed ||
        retVal & cmCTest::BUILD_ERRORS)
      {
      this->RestoreBackupDirectories();
      if (cmakeFailed)
        {
        cmSystemTools::Error("Unable to run cmake:\n", 
                             cmakeFailedOuput.c_str());    
        return 10;
        }
      cmSystemTools::Error("Unable to run ctest:\n", output.c_str());    
      if (!res)
        {
        return 11;
        }
      return retVal * 100;
      }
    }
  
  // if all was succesful, delete the backup dirs to free up disk space
  if (m_Backup)
    {
    cmSystemTools::RemoveADirectory(m_BackupSourceDir.c_str());
    cmSystemTools::RemoveADirectory(m_BackupBinaryDir.c_str());
    }

  return 0;  
}


//-------------------------------------------------------------------------
void cmCTestScriptHandler::RestoreBackupDirectories()
{
  // if we backed up the dirs and the build failed, then restore
  // the backed up dirs
  if (m_Backup)
    {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(m_SourceDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_SourceDir.c_str());
      }
    if (cmSystemTools::FileExists(m_BinaryDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_BinaryDir.c_str());
      }
    // rename the src and binary directories 
    rename(m_BackupSourceDir.c_str(), m_SourceDir.c_str());
    rename(m_BackupBinaryDir.c_str(), m_BinaryDir.c_str());
    }
}


