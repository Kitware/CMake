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

  // build an arg list for TryCompile and extract the runArgs
  std::vector<std::string> tryCompile;

  this->CompileResultVariable = "";
  this->RunResultVariable = "";
  this->OutputVariable = "";
  this->RunOutputVariable = "";
  this->CompileOutputVariable = "";

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
      if (argv[i] == "OUTPUT_VARIABLE")
        {
        if ( argv.size() <= (i+1) )
          {
          cmSystemTools::Error(
            "OUTPUT_VARIABLE specified but there is no variable");
          return false;
          }
        i++;
        this->OutputVariable = argv[i];
        }
      else if (argv[i] == "RUN_OUTPUT_VARIABLE")
        {
        if (argv.size() <= (i + 1))
          {
          cmSystemTools::Error(
            "RUN_OUTPUT_VARIABLE specified but there is no variable");
          return false;
          }
        i++;
        this->RunOutputVariable = argv[i];
        }
      else if (argv[i] == "COMPILE_OUTPUT_VARIABLE")
        {
        if (argv.size() <= (i + 1))
          {
          cmSystemTools::Error(
            "COMPILE_OUTPUT_VARIABLE specified but there is no variable");
          return false;
          }
        i++;
        this->CompileOutputVariable = argv[i];
        }
      else
        {
        tryCompile.push_back(argv[i]);
        }
      }
    }

  // although they could be used together, don't allow it, because
  // using OUTPUT_VARIABLE makes crosscompiling harder
  if (this->OutputVariable.size() 
      && ((this->RunOutputVariable.size()) 
       || (this->CompileOutputVariable.size())))
    {
    cmSystemTools::Error(
      "You cannot use OUTPUT_VARIABLE together with COMPILE_OUTPUT_VARIABLE "
      "or RUN_OUTPUT_VARIABLE. Please use only COMPILE_OUTPUT_VARIABLE and/or "
      "RUN_OUTPUT_VARIABLE.");
    return false;
    }

  bool captureRunOutput = false;
  if (this->OutputVariable.size())
    {
    captureRunOutput = true;
    tryCompile.push_back("OUTPUT_VARIABLE");
    tryCompile.push_back(this->OutputVariable);
    }
  if (this->CompileOutputVariable.size())
    {
    tryCompile.push_back("OUTPUT_VARIABLE");
    tryCompile.push_back(this->CompileOutputVariable);
    }
  if (this->RunOutputVariable.size())
    {
    captureRunOutput = true;
    }

  this->RunResultVariable = argv[0];
  this->CompileResultVariable = argv[1];

  // do the try compile
  int res = this->TryCompileCode(tryCompile);

  // now try running the command if it compiled
  if (!res)
    {
    if (this->OutputFile.size() == 0)
      {
      cmSystemTools::Error(this->FindErrorMessage.c_str());
      }
    else
      {
      // "run" it and capture the output
      std::string runOutputContents;
      if (this->Makefile->IsOn("CMAKE_CROSSCOMPILING"))
        {
        this->DoNotRunExecutable(runArgs, 
                                 argv[3], 
                                 captureRunOutput ? &runOutputContents : 0);
        }
      else
        {
        this->RunExecutable(runArgs, &runOutputContents);
        }

      // now put the output into the variables
      if(this->RunOutputVariable.size())
        {
        this->Makefile->AddDefinition(this->RunOutputVariable.c_str(), 
                                      runOutputContents.c_str());
        }

      if(this->OutputVariable.size())
        {
        // if the TryCompileCore saved output in this outputVariable then
        // prepend that output to this output
        const char* compileOutput
                 = this->Makefile->GetDefinition(this->OutputVariable.c_str());
        if (compileOutput)
          {
          runOutputContents = std::string(compileOutput) + runOutputContents;
          }
        this->Makefile->AddDefinition(this->OutputVariable.c_str(), 
                                      runOutputContents.c_str());
        }
      }
    }

  // if we created a directory etc, then cleanup after ourselves
  if(!this->Makefile->GetCMakeInstance()->GetDebugTryCompile())
    {
    this->CleanupFiles(this->BinaryDirectory.c_str());
    }
  return true;
}

