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
#include "cmFunctionBlocker.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmGeneratedFileStream.h"

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

#include "cmCTestBuildCommand.h"
#include "cmCTestConfigureCommand.h"
#include "cmCTestCoverageCommand.h"
#include "cmCTestEmptyBinaryDirectoryCommand.h"
#include "cmCTestMemCheckCommand.h"
#include "cmCTestRunScriptCommand.h"
#include "cmCTestSleepCommand.h"
#include "cmCTestStartCommand.h"
#include "cmCTestSubmitCommand.h"
#include "cmCTestTestCommand.h"
#include "cmCTestUpdateCommand.h"

#define CTEST_INITIAL_CMAKE_OUTPUT_FILE_NAME "CTestInitialCMakeOutput.log"

// used to keep elapsed time up to date
class cmCTestScriptFunctionBlocker : public cmFunctionBlocker
{
public:
  cmCTestScriptFunctionBlocker() {}
  virtual ~cmCTestScriptFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf);
  //virtual bool ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf);
  //virtual void ScopeEnded(cmMakefile &mf);

  cmCTestScriptHandler* CTestScriptHandler;
};

// simply update the time and don't block anything
bool cmCTestScriptFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& , cmMakefile &)
{
  this->CTestScriptHandler->UpdateElapsedTime();
  return false;
}

//----------------------------------------------------------------------
cmCTestScriptHandler::cmCTestScriptHandler()
{
  this->Backup = false;
  this->EmptyBinDir = false;
  this->EmptyBinDirOnce = false;
  this->Makefile = 0;
  this->LocalGenerator = 0;
  this->CMake = 0;
  this->GlobalGenerator = 0;

  this->ScriptStartTime = 0;

  // the *60 is becuase the settings are in minutes but GetTime is seconds
  this->MinimumInterval = 30*60;
  this->ContinuousDuration = -1;
}

//----------------------------------------------------------------------
void cmCTestScriptHandler::Initialize()
{
  this->Superclass::Initialize();
  this->Backup = false;
  this->EmptyBinDir = false;
  this->EmptyBinDirOnce = false;

  this->SourceDir = "";
  this->BinaryDir = "";
  this->BackupSourceDir = "";
  this->BackupBinaryDir = "";
  this->CTestRoot = "";
  this->CVSCheckOut = "";
  this->CTestCmd = "";
  this->CVSCmd = "";
  this->CTestEnv = "";
  this->InitCache = "";
  this->CMakeCmd = "";
  this->CMOutFile = "";
  this->ExtraUpdates.clear();

  this->MinimumInterval = 20*60;
  this->ContinuousDuration = -1;

  // what time in seconds did this script start running
  this->ScriptStartTime = 0;

  this->Makefile = 0;
  if (this->LocalGenerator)
    {
    delete this->LocalGenerator;
    }
  this->LocalGenerator = 0;
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    }
  this->GlobalGenerator = 0;
  if (this->CMake)
    {
    delete this->CMake;
    }
}

//----------------------------------------------------------------------
cmCTestScriptHandler::~cmCTestScriptHandler()
{
  // local generator owns the makefile
  this->Makefile = 0;
  if (this->LocalGenerator)
    {
    delete this->LocalGenerator;
    }
  this->LocalGenerator = 0;
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    }
  this->GlobalGenerator = 0;
  if (this->CMake)
    {
    delete this->CMake;
    }
}


//----------------------------------------------------------------------
// just adds an argument to the vector
void cmCTestScriptHandler::AddConfigurationScript(const char *script)
{
  this->ConfigurationScripts.push_back(script);
}


//----------------------------------------------------------------------
// the generic entry point for handling scripts, this routine will run all
// the scripts provides a -S arguments
int cmCTestScriptHandler::ProcessHandler()
{
  int res = 0;
  std::vector<cmStdString>::iterator it;
  for ( it = this->ConfigurationScripts.begin();
        it != this->ConfigurationScripts.end();
        it ++ )
    {
    // for each script run it
    res += this->RunConfigurationScript(
      cmSystemTools::CollapseFullPath(it->c_str()));
    }
  if ( res )
    {
    return -1;
    }
  return 0;
}

void cmCTestScriptHandler::UpdateElapsedTime()
{
  if (this->LocalGenerator)
    {
    // set the current elapsed time
    char timeString[20];
    int itime = static_cast<unsigned int>(cmSystemTools::GetTime()
                                          - this->ScriptStartTime);
    sprintf(timeString,"%i",itime);
    this->LocalGenerator->GetMakefile()->AddDefinition("CTEST_ELAPSED_TIME",
                                                   timeString);
    }
}

