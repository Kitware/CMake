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

#include <cmStandardIncludes.h>
#include <cmCTestTestHandler.h>
#include <cmCTestRunTest.h>

/** \class cmCTestMultiProcessHandler
 * \brief run parallel ctest
 *
 * cmCTestMultiProcessHandler 
 */
class cmCTestMultiProcessHandler 
{
public:
  struct TestSet : public std::set<int> {};
  struct TestMap : public std::map<int, TestSet> {};
  struct PropertiesMap : public 
     std::map<int, cmCTestTestHandler::cmCTestTestProperties*> {};

  cmCTestMultiProcessHandler();
  // Set the tests
  void SetTests(TestMap& tests, PropertiesMap& properties);
  // Set the max number of tests that can be run at the same time.
  void SetParallelLevel(size_t);
  void RunTests();
  void PrintTests();
  //void SetCTestCommand(const char* c) { this->CTestCommand = c;}
  //void SetTestCacheFile(const char* c) { this->CTestCacheFile = c;}
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

  void SetTestHandler(cmCTestTestHandler * handler)
  { this->TestHandler = handler; }

  cmCTestTestHandler * GetTestHandler()
  { return this->TestHandler; }
protected:  
  cmCTest* CTest;
  // Start the next test or tests as many as are allowed by
  // ParallelLevel
  void StartNextTests();
  void StartTestProcess(int test);
  bool StartTest(int test);
  // Mark the checkpoint for the given test
  void WriteCheckpoint(int index);
  // Removes the checkpoint file
  void MarkFinished();
  // Return true if there are still tests running
  // check all running processes for output and exit case
  bool CheckOutput();
  void RemoveTest(int index);
  //Check if we need to resume an interrupted test set
  void CheckResume();
  // map from test number to set of depend tests
  TestMap Tests;
  //list of test properties (indices concurrent to the test map)
  PropertiesMap Properties;
  std::map<int, bool> TestRunningMap;
  std::map<int, bool> TestFinishMap;
  std::map<int, cmStdString> TestOutput;
  std::vector<cmStdString>* Passed;
  std::vector<cmStdString>* Failed;
  std::vector<cmCTestTestHandler::cmCTestTestResult>* TestResults;
  size_t ParallelLevel; // max number of process that can be run at once
  std::set<cmCTestRunTest*> RunningTests;  // current running tests
  cmCTestTestHandler * TestHandler;
};

#endif
