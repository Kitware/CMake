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
  this->ProcessId = 0;
}
  // Set the tests
void 
cmCTestMultiProcessHandler::SetTests(TestMap& tests,
                                     std::map<int,cmStdString>& testNames)
{
  // set test run map to false for all
  for(TestMap::iterator i = this->Tests.begin();
      i != this->Tests.end(); ++i)
    {
    this->TestRunningMap[i->first] = false;
    this->TestFinishMap[i->first] = false;
    }
  this->Tests = tests;
  this->TestNames = testNames;
}
  // Set the max number of tests that can be run at the same time.
void cmCTestMultiProcessHandler::SetParallelLevel(size_t l)
{
  this->ParallelLevel = l;
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
  
  for(std::map<int, cmStdString>::iterator i =
        this->TestOutput.begin();
      i != this->TestOutput.end(); ++i)
    {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
               i->second << std::endl);
    }
      
}

void cmCTestMultiProcessHandler::StartTestProcess(int test)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
            " test " << test << "\n");
  this->TestRunningMap[test] = true; // mark the test as running
  // now remove the test itself
  this->Tests.erase(test);
  // now run the test
  cmProcess* newp = new cmProcess;
  newp->SetId(this->ProcessId);
  newp->SetId(test);
  newp->SetCommand(this->CTestCommand.c_str());
  std::vector<std::string> args;
  cmOStringStream width;
  if(this->CTest->GetMaxTestNameWidth())
    {
    args.push_back("-W");
    width << this->CTest->GetMaxTestNameWidth();
    args.push_back(width.str().c_str());
    }
  args.push_back("-I");
  cmOStringStream strm;
  strm << test << "," << test;
  args.push_back(strm.str());
  args.push_back("--parallel-cache");
  args.push_back(this->CTestCacheFile.c_str());
  args.push_back("--internal-ctest-parallel"); 
  cmOStringStream strm2;
  strm2 << test;
  args.push_back(strm2.str());
  if(this->CTest->GetExtraVerbose())
    {
    args.push_back("-VV");
    }
  newp->SetCommandArguments(args);
  if(!newp->StartProcess())
    {
     cmCTestLog(this->CTest, ERROR_MESSAGE, 
                "Error starting " << newp->GetCommand() << "\n");
    this->EndTest(newp);
    }
  else
    {
    this->RunningTests.insert(newp);
    }
 cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
            "ctest -I " << test << "\n");
 this->ProcessId++;
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
  std::vector<cmProcess*> finished;
  std::string out, err;
  for(std::set<cmProcess*>::const_iterator i = this->RunningTests.begin();
      i != this->RunningTests.end(); ++i)
    {
    cmProcess* p = *i;
    int pipe = p->CheckOutput(.1, out, err);
    if(pipe == cmsysProcess_Pipe_STDOUT)
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, 
                 p->GetId() << ": " << out << std::endl);
      this->TestOutput[ p->GetId() ] += out;
      this->TestOutput[ p->GetId() ] += "\n";
      }
    else if(pipe == cmsysProcess_Pipe_STDERR)
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, 
                 p->GetId() << ": " << err << std::endl);
      this->TestOutput[ p->GetId() ] += err;
      this->TestOutput[ p->GetId() ] += "\n";
      }
    if(!p->IsRunning())
      {
      finished.push_back(p);
      }
    }
  for( std::vector<cmProcess*>::iterator i = finished.begin();
       i != finished.end(); ++i)
    {
    cmProcess* p = *i;
    this->EndTest(p);
    }
  return true;
}

void cmCTestMultiProcessHandler::EndTest(cmProcess* p)
{
  // Should have a way of getting this stuff from the 
  // launched ctest, maybe a temp file or some extra xml
  // stuff in the stdout
  // Need things like Reason and ExecutionTime, Path, etc.
  int test = p->GetId();
  int exitVal = p->GetExitValue();
  cmCTestTestHandler::cmCTestTestResult cres;
  cres.Properties = 0;
  cres.ExecutionTime = p->GetTotalTime();
  cres.ReturnValue = exitVal;
  cres.Status = cmCTestTestHandler::COMPLETED;
  cres.TestCount = test;  
  cres.Name = this->TestNames[test];
  cres.Path = "";
  if(exitVal)
    {
    cres.Status = cmCTestTestHandler::FAILED;
    this->Failed->push_back(this->TestNames[test]);
    }
  else
    {
    this->Passed->push_back(this->TestNames[test]);
    }
  this->TestResults->push_back(cres);
  // remove test from depend of all other tests
  for(TestMap::iterator i = this->Tests.begin();
       i!=  this->Tests.end(); ++i)
    {
    i->second.erase(test);
    }
  this->TestFinishMap[test] = true;
  this->TestRunningMap[test] = false;
  this->RunningTests.erase(p);
  delete p;
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, 
             "finish test " << test << "\n");
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
