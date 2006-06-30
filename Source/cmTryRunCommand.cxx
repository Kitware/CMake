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

// cmExecutableCommand
bool cmTryRunCommand::InitialPass(std::vector<std::string> const& argv)
{
  if(argv.size() < 4)
    {
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
    std::string command1 = binaryDirectory;
    command1 += "/cmTryCompileExec";
    command1 += cmSystemTools::GetExecutableExtension();
    std::string fullPath;
    if(cmSystemTools::FileExists(command1.c_str()))
      {
      fullPath = cmSystemTools::CollapseFullPath(command1.c_str());
      }
    else
      {
      std::string command2 = binaryDirectory;
      command2 += "/Debug/cmTryCompileExec";
      command2 += cmSystemTools::GetExecutableExtension();
      if(cmSystemTools::FileExists(command2.c_str()))
        {
        fullPath = cmSystemTools::CollapseFullPath(command2.c_str());
        }
      else
        {
        std::string command3 = binaryDirectory;
        command3 += "/Development/cmTryCompileExec";
        command3 += cmSystemTools::GetExecutableExtension();
        if(cmSystemTools::FileExists(command3.c_str()))
          {
          fullPath = cmSystemTools::CollapseFullPath(command3.c_str());
          }
        else
          {
          cmOStringStream emsg;
          emsg << "Unable to find executable for TRY_RUN: tried \""
               << command1 << "\" and \""
               << command2 << "\" and \""
               << command3 << "\".";
          cmSystemTools::Error(emsg.str().c_str());
          }
        }
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


      