//----------------------------------------------------------------------
void cmCTestScriptHandler::AddCTestCommand(cmCTestCommand* command)
{
  cmCTestCommand* newCom = command;
  newCom->CTest = this->CTest;
  newCom->CTestScriptHandler = this;
  this->CMake->AddCommand(newCom);
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
    cmSystemTools::Error("Cannot find file: ", script.c_str());
    return 1;
    }

  // create a cmake instance to read the configuration script
  // read in the list file to fill the cache
  if (this->CMake)
    {
    delete this->CMake;
    delete this->GlobalGenerator;
    delete this->LocalGenerator;
    }
  this->CMake = new cmake;
  this->CMake->AddCMakePaths(this->CTest->GetCTestExecutable());
  this->GlobalGenerator = new cmGlobalGenerator;
  this->GlobalGenerator->SetCMakeInstance(this->CMake);

  this->LocalGenerator = this->GlobalGenerator->CreateLocalGenerator();
  this->LocalGenerator->SetGlobalGenerator(this->GlobalGenerator);
  this->Makefile = this->LocalGenerator->GetMakefile();

  // set a variable with the path to the current script
  this->Makefile->AddDefinition("CTEST_SCRIPT_DIRECTORY",
                            cmSystemTools::GetFilenamePath(script).c_str());
  this->Makefile->AddDefinition("CTEST_SCRIPT_NAME",
                            cmSystemTools::GetFilenameName(script).c_str());
  this->Makefile->AddDefinition("CTEST_EXECUTABLE_NAME",
                            this->CTest->GetCTestExecutable());
  this->Makefile->AddDefinition("CMAKE_EXECUTABLE_NAME",
                            this->CTest->GetCMakeExecutable());
  this->Makefile->AddDefinition("CTEST_RUN_CURRENT_SCRIPT", true);
  this->UpdateElapsedTime();

  // add any ctest specific commands, probably should have common superclass
  // for ctest commands to clean this up. If a couple more commands are
  // created with the same format lets do that - ken
  this->AddCTestCommand(new cmCTestBuildCommand);
  this->AddCTestCommand(new cmCTestConfigureCommand);
  this->AddCTestCommand(new cmCTestCoverageCommand);
  this->AddCTestCommand(new cmCTestEmptyBinaryDirectoryCommand);
  this->AddCTestCommand(new cmCTestMemCheckCommand);
  this->AddCTestCommand(new cmCTestRunScriptCommand);
  this->AddCTestCommand(new cmCTestSleepCommand);
  this->AddCTestCommand(new cmCTestStartCommand);
  this->AddCTestCommand(new cmCTestSubmitCommand);
  this->AddCTestCommand(new cmCTestTestCommand);
  this->AddCTestCommand(new cmCTestUpdateCommand);

  // add the script arg if defined
  if (script_arg.size())
    {
    this->Makefile->AddDefinition("CTEST_SCRIPT_ARG", script_arg.c_str());
    }

  // always add a function blocker to update the elapsed time
  cmCTestScriptFunctionBlocker *f = new cmCTestScriptFunctionBlocker();
  f->CTestScriptHandler = this;
  this->Makefile->AddFunctionBlocker(f);

  // finally read in the script
  if (!this->Makefile->ReadListFile(0, script.c_str()))
    {
    return 2;
    }

  return 0;
}


