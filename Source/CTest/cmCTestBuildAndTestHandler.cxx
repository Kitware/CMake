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

#include "cmCTestBuildAndTestHandler.h"

#include "cmSystemTools.h"
#include "cmCTest.h"
#include "cmake.h"
#include "cmGlobalGenerator.h"
#include <cmsys/Process.h>

//----------------------------------------------------------------------
cmCTestBuildAndTestHandler::cmCTestBuildAndTestHandler()
{
  this->BuildTwoConfig         = false;
  this->BuildNoClean           = false;
  this->BuildNoCMake           = false;
  this->Timeout = 0;
}

//----------------------------------------------------------------------
void cmCTestBuildAndTestHandler::Initialize()
{
  this->BuildTargets.erase(
    this->BuildTargets.begin(), this->BuildTargets.end());
  this->Superclass::Initialize();
}

//----------------------------------------------------------------------
const char* cmCTestBuildAndTestHandler::GetOutput()
{
  return this->Output.c_str();
}
//----------------------------------------------------------------------
int cmCTestBuildAndTestHandler::ProcessHandler()
{
  this->Output = "";
  std::string output;
  cmSystemTools::ResetErrorOccuredFlag();
  int retv = this->RunCMakeAndTest(&this->Output);
  cmSystemTools::ResetErrorOccuredFlag();
  return retv;
}

//----------------------------------------------------------------------
int cmCTestBuildAndTestHandler::RunCMake(std::string* outstring,
  cmOStringStream &out, std::string &cmakeOutString, std::string &cwd,
  cmake *cm)
{
  unsigned int k;
  std::vector<std::string> args;
  args.push_back(this->CTest->GetCMakeExecutable());
  args.push_back(this->SourceDir);
  if(this->BuildGenerator.size())
    {
    std::string generator = "-G";
    generator += this->BuildGenerator;
    args.push_back(generator);
    }
  if ( this->CTest->GetConfigType().size() > 0 )
    {
    std::string btype
      = "-DCMAKE_BUILD_TYPE:STRING=" + this->CTest->GetConfigType();
    args.push_back(btype);
    }

  for(k=0; k < this->BuildOptions.size(); ++k)
    {
    args.push_back(this->BuildOptions[k]);
    }
  if (cm->Run(args) != 0)
    {
    out << "Error: cmake execution failed\n";
    out << cmakeOutString << "\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    if(outstring)
      {
      *outstring = out.str();
      }
    else
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, out.str() << std::endl);
      }
    return 1;
    }
  // do another config?
  if(this->BuildTwoConfig)
    {
    if (cm->Run(args) != 0)
      {
      out << "Error: cmake execution failed\n";
      out << cmakeOutString << "\n";
      // return to the original directory
      cmSystemTools::ChangeDirectory(cwd.c_str());
      if(outstring)
        {
        *outstring = out.str();
        }
      else
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE, out.str() << std::endl);
        }
      return 1;
      }
    }
  out << "======== CMake output     ======\n";
  out << cmakeOutString;
  out << "======== End CMake output ======\n";
  return 0;
}

//----------------------------------------------------------------------
void CMakeMessageCallback(const char* m, const char*, bool&, void* s)
{
  std::string* out = (std::string*)s;
  *out += m;
  *out += "\n";
}

//----------------------------------------------------------------------
void CMakeStdoutCallback(const char* m, int len, void* s)
{
  std::string* out = (std::string*)s;
  out->append(m, len);
}

