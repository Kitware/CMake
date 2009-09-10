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

#include "cmCTestRunTest.h"
#include "cmCTestMemCheckHandler.h"
#include "cmCTest.h"
#include "cmSystemTools.h"

cmCTestRunTest::cmCTestRunTest()
{
}

cmCTestRunTest::~cmCTestRunTest()
{
}

bool cmCTestRunTest::IsRunning()
{
  return this->TestProcess->IsRunning();
}

//---------------------------------------------------------
//waits .1 sec for output from this process.
void cmCTestRunTest::CheckOutput()
{
  std::string out, err;
  bool running = this->TestProcess->CheckOutput(.1);
  //start our timeout for reading the process output
  double clock_start = cmSystemTools::GetTime();
  int pipe;
  bool gotStdOut = false;
  bool gotStdErr = false;
  while((pipe = this->TestProcess->
        GetNextOutputLine(out, err, gotStdOut, gotStdErr, running) )
        != cmsysProcess_Pipe_Timeout)
    {
    if(pipe == cmsysProcess_Pipe_STDOUT ||
       pipe == cmsysProcess_Pipe_STDERR ||
       pipe == cmsysProcess_Pipe_None)
      {
      if(gotStdErr)
        {
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
                   this->GetIndex() << ": " << err << std::endl);
        this->ProcessOutput += err;
        this->ProcessOutput += "\n";
        }
      if(gotStdOut)
        {
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
                   this->GetIndex() << ": " << out << std::endl);
        this->ProcessOutput += out;
        this->ProcessOutput += "\n";
        }
      if(pipe == cmsysProcess_Pipe_None)
        {
        break;
        }
      }
    gotStdOut = false;
    gotStdErr = false;
    //timeout while reading process output (could denote infinite output)
    if(cmSystemTools::GetTime() - clock_start > .1)
      {
      break;
      }
    }
}