void cmTryRunCommand::RunExecutable(const std::string& runArgs,
                                    std::string* out)
{
  int retVal = -1;
  std::string finalCommand = cmSystemTools::ConvertToRunCommandPath(
                               this->OutputFile.c_str());
  if (runArgs.size())
    {
    finalCommand += runArgs;
    }
  int timeout = 0;
  bool worked = cmSystemTools::RunSingleCommand(finalCommand.c_str(),
                out, &retVal,
                0, false, timeout);
  // set the run var
  char retChar[1000];
  if (worked)
    {
    sprintf(retChar, "%i", retVal);
    }
  else
    {
    strcpy(retChar, "FAILED_TO_RUN");
    }
  this->Makefile->AddCacheDefinition(this->RunResultVariable.c_str(), retChar,
                                     "Result of TRY_RUN",
                                     cmCacheManager::INTERNAL);
}

/* This is only used when cross compiling. Instead of running the
 executable, two cache variables are created which will hold the results
 the executable would have produced. 
*/
void cmTryRunCommand::DoNotRunExecutable(const std::string& runArgs, 
                                    const std::string& srcFile,
                                    std::string* out
                                    )
{

  std::string internalRunOutputName = this->RunResultVariable+"__"
                                +this->CompileResultVariable+"__TRYRUN_OUTPUT";
  bool error = false;
  std::string info = "Source file: ";
  info += srcFile + "\n";
  if (runArgs.size())
    {
    info += "Run arguments: ";
    info += runArgs;
    info += "\n";
    }
  info += "Current CMake stack: " + this->Makefile->GetListFileStack();

  if (this->Makefile->GetDefinition(this->RunResultVariable.c_str()) == 0)
    {
    // if the variables doesn't exist, create it with a helpful error text
    // and mark it as advanced
    std::string comment;
    comment += "Run result of TRY_RUN().\n"
               "This variable should indicate whether the executable would "
               "have been able to run if it was executed on its target "
               "platform.\n"
               "If it would have been able to run, enter the exit code here "
               "(in many cases 0 for success). If not, enter "
               "\"FAILED_TO_RUN\" here.";
    if (out!=0)
      {
      comment += "If it was able to run, also check the variable ";
      comment += internalRunOutputName;
      comment += " and set it appropriately.";
      }
    comment += "\n";
    comment += info;
    this->Makefile->AddCacheDefinition(this->RunResultVariable.c_str(), 
                                       "PLEASE_FILL_OUT-FAILED_TO_RUN",
                                       comment.c_str(),
                                       cmCacheManager::STRING);

    cmCacheManager::CacheIterator it = this->Makefile->GetCacheManager()->
                             GetCacheIterator(this->RunResultVariable.c_str());
    if ( !it.IsAtEnd() )
      {
      it.SetProperty("ADVANCED", "1");
      }

    error = true;
    }

  if (out!=0)
    {
    if (this->Makefile->GetDefinition(internalRunOutputName.c_str()) == 0)
      {
      // if the variables doesn't exist, create it with a helpful error text
      // and mark it as advanced
      std::string comment;
      comment += "Output of TRY_RUN().\n"
                 "This variable should contain the text, which the executable "
                 "run by TRY_RUN() would have printed on stdout and stderr, "
                 "if it was executed on its target platform.\n"
                 "The accompanying variable ";
      comment += this->RunResultVariable;
      comment += " indicates whether the executable would have been able to "
                 "run and its exit code."
                 "If the executable would not have been able to run, set ";
      comment += internalRunOutputName;
      comment += " empty. Otherwise check if the output is evaluated by the "
                 "calling CMake code. If this is the case, check the source "
                 "file what it would have printed if called with the given "
                 "arguments.\n";
      comment += info;

      this->Makefile->AddCacheDefinition(internalRunOutputName.c_str(), 
                                         "PLEASE_FILL_OUT-NOTFOUND",
                                         comment.c_str(),
                                         cmCacheManager::STRING);
      cmCacheManager::CacheIterator it = this->Makefile->GetCacheManager()->
                               GetCacheIterator(internalRunOutputName.c_str());
      if ( !it.IsAtEnd() )
        {
        it.SetProperty("ADVANCED", "1");
        }

      error = true;
      }
    }

  if (error)
    {
    std::string errorMessage = "TRY_RUN() invoked in cross-compiling mode, "
                               "please set the following cache variables "
                               "appropriatly:\n";
    errorMessage += "   " + this->RunResultVariable + " (advanced)\n";
    if (out!=0)
      {
      errorMessage += "   " + internalRunOutputName + " (advanced)\n";
      }
    errorMessage += info;
    cmSystemTools::Error(errorMessage.c_str());
    return;
    }

  if (out!=0)
    {
    (*out) = this->Makefile->GetDefinition(internalRunOutputName.c_str());
    }
}