//----------------------------------------------------------------------
int cmCTestBuildAndTestHandler::RunCMakeAndTest(std::string* outstring)
{
  unsigned int k;
  std::string cmakeOutString;
  cmSystemTools::SetErrorCallback(CMakeMessageCallback, &cmakeOutString);
  cmSystemTools::SetStdoutCallback(CMakeStdoutCallback, &cmakeOutString);
  cmOStringStream out;

  // if the generator and make program are not specified then it is an error
  if (!this->BuildGenerator.size() || !this->BuildMakeProgram.size())
    {
    if(outstring)
      {
      *outstring =
        "--build-and-test requires that both the generator and makeprogram "
        "be provided using the --build-generator and --build-makeprogram "
        "command line options. ";
      }
    return 1;
    }

  // make sure the binary dir is there
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  out << "Internal cmake changing into directory: "
    << this->BinaryDir << std::endl;
  if (!cmSystemTools::FileIsDirectory(this->BinaryDir.c_str()))
    {
    cmSystemTools::MakeDirectory(this->BinaryDir.c_str());
    }
  cmSystemTools::ChangeDirectory(this->BinaryDir.c_str());

  // should we cmake?
  cmake cm;
  cm.SetGlobalGenerator(cm.CreateGlobalGenerator(
      this->BuildGenerator.c_str()));

  if(!this->BuildNoCMake)
    {
    // do the cmake step
    if (this->RunCMake(outstring,out,cmakeOutString,cwd,&cm))
      {
      return 1;
      }
    }

  // do the build
  std::vector<std::string>::iterator tarIt;
  if ( this->BuildTargets.size() == 0 )
    {
    this->BuildTargets.push_back("");
    }
  for ( tarIt = this->BuildTargets.begin(); tarIt != this->BuildTargets.end();
    ++ tarIt )
    {
    std::string output;
    int retVal = cm.GetGlobalGenerator()->Build(
      this->SourceDir.c_str(), this->BinaryDir.c_str(),
      this->BuildProject.c_str(), tarIt->c_str(),
      &output, this->BuildMakeProgram.c_str(),
      this->CTest->GetConfigType().c_str(),!this->BuildNoClean, false);

    out << output;
    // if the build failed then return
    if (retVal)
      {
      if(outstring)
        {
        *outstring =  out.str();
        }
      return 1;
      }
    }
  if(outstring)
    {
    *outstring =  out.str();
    }

  // if no test was specified then we are done
  if (!this->TestCommand.size())
    {
    return 0;
    }

  // now run the compiled test if we can find it
  std::vector<std::string> attempted;
  std::vector<std::string> failed;
  std::string tempPath;
  std::string filepath =
    cmSystemTools::GetFilenamePath(this->TestCommand);
  std::string filename =
    cmSystemTools::GetFilenameName(this->TestCommand);
  // if full path specified then search that first
  if (filepath.size())
    {
    tempPath = filepath;
    tempPath += "/";
    tempPath += filename;
    attempted.push_back(tempPath);
    if(this->CTest->GetConfigType().size())
      {
      tempPath = filepath;
      tempPath += "/";
      tempPath += this->CTest->GetConfigType();
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      // If the file is an OSX bundle then the configtyp
      // will be at the start of the path
      tempPath = this->CTest->GetConfigType();
      tempPath += "/";
      tempPath += filepath;
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      }
    }
  // otherwise search local dirs
  else
    {
    attempted.push_back(filename);
    if(this->CTest->GetConfigType().size())
      {
      tempPath = this->CTest->GetConfigType();
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      }
    }
  // if this->ExecutableDirectory is set try that as well
  if (this->ExecutableDirectory.size())
    {
    tempPath = this->ExecutableDirectory;
    tempPath += "/";
    tempPath += this->TestCommand;
    attempted.push_back(tempPath);
    if(this->CTest->GetConfigType().size())
      {
      tempPath = this->ExecutableDirectory;
      tempPath += "/";
      tempPath += this->CTest->GetConfigType();
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      }
    }

  // store the final location in fullPath
  std::string fullPath;

  // now look in the paths we specified above
  for(unsigned int ai=0;
      ai < attempted.size() && fullPath.size() == 0; ++ai)
    {
    // first check without exe extension
    if(cmSystemTools::FileExists(attempted[ai].c_str())
       && !cmSystemTools::FileIsDirectory(attempted[ai].c_str()))
      {
      fullPath = cmSystemTools::CollapseFullPath(attempted[ai].c_str());
      }
    // then try with the exe extension
    else
      {
      failed.push_back(attempted[ai].c_str());
      tempPath = attempted[ai];
      tempPath += cmSystemTools::GetExecutableExtension();
      if(cmSystemTools::FileExists(tempPath.c_str())
         && !cmSystemTools::FileIsDirectory(tempPath.c_str()))
        {
        fullPath = cmSystemTools::CollapseFullPath(tempPath.c_str());
        }
      else
        {
        failed.push_back(tempPath.c_str());
        }
      }
    }

  if(!cmSystemTools::FileExists(fullPath.c_str()))
    {
    out << "Could not find path to executable, perhaps it was not built: "
      << this->TestCommand << "\n";
    out << "tried to find it in these places:\n";
    out << fullPath.c_str() << "\n";
    for(unsigned int i=0; i < failed.size(); ++i)
      {
      out << failed[i] << "\n";
      }
    if(outstring)
      {
      *outstring =  out.str();
      }
    else
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE, out.str());
      }
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  std::vector<const char*> testCommand;
  testCommand.push_back(fullPath.c_str());
  for(k=0; k < this->TestCommandArgs.size(); ++k)
    {
    testCommand.push_back(this->TestCommandArgs[k].c_str());
    }
  testCommand.push_back(0);
  std::string outs;
  int retval = 0;
  // run the test from the this->BuildRunDir if set
  if(this->BuildRunDir.size())
    {
    out << "Run test in directory: " << this->BuildRunDir << "\n";
    cmSystemTools::ChangeDirectory(this->BuildRunDir.c_str());
    }
  out << "Running test executable: " << fullPath << " ";
  for(k=0; k < this->TestCommandArgs.size(); ++k)
    {
    out << this->TestCommandArgs[k] << " ";
    }
  out << "\n";
  int runTestRes = this->CTest->RunTest(testCommand, &outs, &retval, 0, 
                                        this->Timeout);
  if(runTestRes != cmsysProcess_State_Exited || retval != 0)
    {
    out << "Failed to run test command: " << testCommand[0] << "\n";
    retval = 1;
    }

  out << outs << "\n";
  if(outstring)
    {
    *outstring = out.str();
    }
  else
    {
    cmCTestLog(this->CTest, OUTPUT, out.str() << std::endl);
    }
  return retval;
}