//---------------------------------------------------------
bool cmCTestRunTest::EndTest(size_t completed, size_t total, bool started)
{
  //restore the old environment
  if (this->ModifyEnv)
    {
    cmSystemTools::RestoreEnv(this->OrigEnv);
    }
  this->WriteLogOutputTop(completed, total);
  std::string reason;
  bool passed = true;
  int res = started ? this->TestProcess->GetProcessStatus()
                    : cmsysProcess_State_Error;
  int retVal = this->TestProcess->GetExitValue();
  std::vector<std::pair<cmsys::RegularExpression,
    std::string> >::iterator passIt;
  bool forceFail = false;
  if ( this->TestProperties->RequiredRegularExpressions.size() > 0 )
    {
    bool found = false;
    for ( passIt = this->TestProperties->RequiredRegularExpressions.begin();
          passIt != this->TestProperties->RequiredRegularExpressions.end();
          ++ passIt )
      {
      if ( passIt->first.find(this->ProcessOutput.c_str()) )
        {
        found = true;
        reason = "Required regular expression found.";
        }
      }
    if ( !found )
      { 
      reason = "Required regular expression not found.";
      forceFail = true;
      }
    reason +=  "Regex=["; 
    for ( passIt = this->TestProperties->RequiredRegularExpressions.begin();
          passIt != this->TestProperties->RequiredRegularExpressions.end();
          ++ passIt )
      {
      reason += passIt->second;
      reason += "\n";
      }
    reason += "]";
    }
  if ( this->TestProperties->ErrorRegularExpressions.size() > 0 )
    {
    for ( passIt = this->TestProperties->ErrorRegularExpressions.begin();
          passIt != this->TestProperties->ErrorRegularExpressions.end();
          ++ passIt )
      {
      if ( passIt->first.find(this->ProcessOutput.c_str()) )
        {
        reason = "Error regular expression found in output.";
        reason += " Regex=[";
        reason += passIt->second;
        reason += "]";
        forceFail = true;
        }
      }
    }
  if (res == cmsysProcess_State_Exited)
    {
    bool success = 
      !forceFail &&  (retVal == 0 || 
      this->TestProperties->RequiredRegularExpressions.size());
    if((success && !this->TestProperties->WillFail) 
      || (!success && this->TestProperties->WillFail))
      {
      this->TestResult.Status = cmCTestTestHandler::COMPLETED;
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Passed  " );
      }
    else
      {
      this->TestResult.Status = cmCTestTestHandler::FAILED;
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "***Failed  " << reason );
      }
    }
  else if ( res == cmsysProcess_State_Expired )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "***Timeout");
    this->TestResult.Status = cmCTestTestHandler::TIMEOUT;
    }
  else if ( res == cmsysProcess_State_Exception )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "***Exception: ");
    switch ( retVal )
      {
      case cmsysProcess_Exception_Fault:
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "SegFault");
        this->TestResult.Status = cmCTestTestHandler::SEGFAULT;
        break;
      case cmsysProcess_Exception_Illegal:
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "Illegal");
        this->TestResult.Status = cmCTestTestHandler::ILLEGAL;
        break;
      case cmsysProcess_Exception_Interrupt:
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "Interrupt");
        this->TestResult.Status = cmCTestTestHandler::INTERRUPT;
        break;
      case cmsysProcess_Exception_Numerical:
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "Numerical");
        this->TestResult.Status = cmCTestTestHandler::NUMERICAL;
        break;
      default:
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "Other");
        this->TestResult.Status = cmCTestTestHandler::OTHER_FAULT;
      }
    }
  else // if ( res == cmsysProcess_State_Error )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "***Bad command " << res );
    this->TestResult.Status = cmCTestTestHandler::BAD_COMMAND;
    }

  passed = this->TestResult.Status == cmCTestTestHandler::COMPLETED;
  char buf[1024];
  sprintf(buf, "%6.2f sec", this->TestProcess->GetTotalTime());
  cmCTestLog(this->CTest, HANDLER_OUTPUT, buf << "\n" );
  if ( this->TestHandler->LogFile )
    {
    *this->TestHandler->LogFile << "Test time = " << buf << std::endl;
    }
  this->DartProcessing();
  // if this is doing MemCheck then all the output needs to be put into
  // Output since that is what is parsed by cmCTestMemCheckHandler
  if(!this->TestHandler->MemCheck && started)
    {
    if (this->TestResult.Status == cmCTestTestHandler::COMPLETED)
      {
      this->TestHandler->CleanTestOutput(this->ProcessOutput, 
          static_cast<size_t>
          (this->TestHandler->CustomMaximumPassedTestOutputSize));
      }
    else
      {
      this->TestHandler->CleanTestOutput(this->ProcessOutput,
          static_cast<size_t>
          (this->TestHandler->CustomMaximumFailedTestOutputSize));
      }
    }
  this->TestResult.Reason = reason;
  if (this->TestHandler->LogFile)
    {
    bool pass = true;
    const char* reasonType = "Test Pass Reason";
    if(this->TestResult.Status != cmCTestTestHandler::COMPLETED &&
       this->TestResult.Status != cmCTestTestHandler::NOT_RUN)
      {
      reasonType = "Test Fail Reason";
      pass = false;
      }
    double ttime = this->TestProcess->GetTotalTime();
    int hours = static_cast<int>(ttime / (60 * 60));
    int minutes = static_cast<int>(ttime / 60) % 60;
    int seconds = static_cast<int>(ttime) % 60;
    char buffer[100];
    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
    *this->TestHandler->LogFile
      << "----------------------------------------------------------"
      << std::endl;
    if(this->TestResult.Reason.size())
      {
      *this->TestHandler->LogFile << reasonType << ":\n" 
        << this->TestResult.Reason << "\n";
      }
    else 
      {
      if(pass)
        {
        *this->TestHandler->LogFile << "Test Passed.\n";
        }
      else
        {
        *this->TestHandler->LogFile << "Test Failed.\n";
        }
      }
    *this->TestHandler->LogFile << "\"" << this->TestProperties->Name.c_str()
      << "\" end time: " << this->CTest->CurrentTime() << std::endl
      << "\"" << this->TestProperties->Name.c_str() << "\" time elapsed: "
      << buffer << std::endl
      << "----------------------------------------------------------"
      << std::endl << std::endl;
    }
  if(started)
    {
    this->TestResult.Output = this->ProcessOutput;
    this->TestResult.ReturnValue = this->TestProcess->GetExitValue();
    this->TestResult.CompletionStatus = "Completed";
    this->TestResult.ExecutionTime = this->TestProcess->GetTotalTime();
    this->TestHandler->TestResults.push_back(this->TestResult);

    this->MemCheckPostProcess();
    }
  delete this->TestProcess;
  return passed;
}

