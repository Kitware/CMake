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
  int pipe = this->TestProcess->CheckOutput(.1, out, err);
  if(pipe == cmsysProcess_Pipe_STDOUT)
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
               this->GetIndex() << ": " << out << std::endl);
    this->ProcessOutput += out;
    this->ProcessOutput += "\n";
    }
  else if(pipe == cmsysProcess_Pipe_STDERR)
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
               this->GetIndex() << ": " << err << std::endl);
    this->ProcessOutput += err;
    this->ProcessOutput += "\n";
    }
}

//---------------------------------------------------------
bool cmCTestRunTest::EndTest()
{
  //restore the old environment
  if (this->ModifyEnv)
    {
    cmSystemTools::RestoreEnv(this->OrigEnv);
    }
  this->WriteLogOutputTop();
  std::string reason;
  bool passed = true;
  int res = this->TestProcess->GetProcessStatus();
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
  if(!this->TestHandler->MemCheck)
    {
    if ( this->TestResult.Status == cmCTestTestHandler::COMPLETED )
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
  if ( this->TestHandler->LogFile )
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
  this->TestResult.Output = this->ProcessOutput;
  this->TestResult.ReturnValue = this->TestProcess->GetExitValue();
  this->TestResult.CompletionStatus = "Completed";
  this->TestResult.ExecutionTime = this->TestProcess->GetTotalTime();
  this->TestHandler->TestResults.push_back( this->TestResult );

  delete this->TestProcess;
  return passed;
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
  std::vector<std::string>& args = this->TestProperties->Args;
  this->TestResult.Properties = this->TestProperties;
  this->TestResult.ExecutionTime = 0;
  this->TestResult.ReturnValue = -1;
  this->TestResult.Status = cmCTestTestHandler::NOT_RUN;
  this->TestResult.TestCount = this->TestProperties->Index;  
  this->TestResult.Name = this->TestProperties->Name;
  this->TestResult.Path = this->TestProperties->Directory.c_str();
  
  // find the test executable
  this->ActualCommand 
    = this->TestHandler->FindTheExecutable(args[1].c_str());
  this->TestCommand
    = cmSystemTools::ConvertToOutputPath(this->ActualCommand.c_str());

  // continue if we did not find the executable
  if (this->TestCommand == "")
    {
    *this->TestHandler->LogFile << "Unable to find executable: " 
                   << args[1].c_str() << std::endl;
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Unable to find executable: "
               << args[1].c_str() << std::endl);
    this->TestResult.Output = "Unable to find executable: " + args[1];
    if ( !this->CTest->GetShowOnly() )
      {
      this->TestResult.FullCommandLine = this->ActualCommand;
      this->TestHandler->TestResults.push_back( this->TestResult );
      return false;
      }
    }

  /**
   * Run an executable command and put the stdout in output.
   */
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl
             << this->Index << ": "
             << (this->TestHandler->MemCheck?"MemCheck":"Test") 
             << " command: " << this->TestCommand
             << std::endl);

  this->StartTime = this->CTest->CurrentTime();

  return this->CreateProcess(this->ActualCommand,
                             this->TestProperties->Args,
                             this->TestProperties->Timeout,
                             &this->TestProperties->Environment);
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
bool cmCTestRunTest::CreateProcess(std::string command,
                     std::vector<std::string> args,
                     double testTimeOut,
                     std::vector<std::string>* environment)
{
  std::vector<std::string> commandArgs;
  std::vector<std::string>::iterator i = args.begin();

  ++i; //skip test name
  ++i; //skip executable name
  for(; i != args.end(); ++i)
    {
    commandArgs.push_back(*i);
    }
  this->TestProcess = new cmProcess;
  this->TestProcess->SetId(this->Index);
  this->TestProcess->SetCommand(command.c_str());
  this->TestProcess->SetCommandArguments(commandArgs);

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

void cmCTestRunTest::WriteLogOutputTop()
{
  /* Not sure whether we want to prepend the test index anymore
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::setw(3)
             << this->Index << ": ");*/
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::setw(3) 
             << this->TestProperties->Index << "/");
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::setw(3) 
             << this->TestHandler->TotalNumberOfTests << " ");
  if ( this->TestHandler->MemCheck )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "Memory Check");
    }
  else
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "Testing");
    }
  cmCTestLog(this->CTest, HANDLER_OUTPUT, " ");
  const int maxTestNameWidth = this->CTest->GetMaxTestNameWidth();
  std::string outname = this->TestProperties->Name + " ";
  outname.resize(maxTestNameWidth, '.');

  // add the arguments
  std::vector<std::string>::const_iterator j = 
    this->TestProperties->Args.begin();
  ++j; // skip test name
  ++j; // skip command as it is in actualCommand
  std::vector<const char*> arguments;
  this->TestHandler->GenerateTestCommand(arguments);
  arguments.push_back(this->ActualCommand.c_str());
  for(;j != this->TestProperties->Args.end(); ++j)
    {
    this->TestCommand += " ";
    this->TestCommand += cmSystemTools::EscapeSpaces(j->c_str());
    arguments.push_back(j->c_str());
    }
  arguments.push_back(0);
  this->TestResult.FullCommandLine = this->TestCommand;

  *this->TestHandler->LogFile << this->TestProperties->Index << "/" 
    << this->TestHandler->TotalNumberOfTests << " Testing: " 
    << this->TestProperties->Name << std::endl;
  *this->TestHandler->LogFile << this->TestProperties->Index << "/" 
    << this->TestHandler->TotalNumberOfTests
    << " Test: " << this->TestProperties->Name.c_str() << std::endl;
  *this->TestHandler->LogFile << "Command: ";
  std::vector<cmStdString>::size_type ll;
  for ( ll = 0; ll < arguments.size()-1; ll ++ )
    {
    *this->TestHandler->LogFile << "\"" << arguments[ll] << "\" ";
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