//----------------------------------------------------------------------
int cmCTestBuildAndTestHandler::ProcessCommandLineArguments(
  const std::string& currentArg, size_t& idx,
  const std::vector<std::string>& allArgs)
{
  // --build-and-test options
  if(currentArg.find("--build-and-test",0) == 0 && idx < allArgs.size() - 1)
    {
    if(idx+2 < allArgs.size())
      {
      idx++;
      this->SourceDir = allArgs[idx];
      idx++;
      this->BinaryDir = allArgs[idx];
      // dir must exist before CollapseFullPath is called
      cmSystemTools::MakeDirectory(this->BinaryDir.c_str());
      this->BinaryDir
        = cmSystemTools::CollapseFullPath(this->BinaryDir.c_str());
      this->SourceDir
        = cmSystemTools::CollapseFullPath(this->SourceDir.c_str());
      }
    else
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "--build-and-test must have source and binary dir" << std::endl);
      return 0;
      }
    }
  if(currentArg.find("--build-target",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    this->BuildTargets.push_back(allArgs[idx]);
    }
  if(currentArg.find("--build-nocmake",0) == 0)
    {
    this->BuildNoCMake = true;
    }
  if(currentArg.find("--build-run-dir",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    this->BuildRunDir = allArgs[idx];
    }
  if(currentArg.find("--build-two-config",0) == 0)
    {
    this->BuildTwoConfig = true;
    }
  if(currentArg.find("--build-exe-dir",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    this->ExecutableDirectory = allArgs[idx];
    }
  if(currentArg.find("--test-timeout",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    this->Timeout = atof(allArgs[idx].c_str());
    }
  if(currentArg.find("--build-generator",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    this->BuildGenerator = allArgs[idx];
    }
  if(currentArg.find("--build-project",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    this->BuildProject = allArgs[idx];
    }
  if(currentArg.find("--build-makeprogram",0) == 0 &&
    idx < allArgs.size() - 1)
    {
    idx++;
    this->BuildMakeProgram = allArgs[idx];
    }
  if(currentArg.find("--build-noclean",0) == 0)
    {
    this->BuildNoClean = true;
    }
  if(currentArg.find("--build-options",0) == 0 && idx < allArgs.size() - 1)
    {
    ++idx;
    bool done = false;
    while(idx < allArgs.size() && !done)
      {
      this->BuildOptions.push_back(allArgs[idx]);
      if(idx+1 < allArgs.size()
        && (allArgs[idx+1] == "--build-target" ||
          allArgs[idx+1] == "--test-command"))
        {
        done = true;
        }
      else
        {
        ++idx;
        }
      }
    }
  if(currentArg.find("--test-command",0) == 0 && idx < allArgs.size() - 1)
    {
    ++idx;
    this->TestCommand = allArgs[idx];
    while(idx+1 < allArgs.size())
      {
      ++idx;
      this->TestCommandArgs.push_back(allArgs[idx]);
      }
    }
  return 1;
}


