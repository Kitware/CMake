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
  this->OptimizeForCTest = true;
  cmCTestTestHandler::cmCTestTestResult result;
  this->TestResult = result;
}

cmCTestRunTest::~cmCTestRunTest()
{
}

void cmCTestRunTest::SetTestHandler(cmCTestTestHandler * handler)
{
  this->TestHandler = handler;
  this->CTest = handler->CTest;
}

//----------------------------------------------------------------------
// Executes a test.  Returns whether it passed or failed
bool cmCTestRunTest::Execute()
{
  const std::string& testname = this->TestProperties->Name;
  std::vector<std::string>& args = this->TestProperties->Args;
  this->TestResult.Properties = this->TestProperties;
  this->TestResult.ExecutionTime = 0;
  this->TestResult.ReturnValue = -1;
  this->TestResult.Status = cmCTestTestHandler::NOT_RUN;
  this->TestResult.TestCount = this->TestProperties->Index;  
  this->TestResult.Name = testname;
  this->TestResult.Path = this->TestProperties->Directory.c_str();

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
  std::string outname = testname + " ";
  outname.resize(maxTestNameWidth, '.');
  *this->TestHandler->LogFile << this->TestProperties->Index << "/" 
    << this->TestHandler->TotalNumberOfTests << " Testing: " 
    << testname << std::endl;

  if ( this->CTest->GetShowOnly() )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, outname.c_str() << std::endl);
    }
  else
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, outname.c_str());
    }

  cmCTestLog(this->CTest, DEBUG, "Testing " << args[0].c_str() << " ... ");
  // find the test executable
  std::string actualCommand 
    = this->TestHandler->FindTheExecutable(args[1].c_str());
  std::string testCommand
    = cmSystemTools::ConvertToOutputPath(actualCommand.c_str());

  // continue if we did not find the executable
  if (testCommand == "")
    {
    *this->TestHandler->LogFile << "Unable to find executable: " 
                   << args[1].c_str() << std::endl;
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Unable to find executable: "
               << args[1].c_str() << std::endl);
    this->TestResult.Output = "Unable to find executable: " + args[1];
    if ( !this->CTest->GetShowOnly() )
      {
      this->TestResult.FullCommandLine = actualCommand;
      this->TestHandler->TestResults.push_back( this->TestResult );
      return false;
      }
    }

  // add the arguments
  std::vector<std::string>::const_iterator j = args.begin();
  ++j; // skip test name
  ++j; // skip command as it is in actualCommand
  std::vector<const char*> arguments;
  this->TestHandler->GenerateTestCommand(arguments);
  arguments.push_back(actualCommand.c_str());
  for(;j != args.end(); ++j)
    {
    testCommand += " ";
    testCommand += cmSystemTools::EscapeSpaces(j->c_str());
    arguments.push_back(j->c_str());
    }
  arguments.push_back(0);

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string output;
  int retVal = 0;

  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl
             << (this->TestHandler->MemCheck?"MemCheck":"Test") 
             << " command: " << testCommand
             << std::endl);
  *this->TestHandler->LogFile << this->TestProperties->Index << "/" 
                 << this->TestHandler->TotalNumberOfTests
                 << " Test: " << testname.c_str() << std::endl;
  *this->TestHandler->LogFile << "Command: ";
  std::vector<cmStdString>::size_type ll;
  for ( ll = 0; ll < arguments.size()-1; ll ++ )
    {
    *this->TestHandler->LogFile << "\"" << arguments[ll] << "\" ";
    }
  *this->TestHandler->LogFile
    << std::endl
    << "Directory: " << this->TestProperties->Directory << std::endl
    << "\"" << testname.c_str() << "\" start time: "
    << this->CTest->CurrentTime() << std::endl
    << "Output:" << std::endl
    << "----------------------------------------------------------"
    << std::endl;
  int res = 0;
  double clock_start, clock_finish;
  clock_start = cmSystemTools::GetTime();

  if ( !this->CTest->GetShowOnly() )
    {
    res = this->RunTestProcess(arguments, &output, &retVal,
                               this->TestHandler->LogFile,
                               this->TestProperties->Timeout,
                               &this->TestProperties->Environment);
    this->ProcessOutput = output; //save process output in the object
    }

  clock_finish = cmSystemTools::GetTime();  

  this->TestResult.ExecutionTime = (double)(clock_finish - clock_start);
  this->TestResult.FullCommandLine = testCommand;
  std::string reason;

  bool passed = true;

  if ( !this->CTest->GetShowOnly() )
    {
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
        if ( passIt->first.find(output.c_str()) )
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
        if ( passIt->first.find(output.c_str()) )
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
        cmCTestLog(this->CTest, HANDLER_OUTPUT,   "   Passed  " );
        }
      else
        {
        this->TestResult.Status = cmCTestTestHandler::FAILED;
        cmCTestLog(this->CTest, HANDLER_OUTPUT,
                   "***Failed  " << reason );
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
    sprintf(buf, "%6.2f sec", this->TestResult.ExecutionTime);
    cmCTestLog(this->CTest, HANDLER_OUTPUT, buf << "\n" );
    if ( this->TestHandler->LogFile )
      {
      *this->TestHandler->LogFile << "\nTest time = " << buf << std::endl;
      }
    this->DartProcessing(output);
    } 

  // if this is doing MemCheck then all the output needs to be put into
  // Output since that is what is parsed by cmCTestMemCheckHandler
  if(!this->TestHandler->MemCheck)
    {
    if ( this->TestResult.Status == cmCTestTestHandler::COMPLETED )
      {
      this->TestHandler->CleanTestOutput(output, static_cast<size_t>
                     (this->TestHandler->CustomMaximumPassedTestOutputSize));
      }
    else
      {
      this->TestHandler->CleanTestOutput(output, static_cast<size_t>
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
    double ttime = clock_finish - clock_start;
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
    *this->TestHandler->LogFile << "\"" << testname.c_str() << "\" end time: "
      << this->CTest->CurrentTime() << std::endl
      << "\"" << testname.c_str() << "\" time elapsed: "
      << buffer << std::endl
      << "----------------------------------------------------------"
      << std::endl << std::endl;
    }
  this->TestResult.Output = output;
  this->TestResult.ReturnValue = retVal;
  this->TestResult.CompletionStatus = "Completed";
  this->TestHandler->TestResults.push_back( this->TestResult );

  return passed;
}

//----------------------------------------------------------------------
void cmCTestRunTest::DartProcessing(std::string& output)
{
  if (!output.empty() && output.find("<DartMeasurement") != output.npos)
    {
    if (this->TestHandler->DartStuff.find(output.c_str()))
      {
      std::string dartString = this->TestHandler->DartStuff.match(1);
      // keep searching and replacing until none are left
      while (this->TestHandler->DartStuff1.find(output.c_str()))
        {
        // replace the exact match for the string
        cmSystemTools::ReplaceString(output,
                         this->TestHandler->DartStuff1.match(1).c_str(), "");
        }
      this->TestResult.RegressionImages
        = this->TestHandler->GenerateRegressionImages(dartString);
      }
    }
}

//----------------------------------------------------------------------
int cmCTestRunTest::RunTestProcess(std::vector<const char*> argv,
                     std::string* output, int *retVal,
                     std::ostream* log, double testTimeOut,
                     std::vector<std::string>* environment)
{
  std::vector<std::string> origEnv;
  bool modifyEnv = (environment && environment->size()>0);

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
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "Test timeout computed to be: " << timeout << "\n");

  if(cmSystemTools::SameFile(argv[0], this->CTest->CTestSelf.c_str()) &&
     !this->CTest->ForceNewCTestProcess &&
     this->OptimizeForCTest)
    {
    cmCTest inst;
    inst.ConfigType = this->CTest->ConfigType;
    inst.TimeOut = timeout;

    // Capture output of the child ctest.
    cmOStringStream oss;
    inst.SetStreams(&oss, &oss);

    std::vector<std::string> args;
    for(unsigned int i =0; i < argv.size(); ++i)
      {
      if(argv[i])
        {
        // make sure we pass the timeout in for any build and test 
        // invocations. Since --build-generator is required this is a 
        // good place to check for it, and to add the arguments in
        if (strcmp(argv[i],"--build-generator") == 0 && timeout)
          {
          args.push_back("--test-timeout");
          cmOStringStream msg;
          msg << timeout;
          args.push_back(msg.str());
          }
        args.push_back(argv[i]);
        }
      }
    if ( log )
      {
      *log << "* Run internal CTest" << std::endl;
      }
    std::string oldpath = cmSystemTools::GetCurrentWorkingDirectory();

    if (modifyEnv)
      {
      origEnv = cmSystemTools::AppendEnv(environment);
      }

    *retVal = inst.Run(args, output);
    *output += oss.str();
    if ( log )
      {
      *log << output->c_str();
      }
    cmSystemTools::ChangeDirectory(oldpath.c_str());

    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      "Internal cmCTest object used to run test." << std::endl
      <<  *output << std::endl);

    if (modifyEnv)
      {
      cmSystemTools::RestoreEnv(origEnv);
      }

    return cmsysProcess_State_Exited;
    }
  std::vector<char> tempOutput;
  if ( output )
    {
    *output = "";
    }

  if (modifyEnv)
    {
    origEnv = cmSystemTools::AppendEnv(environment);
    }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmCTestLog(this->CTest, DEBUG, "Command is: " << argv[0] << std::endl);
  if(cmSystemTools::GetRunCommandHideConsole())
    {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
    }

  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);

  char* data;
  int length;
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    if ( output )
      {
      tempOutput.insert(tempOutput.end(), data, data+length);
      }
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
      cmCTestLogWrite(data, length));
    if ( log )
      {
      log->write(data, length);
      }
    }

  cmsysProcess_WaitForExit(cp, 0);
  if(output && tempOutput.begin() != tempOutput.end())
    {
    //We are waiting for exit before finally appending to the output
    output->append(&*tempOutput.begin(), tempOutput.size());
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "-- Process completed"
    << std::endl);

  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    if(*retVal != 0 && this->CTest->OutputTestOutputOnTestFailure)
      {
        this->CTest->OutputTestErrors(tempOutput);
      }
    }
  else if(result == cmsysProcess_State_Exception)
    {
    if(this->CTest->OutputTestOutputOnTestFailure)
      {
        this->CTest->OutputTestErrors(tempOutput);
      }
    *retVal = cmsysProcess_GetExitException(cp);
    std::string outerr = "\n*** Exception executing: ";
    outerr += cmsysProcess_GetExceptionString(cp);
    *output += outerr;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, outerr.c_str()
      << std::endl << std::flush);
    }
  else if(result == cmsysProcess_State_Error)
    {
    std::string outerr = "\n*** ERROR executing: ";
    outerr += cmsysProcess_GetErrorString(cp);
    *output += outerr;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, outerr.c_str()
      << std::endl << std::flush);
    }
  cmsysProcess_Delete(cp);

  if (modifyEnv)
    {
    cmSystemTools::RestoreEnv(origEnv);
    }

  return result;
}

