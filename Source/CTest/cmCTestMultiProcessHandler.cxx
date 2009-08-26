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
#include "cmCTestMultiProcessHandler.h"
#include "cmProcess.h"
#include "cmStandardIncludes.h"
#include "cmCTest.h"


cmCTestMultiProcessHandler::cmCTestMultiProcessHandler()
{
  this->ParallelLevel = 1;
}
  // Set the tests
void 
cmCTestMultiProcessHandler::SetTests(TestMap& tests,
                                     PropertiesMap& properties)
{
  // set test run map to false for all
  for(TestMap::iterator i = this->Tests.begin();
      i != this->Tests.end(); ++i)
    {
    this->TestRunningMap[i->first] = false;
    this->TestFinishMap[i->first] = false;
    }
  this->Tests = tests;
  this->Properties = properties;
}
  // Set the max number of tests that can be run at the same time.
void cmCTestMultiProcessHandler::SetParallelLevel(size_t level)
{
  this->ParallelLevel = level < 1 ? 1 : level;
}

void cmCTestMultiProcessHandler::RunTests()
{
  this->StartNextTests();
  while(this->Tests.size() != 0)
    {
    this->CheckOutput();
    this->StartNextTests();
    }
  // let all running tests finish
  while(this->CheckOutput())
    {
    }
}

void cmCTestMultiProcessHandler::StartTestProcess(int test)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, test << ": "
            << " test " << test << "\n");
  this->TestRunningMap[test] = true; // mark the test as running
  // now remove the test itself
  this->Tests.erase(test);
  cmCTestRunTest* testRun = new cmCTestRunTest;
  testRun->SetCTest(this->CTest);
  testRun->SetTestHandler(this->TestHandler);
  testRun->SetIndex(test);
  testRun->SetTestProperties(this->Properties[test]);
  if(testRun->StartTest())
    {
    this->RunningTests.insert(testRun);
    }
  else
    {
    testRun->EndTest();
    }
}

bool cmCTestMultiProcessHandler::StartTest(int test)
{
  // copy the depend tests locally because when 
  // a test is finished it will be removed from the depend list
  // and we don't want to be iterating a list while removing from it
  TestSet depends = this->Tests[test];
  size_t totalDepends = depends.size();
  if(totalDepends)
    {
    for(TestSet::const_iterator i = depends.begin();
        i != depends.end(); ++i)
      {
      // if the test is not already running then start it
      if(!this->TestRunningMap[*i])
        {
        // this test might be finished, but since
        // this is a copy of the depend map we might
        // still have it
        if(!this->TestFinishMap[*i])
          {
          // only start one test in this function
          return this->StartTest(*i);
          }
        else
          {
          // the depend has been and finished
          totalDepends--;
          }
        }
      }
    }
  // if there are no depends left then run this test
  if(totalDepends == 0)
    {
    // Start this test it has no depends 
    this->StartTestProcess(test);
    return true;
    }
  // This test was not able to start because it is waiting 
  // on depends to run
  return false;
}

void cmCTestMultiProcessHandler::StartNextTests()
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl
             << "Number of running tests : " << this->RunningTests.size()
             << "\n");
  size_t numToStart = this->ParallelLevel - this->RunningTests.size();
  if(numToStart == 0)
    {
    return;
    }
  TestMap tests = this->Tests;
  for(TestMap::iterator i = tests.begin();
      i !=  tests.end(); ++i)
    {
    // start test should start only one test
    if(this->StartTest(i->first))
      {
      numToStart--;
      }
    else
      {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, std::endl
             << "Test did not start waiting on depends to finish: " 
                 << i->first << "\n");
      }
    if(numToStart == 0 )
      {
      return;
      }
    }
}

bool cmCTestMultiProcessHandler::CheckOutput()
{
  // no more output we are done
  if(this->RunningTests.size() == 0)
    {
    return false;
    }
  std::vector<cmCTestRunTest*> finished;
  std::string out, err;
  for(std::set<cmCTestRunTest*>::const_iterator i = this->RunningTests.begin();
      i != this->RunningTests.end(); ++i)
    {
    cmCTestRunTest* p = *i;
    p->CheckOutput(); //reads and stores the process output
    
    if(!p->IsRunning())
      {
      finished.push_back(p);
      }
    }

  for( std::vector<cmCTestRunTest*>::iterator i = finished.begin();
       i != finished.end(); ++i)
    {
    cmCTestRunTest* p = *i;
    int test = p->GetIndex();
    
    if(p->EndTest())
      {
        this->Passed->push_back(p->GetTestProperties()->Name);
      }
    else
      {
        this->Failed->push_back(p->GetTestProperties()->Name);
      }
    for(TestMap::iterator j = this->Tests.begin();
       j!=  this->Tests.end(); ++j)
    {
    j->second.erase(test);
    }
    this->TestFinishMap[test] = true;
    this->TestRunningMap[test] = false;
    this->RunningTests.erase(p);

    delete p;
    }
  return true;
}

void cmCTestMultiProcessHandler::PrintTests()
{
#undef cout
  for( TestMap::iterator i = this->Tests.begin();
       i!=  this->Tests.end(); ++i)
    {
    std::cout << "Test " << i->first << "  (";
    for(TestSet::iterator j = i->second.begin(); 
        j != i->second.end(); ++j)
      {
      std::cout << *j << " ";
      }
    std::cout << ")\n";
    }
}

#if 0
int main()
{
  cmCTestMultiProcessHandler h;
  h.SetParallelLevel(4);
  std::map<int, std::set<int> > tests;
  std::set<int> depends;
  for(int i =1; i < 92; i++)
    {
    tests[i] = depends;
    }
  depends.clear();
  depends.insert(45);  subprject
  tests[46] = depends;  subproject-stage2
  depends.clear();
  depends.insert(55);  simpleinstall simpleinstall-s2
  tests[56] = depends;
  depends.clear();
  depends.insert(70);  wrapping
  tests[71] = depends; qtwrapping
  depends.clear();
  depends.insert(71);  qtwrapping
  tests[72] = depends;  testdriver1
  depends.clear();
  depends.insert(72)    testdriver1
  tests[73] = depends;  testdriver2
  depends.clear();
  depends.insert(73);   testdriver2
  tests[74] = depends;  testdriver3
  depends.clear();
  depends.insert(79);   linkorder1
  tests[80] = depends;  linkorder2
  h.SetTests(tests);
  h.PrintTests();
  h.RunTests();
}
#endif
