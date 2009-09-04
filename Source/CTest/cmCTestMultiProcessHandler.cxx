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
#include "cmSystemTools.h"
#include <stdlib.h>

cmCTestMultiProcessHandler::cmCTestMultiProcessHandler()
{
  this->ParallelLevel = 1;
  this->Completed = 0;
}
  // Set the tests
void 
cmCTestMultiProcessHandler::SetTests(TestMap& tests,
                                     TestMap& expensiveTests,
                                     PropertiesMap& properties)
{
  // set test run map to false for all
  for(TestMap::iterator i = this->Tests.begin();
      i != this->Tests.end(); ++i)
    {
    this->TestRunningMap[i->first] = false;
    this->TestFinishMap[i->first] = false;

    if(this->Properties[i->first]->Expensive)
      {
      this->ExpensiveTests[i->first] = i->second;
      }
    }
  this->Tests = tests;
  this->ExpensiveTests = expensiveTests;
  this->Properties = properties;
  this->Total = this->Tests.size();
}

  // Set the max number of tests that can be run at the same time.
void cmCTestMultiProcessHandler::SetParallelLevel(size_t level)
{
  this->ParallelLevel = level < 1 ? 1 : level;
}

void cmCTestMultiProcessHandler::RunTests()
{
  if(this->CTest->GetBatchJobs())
    {
    this->SubmitBatchTests();
    return;
    }
  this->CheckResume();
  this->TestHandler->SetMaxIndex(this->FindMaxIndex());
  this->StartNextTests();
  while(this->Tests.size() != 0 || this->ExpensiveTests.size() != 0)
    {
    this->CheckOutput();
    this->StartNextTests();
    }
  // let all running tests finish
  while(this->CheckOutput())
    {
    }
  this->MarkFinished();
}

void cmCTestMultiProcessHandler::SubmitBatchTests()
{
  for(cmCTest::CTestConfigurationMap::iterator i =
      this->CTest->CTestConfiguration.begin();
      i != this->CTest->CTestConfiguration.end(); ++i)
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, i->first 
               << " = " << i->second << std::endl);
    }
}

void cmCTestMultiProcessHandler::StartTestProcess(int test)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, test << ": "
            << " test " << test << "\n");
  this->TestRunningMap[test] = true; // mark the test as running
  // now remove the test itself
  if(this->ExpensiveTests.size() > 0)
    {
    this->ExpensiveTests.erase(test);
    }
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
    this->Completed++;
    testRun->EndTest(this->Completed, this->Total, false);
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
    this->StartTestProcess(test);
    return true;
    }
  // This test was not able to start because it is waiting 
  // on depends to run
  return false;
}

void cmCTestMultiProcessHandler::StartNextTests()
{
  size_t numToStart = this->ParallelLevel - this->RunningTests.size();
  if(numToStart == 0)
    {
    return;
    }
  TestMap tests = this->ExpensiveTests.size() > 0 ? 
    this->ExpensiveTests : this->Tests;

  for(TestMap::iterator i = tests.begin();
      i !=  tests.end(); ++i)
    {
    //int processors = this->Properties[i->first]->Processors;
    
//    if(processors > )
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
    this->Completed++;
    cmCTestRunTest* p = *i;
    int test = p->GetIndex();

    if(p->EndTest(this->Completed, this->Total, true))
      {
      this->Passed->push_back(p->GetTestProperties()->Name);
      }
    else
      {
      this->Failed->push_back(p->GetTestProperties()->Name);
      }
    for(TestMap::iterator j = this->ExpensiveTests.begin();
        j != this->ExpensiveTests.end(); ++j)
      {
      j->second.erase(test);
      }
    for(TestMap::iterator j = this->Tests.begin();
        j != this->Tests.end(); ++j)
      {
      j->second.erase(test);
      }
    this->TestFinishMap[test] = true;
    this->TestRunningMap[test] = false;
    this->RunningTests.erase(p);
    this->WriteCheckpoint(test);

    delete p;
    }
  return true;
}

void cmCTestMultiProcessHandler::WriteCheckpoint(int index)
{
  std::string fname = this->CTest->GetBinaryDir()
    + "/Testing/Temporary/CTestCheckpoint.txt";
  std::fstream fout;
  fout.open(fname.c_str(), std::ios::app);
  fout << index << "\n";
  fout.close();
}

void cmCTestMultiProcessHandler::MarkFinished()
{
  std::string fname = this->CTest->GetBinaryDir()
    + "/Testing/Temporary/CTestCheckpoint.txt";
  cmSystemTools::RemoveFile(fname.c_str());
}

//---------------------------------------------------------------------
//For ShowOnly mode
void cmCTestMultiProcessHandler::PrintTestList()
{
  int count = 0;
  for (PropertiesMap::iterator it = this->Properties.begin();
       it != this->Properties.end(); it ++ )
    {
    count++;
    cmCTestTestHandler::cmCTestTestProperties& p = *it->second;

    cmCTestRunTest testRun;
    testRun.SetCTest(this->CTest);
    testRun.SetTestHandler(this->TestHandler);
    testRun.SetIndex(p.Index);
    testRun.SetTestProperties(&p);
    testRun.ComputeArguments(); //logs the command in verbose mode

    cmCTestLog(this->CTest, HANDLER_OUTPUT, std::setw(3)
             << count << "/");
    cmCTestLog(this->CTest, HANDLER_OUTPUT, std::setw(3)
             << this->Total << " ");
    if (this->TestHandler->MemCheck)
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "Memory Check");
      }
     else
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "Testing");
      }
    cmCTestLog(this->CTest, HANDLER_OUTPUT, " ");
    cmCTestLog(this->CTest, HANDLER_OUTPUT, p.Name.c_str() << std::endl);
    }
}

//----------------------------------------------------------------
void cmCTestMultiProcessHandler::CheckResume()
{
  std::string fname = this->CTest->GetBinaryDir()
      + "/Testing/Temporary/CTestCheckpoint.txt";
  if(this->CTest->GetFailover())
    {
    if(cmSystemTools::FileExists(fname.c_str(), true))
      {
      *this->TestHandler->LogFile << "Resuming previously interrupted test set"
        << std::endl
        << "----------------------------------------------------------"
        << std::endl;
        
      std::ifstream fin;
      fin.open(fname.c_str());
      std::string line;
      while(std::getline(fin, line))
        {
        int index = atoi(line.c_str());
        this->RemoveTest(index);
        }
      fin.close();
      }
    }
  else
    {
    if(cmSystemTools::FileExists(fname.c_str(), true))
      {
      cmSystemTools::RemoveFile(fname.c_str());
      }
    }
}

void cmCTestMultiProcessHandler::RemoveTest(int index)
{
  this->Tests.erase(index);
  this->Properties.erase(index);
  this->ExpensiveTests.erase(index);
  this->TestRunningMap[index] = false;
  this->TestFinishMap[index] = true;
  this->Completed++;
}

int cmCTestMultiProcessHandler::FindMaxIndex()
{
  int max = 0;
  cmCTestMultiProcessHandler::TestMap::iterator i = this->Tests.begin();
  for(; i != this->Tests.end(); ++i)
    {
    if(i->first > max)
      {
      max = i->first;
      }
    }
  return max;
}
