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
#ifndef cmCTestRunTest_h
#define cmCTestRunTest_h

#include <cmStandardIncludes.h>
#include <cmCTestTestHandler.h>
#include <cmProcess.h>

/** \class cmRunTest
 * \brief represents a single test to be run
 *
 * cmRunTest contains the information related to running a single test
 */
class cmCTestRunTest
{
public:
  cmCTestRunTest();
  ~cmCTestRunTest();

  void SetTestProperties(cmCTestTestHandler::cmCTestTestProperties * prop)
  { this->TestProperties = prop; }

  cmCTestTestHandler::cmCTestTestProperties * GetTestProperties()
  { return this->TestProperties; }
  
  void SetTestHandler(cmCTestTestHandler * handler);

  void SetIndex(int i) { this->Index = i; }

  void SetCTest(cmCTest * ct) { this->CTest = ct; }

  int GetIndex() { return this->Index; }

  std::string GetProcessOutput() { return this->ProcessOutput; }

  cmCTestTestHandler::cmCTestTestResult GetTestResults()
  { return this->TestResult; }

  bool IsRunning();
  void CheckOutput();
  //launch the test process, return whether it started correctly
  bool StartTest();
  //capture and report the test results
  bool EndTest(size_t completed, size_t total);
  //Called by ctest -N to log the command string
  void ComputeArguments();
private:
  void DartProcessing();
  bool CreateProcess(double testTimeOut,
                     std::vector<std::string>* environment);
  void WriteLogOutputTop(size_t completed, size_t total);
  //Run post processing of the process output for MemCheck
  void MemCheckPostProcess();

  cmCTestTestHandler::cmCTestTestProperties * TestProperties;
  //Pointer back to the "parent"; the handler that invoked this test run
  cmCTestTestHandler * TestHandler;
  cmCTest * CTest;
  cmProcess * TestProcess;
  //If the executable to run is ctest, don't create a new process; 
  //just instantiate a new cmTest.  (Can be disabled for a single test
  //if this option is set to false.)
  //bool OptimizeForCTest;

  //flag for whether the env was modified for this run
  bool ModifyEnv;
  //stores the original environment if we are modifying it
  std::vector<std::string> OrigEnv;
  std::string ProcessOutput;
  //The test results
  cmCTestTestHandler::cmCTestTestResult TestResult;
  int Index;
  std::string StartTime;
  std::string TestCommand;
  std::string ActualCommand;
  std::vector<std::string> Arguments;
};

inline int getNumWidth(int n)
{
  int numWidth = 1;
  if(n >= 10)
    {
    numWidth = 2;
    }
  if(n >= 100)
    {
    numWidth = 3;
    }
  return numWidth;
}
#endif