//--------------------------------------------------------------
void cmCTestRunTest::MemCheckPostProcess()
{
  if(!this->TestHandler->MemCheck)
    {
    return;
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, this->Index 
             << ": process test output now: "
             << this->TestProperties->Name.c_str() << " "
             << this->TestResult.Name.c_str() << std::endl);
  cmCTestMemCheckHandler * handler = static_cast<cmCTestMemCheckHandler*>
    (this->TestHandler);
  if(handler->MemoryTesterStyle == cmCTestMemCheckHandler::BOUNDS_CHECKER)
    {
    handler->PostProcessBoundsCheckerTest(this->TestResult);
    }
  else if(handler->MemoryTesterStyle == cmCTestMemCheckHandler::PURIFY)
    {
    handler->PostProcessPurifyTest(this->TestResult); 
    }
}

void cmCTestRunTest::SetTestHandler(cmCTestTestHandler * handler)
{
  this->TestHandler = handler;
  this->CTest = handler->CTest;
}

//----------------------------------------------------------------------
// Starts the execution of a test.  Returns once it has started
bool cmCTestRunTest::StartTest()
{
  this->ComputeArguments();
  std::vector<std::string>& args = this->TestProperties->Args;
  this->TestResult.Properties = this->TestProperties;
  this->TestResult.ExecutionTime = 0;
  this->TestResult.ReturnValue = -1;
  this->TestResult.CompletionStatus = "Not Run";
  this->TestResult.Status = cmCTestTestHandler::NOT_RUN;
  this->TestResult.TestCount = this->TestProperties->Index;  
  this->TestResult.Name = this->TestProperties->Name;
  this->TestResult.Path = this->TestProperties->Directory.c_str();
  
  // log and return if we did not find the executable
  if (this->ActualCommand == "")
    {
    this->TestProcess = new cmProcess;
    *this->TestHandler->LogFile << "Unable to find executable: " 
                   << args[1].c_str() << std::endl;
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Unable to find executable: "
               << args[1].c_str() << std::endl);
    this->TestResult.Output = "Unable to find executable: " + args[1];
    this->TestResult.FullCommandLine = "";
    this->TestHandler->TestResults.push_back(this->TestResult);
    return false;
    }
  this->StartTime = this->CTest->CurrentTime();

  return this->CreateProcess(this->TestProperties->Timeout,
                             &this->TestProperties->Environment);
}

void cmCTestRunTest::ComputeArguments()
{
  std::vector<std::string>::const_iterator j = 
    this->TestProperties->Args.begin();
  ++j; // skip test name

  // find the test executable
  if(this->TestHandler->MemCheck)
    {
    cmCTestMemCheckHandler * handler = static_cast<cmCTestMemCheckHandler*>
      (this->TestHandler);
    this->ActualCommand = handler->MemoryTester.c_str();
    }
  else
    {
    this->ActualCommand = 
      this->TestHandler->FindTheExecutable(
      this->TestProperties->Args[1].c_str());
    ++j; //skip the executable (it will be actualCommand)
    }
  this->TestCommand
    = cmSystemTools::ConvertToOutputPath(this->ActualCommand.c_str());

  //Prepends memcheck args to our command string
  this->TestHandler->GenerateTestCommand(this->Arguments);
  for(std::vector<std::string>::iterator i = this->Arguments.begin();
      i != this->Arguments.end(); ++i)
    {
    this->TestCommand += " ";
    this->TestCommand += cmSystemTools::EscapeSpaces(j->c_str());
    }

  for(;j != this->TestProperties->Args.end(); ++j)
    {
    this->TestCommand += " ";
    this->TestCommand += cmSystemTools::EscapeSpaces(j->c_str());
    this->Arguments.push_back(*j);
    }
  this->TestResult.FullCommandLine = this->TestCommand;

  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl
             << this->Index << ": "
             << (this->TestHandler->MemCheck?"MemCheck":"Test") 
             << " command: " << this->TestCommand
             << std::endl);
}

