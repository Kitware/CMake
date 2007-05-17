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
#include "cmTryRunCommand.h"
#include "cmCacheManager.h"
#include "cmTryCompileCommand.h"

// cmTryRunCommand
bool cmTryRunCommand::InitialPass(std::vector<std::string> const& argv)
{
  if(argv.size() < 4)
    {
    return false;
    }
    
  if (this->Makefile->IsOn("CMAKE_CROSSCOMPILING"))
    {
    this->SetError("doesn't work when crosscompiling.");
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }

  // build an arg list for TryCompile and extract the runArgs
  std::vector<std::string> tryCompile;
  std::string outputVariable;
  std::string runArgs;
  unsigned int i;
  for (i = 1; i < argv.size(); ++i)
    {
    if (argv[i] == "ARGS")
      {
      ++i;
      while (i < argv.size() && argv[i] != "COMPILE_DEFINITIONS" &&
             argv[i] != "CMAKE_FLAGS")
        {
        runArgs += " ";
        runArgs += argv[i];
        ++i;
        }
      if (i < argv.size())
        {
        tryCompile.push_back(argv[i]);
        }
      } 
    else 
      {
      tryCompile.push_back(argv[i]);
      if (argv[i] == "OUTPUT_VARIABLE")
        {  
        if ( argv.size() <= (i+1) )
          {
          cmSystemTools::Error(
            "OUTPUT_VARIABLE specified but there is no variable");
          return false;
          }
        outputVariable = argv[i+1];
        }
      }
    }
  // do the try compile
  int res = cmTryCompileCommand::CoreTryCompileCode(this->Makefile, 
                                                    tryCompile, false);
  
  // now try running the command if it compiled
  std::string binaryDirectory = argv[2];
  binaryDirectory += cmake::GetCMakeFilesDirectory();
  binaryDirectory += "/CMakeTmp";
  if (!res)
    {
    int retVal = -1;
    std::string output;
    std::string executableSuffix=this->Makefile->GetDefinition("CMAKE_EXECUTABLE_SUFFIX");
    std::string command1 = binaryDirectory;
    std::vector<std::string> attemptedPaths;
    command1 += "/cmTryCompileExec";
    command1 += executableSuffix;
    std::string fullPath;
    if(cmSystemTools::FileExists(command1.c_str()))
      {
      fullPath = cmSystemTools::CollapseFullPath(command1.c_str());
      }
    attemptedPaths.push_back(command1);
    command1 = binaryDirectory;
    // try CMAKE_TRY_COMPILE_CONFIGURATION if it is set
    if (fullPath.empty())
      {
      const char* config = 
        this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");
      // if a config was specified try that first
      if (config && config[0])
        {
        command1 += "/";
        command1 += config;
        command1 += "/cmTryCompileExec";
        command1 += executableSuffix;
        if(cmSystemTools::FileExists(command1.c_str()))
          {
          fullPath = cmSystemTools::CollapseFullPath(command1.c_str());
          }
        attemptedPaths.push_back(command1);
        }
      }
    // try Debug if still not found
    if (fullPath.empty())
      {
      command1 = binaryDirectory;
      command1 += "/Debug/cmTryCompileExec";
      command1 += executableSuffix;
      if(cmSystemTools::FileExists(command1.c_str()))
        {
        fullPath = cmSystemTools::CollapseFullPath(command1.c_str());
        }
      attemptedPaths.push_back(command1);
      }
    // try Deployment if still not found
    if (fullPath.empty())
      {
      command1 = binaryDirectory;
      command1 += "/Development/cmTryCompileExec";
      command1 += executableSuffix;
      if(cmSystemTools::FileExists(command1.c_str()))
        {
        fullPath = cmSystemTools::CollapseFullPath(command1.c_str());
        }
      attemptedPaths.push_back(command1);
      }
    if (fullPath.empty())
      {
      cmOStringStream emsg;
      emsg << "Unable to find executable for TRY_RUN: tried \"";
      for (i = 0; i < attemptedPaths.size(); ++i)
        {
        emsg << attemptedPaths[i];
        if (i < attemptedPaths.size() - 1)
          {
          emsg << "\" and \"";
          }
        else
          {
          emsg << "\".";
          }
        }
      cmSystemTools::Error(emsg.str().c_str());
      }
    if (fullPath.size() > 1)
      {
      std::string finalCommand = fullPath;
      finalCommand = cmSystemTools::ConvertToRunCommandPath(fullPath.c_str());
      if(runArgs.size())
        {
        finalCommand += runArgs;
        }
      int timeout = 0;
      bool worked = cmSystemTools::RunSingleCommand(finalCommand.c_str(),
                                                    &output, &retVal,
                                                    0, false, timeout);
      if(outputVariable.size())
        {
        // if the TryCompileCore saved output in this outputVariable then
        // prepend that output to this output
        const char* compileOutput
          = this->Makefile->GetDefinition(outputVariable.c_str());
        if(compileOutput)
          {
          output = std::string(compileOutput) + output;
          }
        this->Makefile->AddDefinition(outputVariable.c_str(), output.c_str());
        }
      // set the run var
      char retChar[1000];
      if(worked)
        {
        sprintf(retChar,"%i",retVal);
        }
      else
        {
        strcpy(retChar, "FAILED_TO_RUN");
        }
      this->Makefile->AddCacheDefinition(argv[0].c_str(), retChar,
                                     "Result of TRY_RUN",
                                         cmCacheManager::INTERNAL);
      }
    }
  
  // if we created a directory etc, then cleanup after ourselves  
  std::string cacheFile = binaryDirectory;
  cacheFile += "/CMakeLists.txt";
  if(!this->Makefile->GetCMakeInstance()->GetDebugTryCompile())
    {
    cmTryCompileCommand::CleanupFiles(binaryDirectory.c_str());
    }
  return true;
}


      
