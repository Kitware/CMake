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
  
  void SetOptimizeForCTest(bool optimize)
  { this->OptimizeForCTest = optimize; }

  bool GetOptimizeForCTest()
  { return this->OptimizeForCTest; }

  std::string GetProcessOutput()
  { return this->ProcessOutput; }

  //Provides a handle to the log stream in case someone wants
  // to asynchronously process the log
  std::ostream * GetLogStream()
  { return this->TestHandler->LogFile; }

  cmCTestTestHandler::cmCTestTestResult GetTestResults()
  { return this->TestResult; }

  //Runs the test
  bool Execute();
protected:
  void DartProcessing(std::string& output);
  int RunTestProcess(std::vector<const char*> argv,
                     std::string* output, int *retVal,
                     std::ostream* log, double testTimeOut,
                     std::vector<std::string>* environment);
private:
  cmCTestTestHandler::cmCTestTestProperties * TestProperties;
  //Pointer back to the "parent"; the handler that invoked this test run
  cmCTestTestHandler * TestHandler;
  cmCTest * CTest;
  //If the executable to run is ctest, don't create a new process; 
  //just instantiate a new cmTest.  (Can be disabled for a single test
  //if this option is set to false.)
  bool OptimizeForCTest;
  std::string ProcessOutput;
  //The test results
  cmCTestTestHandler::cmCTestTestResult TestResult;
};

#endif

