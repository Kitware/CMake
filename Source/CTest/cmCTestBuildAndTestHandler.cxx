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
  m_BuildTwoConfig         = false;
  m_BuildNoClean           = false;
  m_BuildNoCMake           = false;
}

//----------------------------------------------------------------------
void cmCTestBuildAndTestHandler::Initialize()
{
  m_BuildTargets.erase(m_BuildTargets.begin(), m_BuildTargets.end());
  this->Superclass::Initialize();
}

//----------------------------------------------------------------------
const char* cmCTestBuildAndTestHandler::GetOutput()
{
  return m_Output.c_str();
}
//----------------------------------------------------------------------
int cmCTestBuildAndTestHandler::ProcessHandler()
{
  m_Output = "";
  std::string output;
  cmSystemTools::ResetErrorOccuredFlag();
  int retv = this->RunCMakeAndTest(&m_Output);
  cmSystemTools::ResetErrorOccuredFlag();
  return retv;
}

//----------------------------------------------------------------------
int cmCTestBuildAndTestHandler::RunCMake(std::string* outstring, cmOStringStream &out,
                      std::string &cmakeOutString, std::string &cwd,
                      cmake *cm)
{
  unsigned int k;
  std::vector<std::string> args;
  args.push_back(m_CTest->GetCMakeExecutable());
  args.push_back(m_SourceDir);
  if(m_BuildGenerator.size())
    {
    std::string generator = "-G";
    generator += m_BuildGenerator;
    args.push_back(generator);
    }
  if ( m_CTest->GetConfigType().size() > 0 )
    {
    std::string btype = "-DCMAKE_BUILD_TYPE:STRING=" + m_CTest->GetConfigType();
    args.push_back(btype);
    }

  for(k=0; k < m_BuildOptions.size(); ++k)
    {
    args.push_back(m_BuildOptions[k]);
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
      cmCTestLog(m_CTest, ERROR_MESSAGE, out.str() << std::endl);
      }
    return 1;
    }
  // do another config?
  if(m_BuildTwoConfig)
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
        cmCTestLog(m_CTest, ERROR_MESSAGE, out.str() << std::endl);
        }
      return 1;
      }
    } 
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
  // What is this? double timeout = m_CTest->GetTimeOut();
  int retVal = 0;
  
  // if the generator and make program are not specified then it is an error
  if (!m_BuildGenerator.size() || !m_BuildMakeProgram.size())
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
  out << "Internal cmake changing into directory: " << m_BinaryDir << "\n";  
  if (!cmSystemTools::FileIsDirectory(m_BinaryDir.c_str()))
    {
    cmSystemTools::MakeDirectory(m_BinaryDir.c_str());
    }
  cmSystemTools::ChangeDirectory(m_BinaryDir.c_str());

  // should we cmake?
  cmake cm;
  cm.SetGlobalGenerator(cm.CreateGlobalGenerator(m_BuildGenerator.c_str()));
    
  if(!m_BuildNoCMake)
    {
    // do the cmake step
    if (this->RunCMake(outstring,out,cmakeOutString,cwd,&cm))
      {
      return 1;
      }
    }

  // do the build
  std::vector<std::string>::iterator tarIt;
  if ( m_BuildTargets.size() == 0 )
    {
    m_BuildTargets.push_back("");
    }
  for ( tarIt = m_BuildTargets.begin(); tarIt != m_BuildTargets.end();
    ++ tarIt )
    {
    std::string output;
    retVal = cm.GetGlobalGenerator()->Build(
      m_SourceDir.c_str(), m_BinaryDir.c_str(),
      m_BuildProject.c_str(), tarIt->c_str(),
      &output, m_BuildMakeProgram.c_str(),
      m_CTest->GetConfigType().c_str(),!m_BuildNoClean);

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
  
  // if not test was specified then we are done
  if (!m_TestCommand.size())
    {
    return 0;
    }
  
  // now run the compiled test if we can find it
  std::vector<std::string> attempted;
  std::vector<std::string> failed;
  std::string tempPath;
  std::string filepath = 
    cmSystemTools::GetFilenamePath(m_TestCommand);
  std::string filename = 
    cmSystemTools::GetFilenameName(m_TestCommand);
  // if full path specified then search that first
  if (filepath.size())
    {
    tempPath = filepath;
    tempPath += "/";
    tempPath += filename;
    attempted.push_back(tempPath);
    if(m_CTest->GetConfigType().size())
      {
      tempPath = filepath;
      tempPath += "/";
      tempPath += m_CTest->GetConfigType();
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      // If the file is an OSX bundle then the configtyp
      // will be at the start of the path
      tempPath = m_CTest->GetConfigType();
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
    if(m_CTest->GetConfigType().size())
      {
      tempPath = m_CTest->GetConfigType();
      tempPath += "/";
      tempPath += filename;
      attempted.push_back(tempPath);
      }
    }
  // if m_ExecutableDirectory is set try that as well
  if (m_ExecutableDirectory.size())
    {
    tempPath = m_ExecutableDirectory;
    tempPath += "/";
    tempPath += m_TestCommand;
    attempted.push_back(tempPath);
    if(m_CTest->GetConfigType().size())
      {
      tempPath = m_ExecutableDirectory;
      tempPath += "/";
      tempPath += m_CTest->GetConfigType();
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
    out << "Could not find path to executable, perhaps it was not built: " <<
      m_TestCommand << "\n";
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
      cmCTestLog(m_CTest, ERROR_MESSAGE, out.str());
      }
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  std::vector<const char*> testCommand;
  testCommand.push_back(fullPath.c_str());
  for(k=0; k < m_TestCommandArgs.size(); ++k)
    {
    testCommand.push_back(m_TestCommandArgs[k].c_str());
    }
  testCommand.push_back(0);
  std::string outs;
  int retval = 0;
  // run the test from the m_BuildRunDir if set
  if(m_BuildRunDir.size())
    {
    out << "Run test in directory: " << m_BuildRunDir << "\n";
    cmSystemTools::ChangeDirectory(m_BuildRunDir.c_str());
    }
  out << "Running test executable: " << fullPath << " ";
  for(k=0; k < m_TestCommandArgs.size(); ++k)
    {
    out << m_TestCommandArgs[k] << " ";
    }
  out << "\n";
  // What is this? m_TimeOut = timeout;
  int runTestRes = m_CTest->RunTest(testCommand, &outs, &retval, 0);
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
    cmCTestLog(m_CTest, OUTPUT, out.str() << std::endl);
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
      m_SourceDir = allArgs[idx];
      idx++;
      m_BinaryDir = allArgs[idx];
      // dir must exist before CollapseFullPath is called
      cmSystemTools::MakeDirectory(m_BinaryDir.c_str());
      m_BinaryDir = cmSystemTools::CollapseFullPath(m_BinaryDir.c_str());
      m_SourceDir = cmSystemTools::CollapseFullPath(m_SourceDir.c_str());
      }
    else
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "--build-and-test must have source and binary dir" << std::endl);
      return 0;
      }
    }
  if(currentArg.find("--build-target",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    m_BuildTargets.push_back(allArgs[idx]);
    }
  if(currentArg.find("--build-nocmake",0) == 0)
    {
    m_BuildNoCMake = true;
    }
  if(currentArg.find("--build-run-dir",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    m_BuildRunDir = allArgs[idx];
    }
  if(currentArg.find("--build-two-config",0) == 0)
    {
    m_BuildTwoConfig = true;
    }
  if(currentArg.find("--build-exe-dir",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    m_ExecutableDirectory = allArgs[idx];
    }
  if(currentArg.find("--build-generator",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    m_BuildGenerator = allArgs[idx];
    }
  if(currentArg.find("--build-project",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    m_BuildProject = allArgs[idx];
    }
  if(currentArg.find("--build-makeprogram",0) == 0 && idx < allArgs.size() - 1)
    {
    idx++;
    m_BuildMakeProgram = allArgs[idx];
    }
  if(currentArg.find("--build-noclean",0) == 0)
    {
    m_BuildNoClean = true;
    }
  if(currentArg.find("--build-options",0) == 0 && idx < allArgs.size() - 1)
    {
    ++idx;
    bool done = false;
    while(idx < allArgs.size() && !done)
      {
      m_BuildOptions.push_back(allArgs[idx]);
      if(idx+1 < allArgs.size() 
        && (allArgs[idx+1] == "--build-target" || allArgs[idx+1] == "--test-command"))
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
    m_TestCommand = allArgs[idx];
    while(idx+1 < allArgs.size())
      {
      ++idx;
      m_TestCommandArgs.push_back(allArgs[idx]);
      }
    }
  return 1;
}


