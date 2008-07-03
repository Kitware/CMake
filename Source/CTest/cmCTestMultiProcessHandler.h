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
#ifndef cmCTestMultiProcessHandler_h
#define cmCTestMultiProcessHandler_h

#include <set>
#include <map>
#include <string>
#include <vector>
class cmProcess;
#include <cmStandardIncludes.h>
#include <cmCTestTestHandler.h>

/** \class cmCTestMultiProcessHandler
 * \brief run parallel ctest
 *
 * cmCTestMultiProcessHandler 
 */
class cmCTestMultiProcessHandler 
{
public:
  cmCTestMultiProcessHandler();
  // Set the tests
  void SetTests(std::map<int, std::set<int> >& tests,
                std::map<int, cmStdString>& testNames);
  // Set the max number of tests that can be run at the same time.
  void SetParallelLevel(size_t);
  void RunTests();
  void PrintTests();
  void SetCTestCommand(const char* c) { this->CTestCommand = c;}
  void SetTestCacheFile(const char* c) { this->CTestCacheFile = c;}
  void SetPassFailVectors(std::vector<cmStdString>* passed,
                          std::vector<cmStdString>* failed)
    {
      this->Passed = passed;
      this->Failed = failed;
    }
  void SetTestResults(std::vector<cmCTestTestHandler::cmCTestTestResult>* r)
    {
      this->TestResults = r;
    }
  void SetCTest(cmCTest* ctest) { this->CTest = ctest;}
protected:  
  cmCTest* CTest;
  // Start the next test or tests as many as are allowed by
  // ParallelLevel
  void StartNextTests();
  void StartTestProcess(int test);
  bool StartTest(int test);
  void EndTest(cmProcess*);
  // Return true if there are still tests running
  // check all running processes for output and exit case
  bool CheckOutput();
  // map from test number to set of depend tests
  std::map<int, std::set<int> > Tests;
  std::map<int, cmStdString> TestNames;
  std::map<int, bool> TestRunningMap;
  std::map<int, bool> TestFinishMap;
  std::map<int, cmStdString> TestOutput;
  std::string CTestCommand;
  std::string CTestCacheFile;
  std::vector<cmStdString>* Passed;
  std::vector<cmStdString>* Failed;
  std::vector<cmCTestTestHandler::cmCTestTestResult>* TestResults;
  int ProcessId;
  size_t ParallelLevel; // max number of process that can be run at once
  std::set<cmProcess*> RunningTests;  // current running tests
};

#endif