//----------------------------------------------------------------------
// extract variabels from the script to set ivars
int cmCTestScriptHandler::ExtractVariables()
{
  // Temporary variables
  const char* minInterval;
  const char* contDuration;

  this->SourceDir
    = this->Makefile->GetSafeDefinition("CTEST_SOURCE_DIRECTORY");
  this->BinaryDir
    = this->Makefile->GetSafeDefinition("CTEST_BINARY_DIRECTORY");
  this->CTestCmd
    = this->Makefile->GetSafeDefinition("CTEST_COMMAND");
  this->CVSCheckOut
    = this->Makefile->GetSafeDefinition("CTEST_CVS_CHECKOUT");
  this->CTestRoot
    = this->Makefile->GetSafeDefinition("CTEST_DASHBOARD_ROOT");
  this->CVSCmd
    = this->Makefile->GetSafeDefinition("CTEST_CVS_COMMAND");
  this->CTestEnv
    = this->Makefile->GetSafeDefinition("CTEST_ENVIRONMENT");
  this->InitCache
    = this->Makefile->GetSafeDefinition("CTEST_INITIAL_CACHE");
  this->CMakeCmd
    = this->Makefile->GetSafeDefinition("CTEST_CMAKE_COMMAND");
  this->CMOutFile
    = this->Makefile->GetSafeDefinition("CTEST_CMAKE_OUTPUT_FILE_NAME");

  this->Backup
    = this->Makefile->IsOn("CTEST_BACKUP_AND_RESTORE");
  this->EmptyBinDir
    = this->Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY");
  this->EmptyBinDirOnce
    = this->Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE");

  minInterval
    = this->Makefile->GetDefinition("CTEST_CONTINUOUS_MINIMUM_INTERVAL");
  contDuration
    = this->Makefile->GetDefinition("CTEST_CONTINUOUS_DURATION");

  char updateVar[40];
  int i;
  for (i = 1; i < 10; ++i)
    {
    sprintf(updateVar,"CTEST_EXTRA_UPDATES_%i",i);
    const char *updateVal = this->Makefile->GetDefinition(updateVar);
    if ( updateVal )
      {
      if ( this->CVSCmd.empty() )
        {
        cmSystemTools::Error(updateVar,
          " specified without specifying CTEST_CVS_COMMAND.");
        return 12;
        }
      this->ExtraUpdates.push_back(updateVal);
      }
    }

  // in order to backup and restore we also must have the cvs root
  if (this->Backup && this->CVSCheckOut.empty())
    {
    cmSystemTools::Error(
      "Backup was requested without specifying CTEST_CVS_CHECKOUT.");
    return 3;
    }

  // make sure the required info is here
  if (this->SourceDir.empty() ||
      this->BinaryDir.empty() ||
      this->CTestCmd.empty())
    {
    std::string msg = "CTEST_SOURCE_DIRECTORY = ";
    msg += (!this->SourceDir.empty()) ? this->SourceDir.c_str() : "(Null)";
    msg += "\nCTEST_BINARY_DIRECTORY = ";
    msg += (!this->BinaryDir.empty()) ? this->BinaryDir.c_str() : "(Null)";
    msg += "\nCTEST_COMMAND = ";
    msg += (!this->CTestCmd.empty()) ? this->CTestCmd.c_str() : "(Null)";
    cmSystemTools::Error(
      "Some required settings in the configuration file were missing:\n",
      msg.c_str());
    return 4;
    }

  // if the dashboard root isn't specified then we can compute it from the
  // this->SourceDir
  if (this->CTestRoot.empty() )
    {
    this->CTestRoot = cmSystemTools::GetFilenamePath(this->SourceDir).c_str();
    }

  // the script may override the minimum continuous interval
  if (minInterval)
    {
    this->MinimumInterval = 60 * atof(minInterval);
    }
  if (contDuration)
    {
    this->ContinuousDuration = 60.0 * atof(contDuration);
    }


  this->UpdateElapsedTime();

  return 0;
}

//----------------------------------------------------------------------
void cmCTestScriptHandler::SleepInSeconds(unsigned int secondsToWait)
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

  this->ScriptStartTime =
    cmSystemTools::GetTime();

  // read in the script
  result = this->ReadInScript(total_script_arg);
  if (result)
    {
    return result;
    }

  // only run the curent script if we should
  if (this->Makefile && this->Makefile->IsOn("CTEST_RUN_CURRENT_SCRIPT"))
    {
    return this->RunCurrentScript();
    }
  return result;
}