//----------------------------------------------------------------------
void cmCTestRunTest::DartProcessing()
{
  if (!this->ProcessOutput.empty() && 
     this->ProcessOutput.find("<DartMeasurement") != this->ProcessOutput.npos)
    {
    if (this->TestHandler->DartStuff.find(this->ProcessOutput.c_str()))
      {
      std::string dartString = this->TestHandler->DartStuff.match(1);
      // keep searching and replacing until none are left
      while (this->TestHandler->DartStuff1.find(this->ProcessOutput.c_str()))
        {
        // replace the exact match for the string
        cmSystemTools::ReplaceString(this->ProcessOutput,
                         this->TestHandler->DartStuff1.match(1).c_str(), "");
        }
      this->TestResult.RegressionImages
        = this->TestHandler->GenerateRegressionImages(dartString);
      }
    }
}

//----------------------------------------------------------------------
bool cmCTestRunTest::CreateProcess(double testTimeOut,
                     std::vector<std::string>* environment)
{
  this->TestProcess = new cmProcess;
  this->TestProcess->SetId(this->Index);
  this->TestProcess->SetWorkingDirectory(
        this->TestProperties->Directory.c_str());
  this->TestProcess->SetCommand(this->ActualCommand.c_str());
  this->TestProcess->SetCommandArguments(this->Arguments);

  std::vector<std::string> origEnv;
  this->ModifyEnv = (environment && environment->size()>0);

  // determine how much time we have
  double timeout = this->CTest->GetRemainingTimeAllowed() - 120;
  if (this->CTest->GetTimeOut() && this->CTest->GetTimeOut() < timeout)
    {
    timeout = this->CTest->GetTimeOut();
    }
  if (testTimeOut 
      && testTimeOut < this->CTest->GetRemainingTimeAllowed())
    {
    timeout = testTimeOut;
    }

  // always have at least 1 second if we got to here
  if (timeout <= 0)
    {
    timeout = 1;
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, this->Index << ": "
             << "Test timeout computed to be: " << timeout << "\n");

  if (this->ModifyEnv)
    {
    this->OrigEnv = cmSystemTools::AppendEnv(environment);
    }

  return this->TestProcess->StartProcess();
}

void cmCTestRunTest::WriteLogOutputTop(size_t completed, size_t total)
{
  
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::setw(getNumWidth(total))
             << completed << "/");
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::setw(getNumWidth(total))
             << total << " ");

  if ( this->TestHandler->MemCheck )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "MemCheck");
    }
  else
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "Test");
    }

  cmOStringStream indexStr;
  indexStr << " #" << this->Index << ":";
  cmCTestLog(this->CTest, HANDLER_OUTPUT, 
             std::setw(3 + getNumWidth(this->TestHandler->GetMaxIndex()))
             << indexStr.str().c_str());
  cmCTestLog(this->CTest, HANDLER_OUTPUT, " ");
  const int maxTestNameWidth = this->CTest->GetMaxTestNameWidth();
  std::string outname = this->TestProperties->Name + " ";
  outname.resize(maxTestNameWidth + 4, '.');

  *this->TestHandler->LogFile << this->TestProperties->Index << "/"
    << this->TestHandler->TotalNumberOfTests << " Testing: " 
    << this->TestProperties->Name << std::endl;
  *this->TestHandler->LogFile << this->TestProperties->Index << "/"
    << this->TestHandler->TotalNumberOfTests
    << " Test: " << this->TestProperties->Name.c_str() << std::endl;
  *this->TestHandler->LogFile << "Command: \"" << this->ActualCommand << "\"";
  
  for (std::vector<std::string>::iterator i = this->Arguments.begin();
       i != this->Arguments.end(); ++i)
    {
    *this->TestHandler->LogFile 
      << " \"" << i->c_str() << "\"";
    }
  *this->TestHandler->LogFile << std::endl
    << "Directory: " << this->TestProperties->Directory << std::endl
    << "\"" << this->TestProperties->Name.c_str() << "\" start time: "
    << this->StartTime << std::endl;

  *this->TestHandler->LogFile
    << "Output:" << std::endl
    << "----------------------------------------------------------"
    << std::endl;
  *this->TestHandler->LogFile
    << this->ProcessOutput.c_str() << "<end of output>" << std::endl;

  cmCTestLog(this->CTest, HANDLER_OUTPUT, outname.c_str());
  cmCTestLog(this->CTest, DEBUG, "Testing " 
             << this->TestProperties->Name.c_str() << " ... ");
}