//----------------------------------------------------------------------
int cmCTestScriptHandler::RunCurrentScript()
{
  int result;

  // do not run twice
  this->Makefile->AddDefinition("CTEST_RUN_CURRENT_SCRIPT", false);

  // no popup widows
  cmSystemTools::SetRunCommandHideConsole(true);

  // extract the vars from the cache and store in ivars
  result = this->ExtractVariables();
  if (result)
    {
    return result;
    }

  // set any environment variables
  if (!this->CTestEnv.empty())
    {
    std::vector<std::string> envArgs;
    cmSystemTools::ExpandListArgument(this->CTestEnv.c_str(),envArgs);
    // for each variable/argument do a putenv
    for (unsigned i = 0; i < envArgs.size(); ++i)
      {
      cmSystemTools::PutEnv(envArgs[i].c_str());
      }
    }

  // now that we have done most of the error checking finally run the
  // dashboard, we may be asked to repeatedly run this dashboard, such as
  // for a continuous, do we ned to run it more than once?
  if ( this->ContinuousDuration >= 0 )
    {
    this->UpdateElapsedTime();
    double ending_time  = cmSystemTools::GetTime() + this->ContinuousDuration;
    if (this->EmptyBinDirOnce)
      {
      this->EmptyBinDir = true;
      }
    do
      {
      double interval = cmSystemTools::GetTime();
      result = this->RunConfigurationDashboard();
      interval = cmSystemTools::GetTime() - interval;
      if (interval < this->MinimumInterval)
        {
        this->SleepInSeconds(
          static_cast<unsigned int>(this->MinimumInterval - interval));
        }
      if (this->EmptyBinDirOnce)
        {
        this->EmptyBinDir = false;
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

  if (!cmSystemTools::FileExists(this->SourceDir.c_str()) &&
      !this->CVSCheckOut.empty())
    {
    // we must now checkout the src dir
    output = "";
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "Run cvs: " << this->CVSCheckOut << std::endl);
    res = cmSystemTools::RunSingleCommand(this->CVSCheckOut.c_str(), &output,
      &retVal, this->CTestRoot.c_str(), this->HandlerVerbose,
      0 /*this->TimeOut*/);
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
  this->BackupSourceDir = this->SourceDir;
  this->BackupSourceDir += "_CMakeBackup";
  this->BackupBinaryDir = this->BinaryDir;
  this->BackupBinaryDir += "_CMakeBackup";

  // backup the binary and src directories if requested
  if (this->Backup)
    {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(this->BackupSourceDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(this->BackupSourceDir.c_str());
      }
    if (cmSystemTools::FileExists(this->BackupBinaryDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(this->BackupBinaryDir.c_str());
      }

    // first rename the src and binary directories
    rename(this->SourceDir.c_str(), this->BackupSourceDir.c_str());
    rename(this->BinaryDir.c_str(), this->BackupBinaryDir.c_str());

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
  command = this->CVSCmd;
  std::vector<cmStdString>::iterator it;
  for (it = this->ExtraUpdates.begin();
    it != this->ExtraUpdates.end();
    ++ it )
    {
    std::vector<std::string> cvsArgs;
    cmSystemTools::ExpandListArgument(it->c_str(),cvsArgs);
    if (cvsArgs.size() == 2)
      {
      std::string fullCommand = command;
      fullCommand += " update ";
      fullCommand += cvsArgs[1];
      output = "";
      retVal = 0;
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Run CVS: "
        << fullCommand.c_str() << std::endl);
      res = cmSystemTools::RunSingleCommand(fullCommand.c_str(), &output,
        &retVal, cvsArgs[0].c_str(),
        this->HandlerVerbose, 0 /*this->TimeOut*/);
      if (!res || retVal != 0)
        {
        cmSystemTools::Error("Unable to perform extra cvs updates:\n",
          output.c_str());
        return 0;
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
  if (this->EmptyBinDir)
    {
    if ( !cmCTestScriptHandler::EmptyBinaryDirectory(
        this->BinaryDir.c_str()) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Problem removing the binary directory" << std::endl);
      }
    }

  // make sure the binary directory exists if it isn't the srcdir
  if (!cmSystemTools::FileExists(this->BinaryDir.c_str()) &&
      this->SourceDir != this->BinaryDir)
    {
    if (!cmSystemTools::MakeDirectory(this->BinaryDir.c_str()))
      {
      cmSystemTools::Error("Unable to create the binary directory:\n",
                           this->BinaryDir.c_str());
      this->RestoreBackupDirectories();
      return 7;
      }
    }

  // if the binary directory and the source directory are the same,
  // and we are starting with an empty binary directory, then that means
  // we must check out the source tree
  if (this->EmptyBinDir && this->SourceDir == this->BinaryDir)
    {
    // make sure we have the required info
    if (this->CVSCheckOut.empty())
      {
      cmSystemTools::Error("You have specified the source and binary "
        "directories to be the same (an in source build). You have also "
        "specified that the binary directory is to be erased. This means "
        "that the source will have to be checked out from CVS. But you have "
        "not specified CTEST_CVS_CHECKOUT");
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
  if (!this->InitCache.empty())
    {
    std::string cacheFile = this->BinaryDir;
    cacheFile += "/CMakeCache.txt";
    cmGeneratedFileStream fout(cacheFile.c_str());
    if(!fout)
      {
      this->RestoreBackupDirectories();
      return 9;
      }

    fout.write(this->InitCache.c_str(), this->InitCache.size());

    // Make sure the operating system has finished writing the file
    // before closing it.  This will ensure the file is finished before
    // the check below.
    fout.flush();
    fout.close();
    }

  // do an initial cmake to setup the DartConfig file
  int cmakeFailed = 0;
  std::string cmakeFailedOuput;
  if (!this->CMakeCmd.empty())
    {
    command = this->CMakeCmd;
    command += " \"";
    command += this->SourceDir;
    output = "";
    command += "\"";
    retVal = 0;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Run cmake command: "
      << command.c_str() << std::endl);
    res = cmSystemTools::RunSingleCommand(command.c_str(), &output,
      &retVal, this->BinaryDir.c_str(),
      this->HandlerVerbose, 0 /*this->TimeOut*/);

    if ( !this->CMOutFile.empty() )
      {
      std::string cmakeOutputFile = this->CMOutFile;
      if ( !cmSystemTools::FileIsFullPath(cmakeOutputFile.c_str()) )
        {
        cmakeOutputFile = this->BinaryDir + "/" + cmakeOutputFile;
        }

      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        "Write CMake output to file: " << cmakeOutputFile.c_str()
        << std::endl);
      cmGeneratedFileStream fout(cmakeOutputFile.c_str());
      if ( fout )
        {
        fout << output.c_str();
        }
      else
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "Cannot open CMake output file: "
          << cmakeOutputFile.c_str() << " for writing" << std::endl);
        }
      }
    if (!res || retVal != 0)
      {
      // even if this fails continue to the next step
      cmakeFailed = 1;
      cmakeFailedOuput = output;
      }
    }

  // run ctest, it may be more than one command in here
  std::vector<std::string> ctestCommands;
  cmSystemTools::ExpandListArgument(this->CTestCmd,ctestCommands);
  // for each variable/argument do a putenv
  for (unsigned i = 0; i < ctestCommands.size(); ++i)
    {
    command = ctestCommands[i];
    output = "";
    retVal = 0;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Run ctest command: "
      << command.c_str() << std::endl);
    res = cmSystemTools::RunSingleCommand(command.c_str(), &output,
      &retVal, this->BinaryDir.c_str(), this->HandlerVerbose,
      0 /*this->TimeOut*/);

    // did something critical fail in ctest
    if (!res || cmakeFailed ||
        retVal & cmCTest::BUILD_ERRORS)
      {
      this->RestoreBackupDirectories();
      if (cmakeFailed)
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "Unable to run cmake:" << std::endl
          << cmakeFailedOuput.c_str() << std::endl);
        return 10;
        }
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Unable to run ctest:" << std::endl
        << output.c_str() << std::endl);
      if (!res)
        {
        return 11;
        }
      return retVal * 100;
      }
    }

  // if all was succesful, delete the backup dirs to free up disk space
  if (this->Backup)
    {
    cmSystemTools::RemoveADirectory(this->BackupSourceDir.c_str());
    cmSystemTools::RemoveADirectory(this->BackupBinaryDir.c_str());
    }

  return 0;
}


//-------------------------------------------------------------------------
void cmCTestScriptHandler::RestoreBackupDirectories()
{
  // if we backed up the dirs and the build failed, then restore
  // the backed up dirs
  if (this->Backup)
    {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(this->SourceDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(this->SourceDir.c_str());
      }
    if (cmSystemTools::FileExists(this->BinaryDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(this->BinaryDir.c_str());
      }
    // rename the src and binary directories
    rename(this->BackupSourceDir.c_str(), this->SourceDir.c_str());
    rename(this->BackupBinaryDir.c_str(), this->BinaryDir.c_str());
    }
}

bool cmCTestScriptHandler::RunScript(cmCTest* ctest, const char *sname)
{
  cmCTestScriptHandler* sh = new cmCTestScriptHandler();
  sh->SetCTestInstance(ctest);
  sh->AddConfigurationScript(sname);
  sh->ProcessHandler();
  delete sh;
  return true;
}

bool cmCTestScriptHandler::EmptyBinaryDirectory(const char *sname)
{
  // try to avoid deleting root
  if (!sname || strlen(sname) < 2)
    {
    return false;
    }

  // try to avoid deleting directories that we shouldn't
  std::string check = sname;
  check += "/CMakeCache.txt";
  if(cmSystemTools::FileExists(check.c_str()) &&
     !cmSystemTools::RemoveADirectory(sname))
    {
    return false;
    }
  return true;
}
