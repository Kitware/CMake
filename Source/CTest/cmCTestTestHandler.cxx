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

#include "cmCTestTestHandler.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include <cmsys/Process.h>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/Base64.h>
#include "cmMakefile.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmCommand.h"

#include <stdlib.h> 
#include <math.h>
#include <float.h>

#include <memory> // auto_ptr

//----------------------------------------------------------------------
class cmCTestSubdirCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    cmCTestSubdirCommand* c = new cmCTestSubdirCommand;
    c->m_TestHandler = m_TestHandler;
    return c;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SUBDIRS";}

  // Unused methods
  virtual const char* GetTerseDocumentation() { return ""; }
  virtual const char* GetFullDocumentation() { return ""; }

  cmTypeMacro(cmCTestSubdirCommand, cmCommand);

  cmCTestTestHandler* m_TestHandler;
};

//----------------------------------------------------------------------
bool cmCTestSubdirCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string>::const_iterator it;
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory(); 
  for ( it = args.begin(); it != args.end(); ++ it )
    {
    cmSystemTools::ChangeDirectory(cwd.c_str());
    std::string fname = cwd;
    fname += "/";
    fname += *it;
 
    if ( !cmSystemTools::FileExists(fname.c_str()) )
      {
      // No subdirectory? So what...
      continue;
      }
    cmSystemTools::ChangeDirectory(fname.c_str());
    const char* testFilename;
    if( cmSystemTools::FileExists("CTestTestfile.cmake") )
      {
      // does the CTestTestfile.cmake exist ?
      testFilename = "CTestTestfile.cmake";
      }
    else if( cmSystemTools::FileExists("DartTestfile.txt") )
      {
      // does the DartTestfile.txt exist ?
      testFilename = "DartTestfile.txt";
      }
    else
      {
      // No DartTestfile.txt? Who cares...
      cmSystemTools::ChangeDirectory(cwd.c_str());
      continue;
      }
    fname += "/";
    fname += testFilename;
    bool readit = m_Makefile->ReadListFile( m_Makefile->GetCurrentListFile(), fname.c_str());
    cmSystemTools::ChangeDirectory(cwd.c_str());
    if(!readit)
      {
      std::string m = "Could not find include file: ";
      m += fname;
      this->SetError(m.c_str());
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------
class cmCTestAddTestCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    cmCTestAddTestCommand* c = new cmCTestAddTestCommand;
    c->m_TestHandler = m_TestHandler;
    return c;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ADD_TEST";}

  // Unused methods
  virtual const char* GetTerseDocumentation() { return ""; }
  virtual const char* GetFullDocumentation() { return ""; }

  cmTypeMacro(cmCTestAddTestCommand, cmCommand);

  cmCTestTestHandler* m_TestHandler;
};

//----------------------------------------------------------------------
bool cmCTestAddTestCommand::InitialPass(std::vector<std::string> const& args)
{
  if ( args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  return m_TestHandler->AddTest(args);
}

//----------------------------------------------------------------------
// Try to find an executable, if found fullPath will be set to the full path
// of where it was found. The directory and filename to search for are passed
// in as well an a subdir (typically used for configuraitons such as
// Release/Debug/etc)
bool TryExecutable(const char *dir, const char *file,
                   std::string *fullPath, const char *subdir)
{
  // try current directory
  std::string tryPath;
  if (dir && strcmp(dir,""))
    {
    tryPath = dir;
    tryPath += "/";
    }
  
  if (subdir && strcmp(subdir,""))
    {
    tryPath += subdir;
    tryPath += "/";
    }
  
  tryPath += file;
  
  // find the file without an executable extension
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }

  // if not found try it with the executable extension
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }
  
  // not found at all, return false
  return false;
}

//----------------------------------------------------------------------
// get the next number in a string with numbers separated by ,
// pos is the start of the search and pos2 is the end of the search
// pos becomes pos2 after a call to GetNextNumber.   
// -1 is returned at the end of the list.
inline int GetNextNumber(std::string const& in, 
                         int& val,
                         std::string::size_type& pos,
                         std::string::size_type& pos2)
{
  pos2 = in.find(',', pos);
  if(pos2 != in.npos)
    {
    if(pos2-pos == 0)
      {
      val = -1;
      }
    else
      {
      val = atoi(in.substr(pos, pos2-pos).c_str());
      }
    pos = pos2+1;
    return 1;
    }
  else
    {
    if(in.size()-pos == 0)
      {
       val = -1;
      }
    else
      {
      val = atoi(in.substr(pos, in.size()-pos).c_str());
      }
    return 0;
    }
}

//----------------------------------------------------------------------
// get the next number in a string with numbers separated by ,
// pos is the start of the search and pos2 is the end of the search
// pos becomes pos2 after a call to GetNextNumber.   
// -1 is returned at the end of the list.
inline int GetNextRealNumber(std::string const& in, 
                             double& val,
                             std::string::size_type& pos,
                             std::string::size_type& pos2)
{
  pos2 = in.find(',', pos);
  if(pos2 != in.npos)
    {
    if(pos2-pos == 0)
      {
      val = -1;
      }
    else
      {
      val = atof(in.substr(pos, pos2-pos).c_str());
      }
    pos = pos2+1;
    return 1;
    }
  else
    {
    if(in.size()-pos == 0)
      {
       val = -1;
      }
    else
      {
      val = atof(in.substr(pos, in.size()-pos).c_str());
      }
    return 0;
    }
}

                         
//----------------------------------------------------------------------
cmCTestTestHandler::cmCTestTestHandler()
{
  m_UseUnion = false;
  
  m_UseIncludeRegExp       = false;
  m_UseExcludeRegExp       = false;
  m_UseExcludeRegExpFirst  = false;

  m_CustomMaximumPassedTestOutputSize = 1 * 1024;
  m_CustomMaximumFailedTestOutputSize = 300 * 1024;
  
  m_MemCheck = false;

  m_LogFile = 0;

  m_DartStuff.compile("(<DartMeasurement.*/DartMeasurement[a-zA-Z]*>)");
}

//----------------------------------------------------------------------
void cmCTestTestHandler::Initialize()
{
  this->Superclass::Initialize();

  m_ElapsedTestingTime = -1;

  m_TestResults.clear();

  m_CustomTestsIgnore.clear();
  m_StartTest = "";
  m_EndTest = "";

  m_CustomPreTest.clear();
  m_CustomPostTest.clear();
  m_CustomMaximumPassedTestOutputSize = 1 * 1024;
  m_CustomMaximumFailedTestOutputSize = 300 * 1024;

  m_TestsToRun.clear();

  m_UseIncludeRegExp = false;
  m_UseExcludeRegExp = false;
  m_UseExcludeRegExpFirst = false;
  m_IncludeRegExp = "";
  m_ExcludeRegExp = "";

  TestsToRunString = "";
  m_UseUnion = false;
  m_TestList.clear();
}

//----------------------------------------------------------------------
void cmCTestTestHandler::PopulateCustomVectors(cmMakefile *mf)
{
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_TEST", 
                                m_CustomPreTest);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_POST_TEST", 
                                m_CustomPostTest);
  cmCTest::PopulateCustomVector(mf,
                             "CTEST_CUSTOM_TESTS_IGNORE", 
                             m_CustomTestsIgnore);
  cmCTest::PopulateCustomInteger(mf, 
                             "CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE", 
                             m_CustomMaximumPassedTestOutputSize);
  cmCTest::PopulateCustomInteger(mf, 
                             "CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE", 
                             m_CustomMaximumFailedTestOutputSize);
}

//----------------------------------------------------------------------
int cmCTestTestHandler::PreProcessHandler()
{
  if ( !this->ExecuteCommands(m_CustomPreTest) )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Problem executing pre-test command(s)." << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCTestTestHandler::PostProcessHandler()
{
  if ( !this->ExecuteCommands(m_CustomPostTest) )
    {
    cmCTestLog(m_CTest, ERROR_MESSAGE, "Problem executing post-test command(s)." << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestTestHandler::ProcessHandler()
{
  // Update internal data structure from generic one
  this->SetTestsToRunInformation(this->GetOption("TestsToRunInformation"));
  this->SetUseUnion(cmSystemTools::IsOn(this->GetOption("UseUnion")));
  const char* val;
  val = this->GetOption("IncludeRegularExpression");
  if ( val )
    {
    this->UseIncludeRegExp();
    this->SetIncludeRegExp(val);
    }
  val = this->GetOption("ExcludeRegularExpression");
  if ( val )
    {
    this->UseExcludeRegExp();
    this->SetExcludeRegExp(val);
    }

  m_TestResults.clear();

  cmCTestLog(m_CTest, HANDLER_OUTPUT, (m_MemCheck ? "Memory check" : "Test") << " project" << std::endl);
  if ( ! this->PreProcessHandler() )
    {
    return -1;
    }

  cmGeneratedFileStream mLogFile;
  this->StartLogFile("Tests", mLogFile);
  m_LogFile = &mLogFile;

  std::vector<cmStdString> passed;
  std::vector<cmStdString> failed;
  int total;

  this->ProcessDirectory(passed, failed);

  total = int(passed.size()) + int(failed.size());

  if (total == 0)
    {
    if ( !m_CTest->GetShowOnly() )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "No tests were found!!!" << std::endl);
      }
    }
  else
    {
    if (m_HandlerVerbose && passed.size() && 
      (m_UseIncludeRegExp || m_UseExcludeRegExp)) 
      {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, std::endl << "The following tests passed:" << std::endl);
      for(std::vector<cmStdString>::iterator j = passed.begin();
          j != passed.end(); ++j)
        {   
        cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "\t" << *j << std::endl);
        }
      }

    float percent = float(passed.size()) * 100.0f / total;
    if ( failed.size() > 0 &&  percent > 99)
      {
      percent = 99;
      }
    cmCTestLog(m_CTest, HANDLER_OUTPUT, std::endl << static_cast<int>(percent + .5) << "% tests passed, "
      << failed.size() << " tests failed out of " << total << std::endl);
    //fprintf(stderr,"\n%.0f%% tests passed, %i tests failed out of %i\n",
    //  percent, int(failed.size()), total);

    if (failed.size()) 
      {
      cmGeneratedFileStream ofs;

      cmCTestLog(m_CTest, ERROR_MESSAGE, std::endl << "The following tests FAILED:" << std::endl);
      this->StartLogFile("TestsFailed", ofs);

      std::vector<cmCTestTestHandler::cmCTestTestResult>::iterator ftit;
      for(ftit = m_TestResults.begin();
        ftit != m_TestResults.end(); ++ftit)
        {
        if ( ftit->m_Status != cmCTestTestHandler::COMPLETED )
          {
          ofs << ftit->m_TestCount << ":" << ftit->m_Name << std::endl;
          cmCTestLog(m_CTest, HANDLER_OUTPUT, "\t" << std::setw(3) << ftit->m_TestCount << " - " << ftit->m_Name.c_str() << " (" << this->GetTestStatus(ftit->m_Status) << ")" << std::endl);
          //fprintf(stderr, "\t%3d - %s (%s)\n", ftit->m_TestCount, ftit->m_Name.c_str(),
          //  this->GetTestStatus(ftit->m_Status));
          }
        }

      }
    }

  if ( m_CTest->GetProduceXML() )
    {
    cmGeneratedFileStream xmlfile;
    if( !this->StartResultingXML((m_MemCheck ? "DynamicAnalysis" : "Test"), xmlfile) )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot create " << (m_MemCheck ? "memory check" : "testing")
        << " XML file" << std::endl);
      m_LogFile = 0;
      return 1;
      }
    this->GenerateDartOutput(xmlfile);
    }

  if ( ! this->PostProcessHandler() )
    {
    m_LogFile = 0;
    return -1;
    }

  if ( !failed.empty() )
    {
    m_LogFile = 0;
    return -1;
    }
  m_LogFile = 0;
  return 0;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::ProcessDirectory(std::vector<cmStdString> &passed, 
                                          std::vector<cmStdString> &failed)
{
  std::string current_dir = cmSystemTools::GetCurrentWorkingDirectory();
  m_TestList.clear();

  this->GetListOfTests();
  tm_ListOfTests::size_type tmsize = m_TestList.size();

  m_StartTest = m_CTest->CurrentTime();
  double elapsed_time_start = cmSystemTools::GetTime();

  *m_LogFile << "Start testing: " << m_StartTest << std::endl
    << "----------------------------------------------------------"
    << std::endl;

  // how many tests are in based on RegExp?
  int inREcnt = 0;
  tm_ListOfTests::iterator it;
  for ( it = m_TestList.begin(); it != m_TestList.end(); it ++ )
    {
    if (it->m_IsInBasedOnREOptions)
      {
      inREcnt ++;
      }
    }
  // expand the test list based on the union flag
  if (m_UseUnion)
    {
    this->ExpandTestsToRunInformation((int)tmsize);
    }
  else
    {
    this->ExpandTestsToRunInformation(inREcnt);
    }
  
  int cnt = 0;
  inREcnt = 0;
  std::string last_directory = "";
  for ( it = m_TestList.begin(); it != m_TestList.end(); it ++ )
    {
    cnt ++;
    if (it->m_IsInBasedOnREOptions)
      {
      inREcnt++;
      }
    const std::string& testname = it->m_Name;
    std::vector<std::string>& args = it->m_Args;
    cmCTestTestResult cres;
    cres.m_Status = cmCTestTestHandler::NOT_RUN;
    cres.m_TestCount = cnt;

    if (!(last_directory == it->m_Directory))
      {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Changing directory into " 
        << it->m_Directory.c_str() << "\n");
      *m_LogFile << "Changing directory into: " << it->m_Directory.c_str()
        << std::endl;
      last_directory = it->m_Directory;
      cmSystemTools::ChangeDirectory(it->m_Directory.c_str());
      }
    cres.m_Name = testname;
    cres.m_Path = it->m_Directory.c_str();
    
    if (m_UseUnion)
      {
      // if it is not in the list and not in the regexp then skip
      if ((m_TestsToRun.size() && 
           std::find(m_TestsToRun.begin(), m_TestsToRun.end(), cnt) 
           == m_TestsToRun.end()) && !it->m_IsInBasedOnREOptions)
        {
        continue;
        }
      }
    else
      {
      // is this test in the list of tests to run? If not then skip it
      if ((m_TestsToRun.size() && 
           std::find(m_TestsToRun.begin(), m_TestsToRun.end(), inREcnt) 
           == m_TestsToRun.end()) || !it->m_IsInBasedOnREOptions)
        {
        continue;
        }
      }

    cmCTestLog(m_CTest, HANDLER_OUTPUT, std::setw(3) << cnt << "/");
    cmCTestLog(m_CTest, HANDLER_OUTPUT, std::setw(3) << tmsize << " Testing ");
    std::string outname = testname;
    outname.resize(30, ' ');
    *m_LogFile << cnt << "/" << tmsize << " Testing: " << testname
      << std::endl;

    if ( m_CTest->GetShowOnly() )
      {
      cmCTestLog(m_CTest, HANDLER_OUTPUT, outname.c_str() << std::endl);
      }
    else
      {
      cmCTestLog(m_CTest, HANDLER_OUTPUT, outname.c_str());
      }
    
    cmCTestLog(m_CTest, DEBUG, "Testing " << args[0].c_str() << " ... ");
    // find the test executable
    std::string actualCommand = this->FindTheExecutable(args[1].c_str());
    std::string testCommand = cmSystemTools::ConvertToOutputPath(actualCommand.c_str());

    // continue if we did not find the executable
    if (testCommand == "")
      {
      *m_LogFile << "Unable to find executable: " << args[1].c_str()
        << std::endl;
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Unable to find executable: "
        << args[1].c_str() << std::endl);
      if ( !m_CTest->GetShowOnly() )
        {
        cres.m_FullCommandLine = actualCommand;
        m_TestResults.push_back( cres ); 
        failed.push_back(testname);
        continue;
        }
      }

    // add the arguments
    std::vector<std::string>::const_iterator j = args.begin();
    ++j;
    ++j;
    std::vector<const char*> arguments;
    this->GenerateTestCommand(arguments);
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


    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, std::endl << (m_MemCheck?"MemCheck":"Test") << " command: " << testCommand << std::endl);
    *m_LogFile << cnt << "/" << tmsize 
      << " Test: " << testname.c_str() << std::endl;
    *m_LogFile << "Command: ";
    std::vector<cmStdString>::size_type ll;
    for ( ll = 0; ll < arguments.size()-1; ll ++ )
      {
      *m_LogFile << "\"" << arguments[ll] << "\" ";
      }
    *m_LogFile 
      << std::endl 
      << "Directory: " << it->m_Directory << std::endl 
      << "\"" << testname.c_str() << "\" start time: " 
      << m_CTest->CurrentTime() << std::endl
      << "Output:" << std::endl 
      << "----------------------------------------------------------"
      << std::endl;
    int res = 0;
    double clock_start, clock_finish;
    clock_start = cmSystemTools::GetTime();

    if ( !m_CTest->GetShowOnly() )
      {
      res = m_CTest->RunTest(arguments, &output, &retVal, m_LogFile);
      }

    clock_finish = cmSystemTools::GetTime();

    if ( m_LogFile )
      {
      double ttime = clock_finish - clock_start;
      int hours = static_cast<int>(ttime / (60 * 60));
      int minutes = static_cast<int>(ttime / 60) % 60;
      int seconds = static_cast<int>(ttime) % 60;
      char buffer[100];
      sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
      *m_LogFile 
        << "----------------------------------------------------------"
        << std::endl
        << "\"" << testname.c_str() << "\" end time: " 
        << m_CTest->CurrentTime() << std::endl
        << "\"" << testname.c_str() << "\" time elapsed: " 
        << buffer << std::endl
        << "----------------------------------------------------------"
        << std::endl << std::endl;
      }

    cres.m_ExecutionTime = (double)(clock_finish - clock_start);
    cres.m_FullCommandLine = testCommand;

    if ( !m_CTest->GetShowOnly() )
      {
      if (res == cmsysProcess_State_Exited && retVal == 0)
        {
        cmCTestLog(m_CTest, HANDLER_OUTPUT,   "   Passed" << std::endl);
        passed.push_back(testname);
        cres.m_Status = cmCTestTestHandler::COMPLETED;
        }
      else
        {
        cres.m_Status = cmCTestTestHandler::FAILED;
        if ( res == cmsysProcess_State_Expired )
          {
          cmCTestLog(m_CTest, HANDLER_OUTPUT, "***Timeout" << std::endl);
          cres.m_Status = cmCTestTestHandler::TIMEOUT;
          }
        else if ( res == cmsysProcess_State_Exception )
          {
          cmCTestLog(m_CTest, HANDLER_OUTPUT, "***Exception: ");
          switch ( retVal )
            {
          case cmsysProcess_Exception_Fault:
            cmCTestLog(m_CTest, HANDLER_OUTPUT, "SegFault");
            cres.m_Status = cmCTestTestHandler::SEGFAULT;
            break;
          case cmsysProcess_Exception_Illegal:
            cmCTestLog(m_CTest, HANDLER_OUTPUT, "Illegal");
            cres.m_Status = cmCTestTestHandler::ILLEGAL;
            break;
          case cmsysProcess_Exception_Interrupt:
            cmCTestLog(m_CTest, HANDLER_OUTPUT, "Interrupt");
            cres.m_Status = cmCTestTestHandler::INTERRUPT;
            break;
          case cmsysProcess_Exception_Numerical:
            cmCTestLog(m_CTest, HANDLER_OUTPUT, "Numerical");
            cres.m_Status = cmCTestTestHandler::NUMERICAL;
            break;
          default:
            cmCTestLog(m_CTest, HANDLER_OUTPUT, "Other");
            cres.m_Status = cmCTestTestHandler::OTHER_FAULT;
            }
           cmCTestLog(m_CTest, HANDLER_OUTPUT, std::endl);
          }
        else if ( res == cmsysProcess_State_Error )
          {
          cmCTestLog(m_CTest, HANDLER_OUTPUT, "***Bad command " << res << std::endl);
          cres.m_Status = cmCTestTestHandler::BAD_COMMAND;
          }
        else
          {
          cmCTestLog(m_CTest, HANDLER_OUTPUT, "***Failed" << std::endl);
          }
        failed.push_back(testname);
        }
      if (!output.empty() && output.find("<DartMeasurement") != output.npos)
        {
        if (m_DartStuff.find(output.c_str()))
          {
          std::string dartString = m_DartStuff.match(1);
          cmSystemTools::ReplaceString(output, dartString.c_str(),"");
          cres.m_RegressionImages = this->GenerateRegressionImages(dartString);
          }
        }
      }

    if ( cres.m_Status == cmCTestTestHandler::COMPLETED )
      {
      this->CleanTestOutput(output, static_cast<size_t>(m_CustomMaximumPassedTestOutputSize));
      }
    else
      {
      this->CleanTestOutput(output, static_cast<size_t>(m_CustomMaximumFailedTestOutputSize));
      }

    cres.m_Output = output;
    cres.m_ReturnValue = retVal;
    cres.m_CompletionStatus = "Completed";
    m_TestResults.push_back( cres );
    }

  m_EndTest = m_CTest->CurrentTime();
  m_ElapsedTestingTime = cmSystemTools::GetTime() - elapsed_time_start;
  if ( m_LogFile )
    {
    *m_LogFile << "End testing: " << m_EndTest << std::endl;
    }
  cmSystemTools::ChangeDirectory(current_dir.c_str());
}

//----------------------------------------------------------------------
void cmCTestTestHandler::GenerateTestCommand(std::vector<const char*>&)
{
}

//----------------------------------------------------------------------
void cmCTestTestHandler::GenerateDartOutput(std::ostream& os)
{
  if ( !m_CTest->GetProduceXML() )
    {
    return;
    }

  m_CTest->StartXML(os);
  os << "<Testing>\n"
    << "\t<StartDateTime>" << m_StartTest << "</StartDateTime>\n"
    << "\t<TestList>\n";
  tm_TestResultsVector::size_type cc;
  for ( cc = 0; cc < m_TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &m_TestResults[cc];
    std::string testPath = result->m_Path + "/" + result->m_Name;
    os << "\t\t<Test>" << cmCTest::MakeXMLSafe(
      m_CTest->GetShortPathToFile(testPath.c_str()))
      << "</Test>" << std::endl;
    }
  os << "\t</TestList>\n";
  for ( cc = 0; cc < m_TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &m_TestResults[cc];
    os << "\t<Test Status=\"";
    if ( result->m_Status == cmCTestTestHandler::COMPLETED )
      {
      os << "passed";
      }
    else if ( result->m_Status == cmCTestTestHandler::NOT_RUN )
      {
      os << "notrun";
      }
    else
      {
      os << "failed";
      }
    std::string testPath = result->m_Path + "/" + result->m_Name;
    os << "\">\n"
      << "\t\t<Name>" << cmCTest::MakeXMLSafe(result->m_Name) << "</Name>\n"
      << "\t\t<Path>" << cmCTest::MakeXMLSafe(
        m_CTest->GetShortPathToFile(result->m_Path.c_str())) << "</Path>\n"
      << "\t\t<FullName>" << cmCTest::MakeXMLSafe(
        m_CTest->GetShortPathToFile(testPath.c_str())) << "</FullName>\n"
      << "\t\t<FullCommandLine>" 
      << cmCTest::MakeXMLSafe(result->m_FullCommandLine) 
      << "</FullCommandLine>\n"
      << "\t\t<Results>" << std::endl;
    if ( result->m_Status != cmCTestTestHandler::NOT_RUN )
      {
      if ( result->m_Status != cmCTestTestHandler::COMPLETED || result->m_ReturnValue )
        {
        os << "\t\t\t<NamedMeasurement type=\"text/string\" name=\"Exit Code\"><Value>"
          << this->GetTestStatus(result->m_Status) << "</Value></NamedMeasurement>\n"
          << "\t\t\t<NamedMeasurement type=\"text/string\" name=\"Exit Value\"><Value>"
          << result->m_ReturnValue << "</Value></NamedMeasurement>" << std::endl;
        }
      os << result->m_RegressionImages;
      os << "\t\t\t<NamedMeasurement type=\"numeric/double\" "
        << "name=\"Execution Time\"><Value>"
        << result->m_ExecutionTime << "</Value></NamedMeasurement>\n";
      os 
        << "\t\t\t<NamedMeasurement type=\"text/string\" "
        << "name=\"Completion Status\"><Value>"
        << result->m_CompletionStatus << "</Value></NamedMeasurement>\n";
      }
    os 
      << "\t\t\t<Measurement>\n"
      << "\t\t\t\t<Value>";
    os << cmCTest::MakeXMLSafe(result->m_Output);
    os
      << "</Value>\n"
      << "\t\t\t</Measurement>\n"
      << "\t\t</Results>\n"
      << "\t</Test>" << std::endl;
    }

  os << "\t<EndDateTime>" << m_EndTest << "</EndDateTime>\n"
     << "<ElapsedMinutes>" 
     << static_cast<int>(m_ElapsedTestingTime/6)/10.0
     << "</ElapsedMinutes>"
    << "</Testing>" << std::endl;
  m_CTest->EndXML(os);
}

//----------------------------------------------------------------------
int cmCTestTestHandler::ExecuteCommands(std::vector<cmStdString>& vec)
{
  std::vector<cmStdString>::iterator it;
  for ( it = vec.begin(); it != vec.end(); ++it )
    {
    int retVal = 0;
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Run command: " << *it << std::endl);
    if ( !cmSystemTools::RunSingleCommand(it->c_str(), 0, &retVal, 0, true /*m_Verbose*/) || 
      retVal != 0 )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Problem running command: " << *it << std::endl);
      return 0;
      }
    }
  return 1;
}


//----------------------------------------------------------------------
// Find the appropriate executable to run for a test
std::string cmCTestTestHandler::FindTheExecutable(const char *exe)
{
  std::string fullPath = "";
  std::string dir;
  std::string file;

  cmSystemTools::SplitProgramPath(exe, dir, file);
  // first try to find the executable given a config type subdir if there is one
  if(m_CTest->GetConfigType() != "" && 
    ::TryExecutable(dir.c_str(), file.c_str(), &fullPath, 
      m_CTest->GetConfigType().c_str()))
    {
    return fullPath;
    }

  // next try the current directory as the subdir
  if (::TryExecutable(dir.c_str(),file.c_str(),&fullPath,"."))
    {
    return fullPath;
    }

  // try without the config subdir
  if (::TryExecutable(dir.c_str(),file.c_str(),&fullPath,""))
    {
    return fullPath;
    }

  if ( m_CTest->GetConfigType() == "" )
    {
    // No config type, so try to guess it
    if (::TryExecutable(dir.c_str(),file.c_str(),&fullPath,"Release"))
      {
      return fullPath;
      }

    if (::TryExecutable(dir.c_str(),file.c_str(),&fullPath,"Debug"))
      {
      return fullPath;
      }

    if (::TryExecutable(dir.c_str(),file.c_str(),&fullPath,"MinSizeRel"))
      {
      return fullPath;
      }

    if (::TryExecutable(dir.c_str(),file.c_str(),&fullPath,"RelWithDebInfo"))
      {
      return fullPath;
      }
    }

  // if everything else failed, check the users path, but only if a full path
  // wasn;t specified
  if (dir.size() == 0)
    {
    std::string path = cmSystemTools::FindProgram(file.c_str());
    if (path != "")
      {
      return path;
      }
    }

  if ( m_CTest->GetConfigType() != "" )
    {
    dir += "/";
    dir += m_CTest->GetConfigType();
    dir += "/";
    dir += file;
    cmSystemTools::Error("config type specified on the command line, but test executable not found.",
      dir.c_str());
    return "";
    }
  return fullPath;
}


//----------------------------------------------------------------------
void cmCTestTestHandler::GetListOfTests()
{
  if ( !m_IncludeRegExp.empty() )
    {
    m_IncludeTestsRegularExpression.compile(m_IncludeRegExp.c_str());
    }
  if ( !m_ExcludeRegExp.empty() )
    {
    m_ExcludeTestsRegularExpression.compile(m_ExcludeRegExp.c_str());
    }
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Constructing a list of tests" << std::endl);
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  lg->SetGlobalGenerator(&gg);
  cmMakefile *mf = lg->GetMakefile();
  mf->AddDefinition("CTEST_CONFIGURATION_TYPE", m_CTest->GetConfigType().c_str());

  // Add handler for ADD_TEST
  cmCTestAddTestCommand* newCom1 = new cmCTestAddTestCommand;
  newCom1->m_TestHandler = this;
  cm.AddCommand(newCom1);

  // Add handler for SUBDIR 
  cmCTestSubdirCommand* newCom2 = new cmCTestSubdirCommand;
  newCom2->m_TestHandler = this;
  cm.AddCommand(newCom2);

  const char* testFilename;
  if( cmSystemTools::FileExists("CTestTestfile.cmake") )
    {
    // does the CTestTestfile.cmake exist ?
    testFilename = "CTestTestfile.cmake";
    }
  else if( cmSystemTools::FileExists("DartTestfile.txt") )
    {
    // does the DartTestfile.txt exist ?
    testFilename = "DartTestfile.txt";
    }
  else
    {
    return;
    }

  if ( !mf->ReadListFile(0, testFilename) )
    {
    return;
    }
  if ( cmSystemTools::GetErrorOccuredFlag() )
    {
    return;
    }
  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Done constructing a list of tests" << std::endl);
}

//----------------------------------------------------------------------
void cmCTestTestHandler::UseIncludeRegExp()
{
  this->m_UseIncludeRegExp = true;  
}

//----------------------------------------------------------------------
void cmCTestTestHandler::UseExcludeRegExp()
{
  this->m_UseExcludeRegExp = true;
  this->m_UseExcludeRegExpFirst = this->m_UseIncludeRegExp ? false : true;
}
  
//----------------------------------------------------------------------
const char* cmCTestTestHandler::GetTestStatus(int status)
{
  static const char statuses[][100] = {
    "Not Run",
    "Timeout",
    "SEGFAULT",
    "ILLEGAL",
    "INTERRUPT",
    "NUMERICAL",
    "OTHER_FAULT",
    "Failed",
    "BAD_COMMAND",
    "Completed"
  };

  if ( status < cmCTestTestHandler::NOT_RUN || 
       status > cmCTestTestHandler::COMPLETED )
    {
    return "No Status";
    }
  return statuses[status];
}

//----------------------------------------------------------------------
void cmCTestTestHandler::ExpandTestsToRunInformation(int numTests)
{
  if (this->TestsToRunString.empty())
    {
    return;
    }
  
  int start;
  int end = -1;
  double stride = -1;
  std::string::size_type pos = 0;
  std::string::size_type pos2;
  // read start
  if(GetNextNumber(this->TestsToRunString, start, pos, pos2))
    {
    // read end
    if(GetNextNumber(this->TestsToRunString, end, pos, pos2))
      {
      // read stride
      if(GetNextRealNumber(this->TestsToRunString, stride, pos, pos2))
        {
        int val =0;
        // now read specific numbers
        while(GetNextNumber(this->TestsToRunString, val, pos, pos2))
          {
          m_TestsToRun.push_back(val);
          }
        m_TestsToRun.push_back(val);
        }
      }
    }

  // if start is not specified then we assume we start at 1
  if(start == -1)
    {
    start = 1;
    }

  // if end isnot specified then we assume we end with the last test
  if(end == -1)
    {
    end = numTests;
    }
  
  // if the stride wasn't specified then it defaults to 1
  if(stride == -1)
    {
    stride = 1;
    }

  // if we have a range then add it
  if(end != -1 && start != -1 && stride > 0)
    {
    int i = 0;
    while (i*stride + start <= end)
      {
      m_TestsToRun.push_back(static_cast<int>(i*stride+start));
      ++i;
      }
    }

  // sort the array
  std::sort(m_TestsToRun.begin(), m_TestsToRun.end(), std::less<int>());
  // remove duplicates
  std::vector<int>::iterator new_end = 
    std::unique(m_TestsToRun.begin(), m_TestsToRun.end());
  m_TestsToRun.erase(new_end, m_TestsToRun.end());
}

//----------------------------------------------------------------------
// Just for convenience
#define SPACE_REGEX "[ \t\r\n]"
//----------------------------------------------------------------------
std::string cmCTestTestHandler::GenerateRegressionImages(
  const std::string& xml)
{
  cmsys::RegularExpression twoattributes(
    "<DartMeasurement" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression threeattributes(
    "<DartMeasurement" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression fourattributes(
    "<DartMeasurement" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurement>");
  cmsys::RegularExpression measurementfile(
    "<DartMeasurementFile" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*(name|type|encoding|compression)=\"([^\"]*)\"" 
    SPACE_REGEX "*>([^<]*)</DartMeasurementFile>");

  cmOStringStream ostr;
  bool done = false;
  std::string cxml = xml;
  while ( ! done )
    {
    if ( twoattributes.find(cxml) )
      {
      ostr
        << "\t\t\t<NamedMeasurement" 
        << " " << twoattributes.match(1) << "=\"" << twoattributes.match(2) << "\""
        << " " << twoattributes.match(3) << "=\"" << twoattributes.match(4) << "\""
        << "><Value>" << twoattributes.match(5) 
        << "</Value></NamedMeasurement>" 
        << std::endl;
      cxml.erase(twoattributes.start(), twoattributes.end() - twoattributes.start());
      }
    else if ( threeattributes.find(cxml) )
      {
      ostr
        << "\t\t\t<NamedMeasurement" 
        << " " << threeattributes.match(1) << "=\"" << threeattributes.match(2) << "\""
        << " " << threeattributes.match(3) << "=\"" << threeattributes.match(4) << "\""
        << " " << threeattributes.match(5) << "=\"" << threeattributes.match(6) << "\""
        << "><Value>" << threeattributes.match(7) 
        << "</Value></NamedMeasurement>" 
        << std::endl;
      cxml.erase(threeattributes.start(), threeattributes.end() - threeattributes.start());
      }
    else if ( fourattributes.find(cxml) )
      {
      ostr
        << "\t\t\t<NamedMeasurement" 
        << " " << fourattributes.match(1) << "=\"" << fourattributes.match(2) << "\""
        << " " << fourattributes.match(3) << "=\"" << fourattributes.match(4) << "\""
        << " " << fourattributes.match(5) << "=\"" << fourattributes.match(6) << "\""
        << " " << fourattributes.match(7) << "=\"" << fourattributes.match(8) << "\""
        << "><Value>" << fourattributes.match(9) 
        << "</Value></NamedMeasurement>" 
        << std::endl;
      cxml.erase(fourattributes.start(), fourattributes.end() - fourattributes.start());
      }
    else if ( measurementfile.find(cxml) )
      {
      const std::string& filename = 
        cmCTest::CleanString(measurementfile.match(5));
      if ( cmSystemTools::FileExists(filename.c_str()) )
        {
        long len = cmSystemTools::FileLength(filename.c_str());
        if ( len == 0 )
          {
          std::string k1 = measurementfile.match(1);
          std::string v1 = measurementfile.match(2);
          std::string k2 = measurementfile.match(3);
          std::string v2 = measurementfile.match(4);
          if ( cmSystemTools::LowerCase(k1) == "type" )
            {
            v1 = "text/string";
            }
          if ( cmSystemTools::LowerCase(k2) == "type" )
            {
            v2 = "text/string";
            }          
          
          ostr
            << "\t\t\t<NamedMeasurement" 
            << " " << k1 << "=\"" << v1 << "\""
            << " " << k2 << "=\"" << v2 << "\""
            << " encoding=\"none\""
            << "><Value>Image " << filename.c_str() 
            << " is empty</Value></NamedMeasurement>";
          }
        else
          {
          std::ifstream ifs(filename.c_str(), std::ios::in 
#ifdef _WIN32
                            | std::ios::binary
#endif
            );
          unsigned char *file_buffer = new unsigned char [ len + 1 ];
          ifs.read(reinterpret_cast<char*>(file_buffer), len);
          unsigned char *encoded_buffer = new unsigned char [ static_cast<int>(len * 1.5 + 5) ];
          
          unsigned long rlen = cmsysBase64_Encode(file_buffer, len, encoded_buffer, 1);
          unsigned long cc;
          
          ostr
            << "\t\t\t<NamedMeasurement" 
          << " " << measurementfile.match(1) << "=\"" << measurementfile.match(2) << "\""
            << " " << measurementfile.match(3) << "=\"" << measurementfile.match(4) << "\""
            << " encoding=\"base64\""
            << ">" << std::endl << "\t\t\t\t<Value>";
          for ( cc = 0; cc < rlen; cc ++ )
            {
            ostr << encoded_buffer[cc];
            if ( cc % 60 == 0 && cc )
              {
              ostr << std::endl;
              }
            }
          ostr
            << "</Value>" << std::endl << "\t\t\t</NamedMeasurement>" 
            << std::endl;
          delete [] file_buffer;
          delete [] encoded_buffer;
          }
        }
      else
        {
        int idx = 4;
        if ( measurementfile.match(1) == "name" )
          {
          idx = 2;
          }
        ostr
          << "\t\t\t<NamedMeasurement" 
          << " name=\"" << measurementfile.match(idx) << "\""
          << " text=\"text/string\""
          << "><Value>File " << filename.c_str() << " not found</Value></NamedMeasurement>" 
          << std::endl;
        cmCTestLog(m_CTest, HANDLER_OUTPUT, "File \"" << filename.c_str() << "\" not found." << std::endl);
        }
      cxml.erase(measurementfile.start(), measurementfile.end() - measurementfile.start());
      }
    else
      {
      done = true;
      }
    }
  return ostr.str();
}

//----------------------------------------------------------------------
void cmCTestTestHandler::SetIncludeRegExp(const char *arg)
{
  m_IncludeRegExp = arg;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::SetExcludeRegExp(const char *arg)
{
  m_ExcludeRegExp = arg;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::SetTestsToRunInformation(const char* in)
{
  if ( !in )
    {
    return;
    }
  this->TestsToRunString = in;
  // if the argument is a file, then read it and use the contents as the string
  if(cmSystemTools::FileExists(in))
    {
    std::ifstream fin(in);
    unsigned long filelen = cmSystemTools::FileLength(in);
    char* buff = new char[filelen+1];
    fin.getline(buff, filelen);
    buff[fin.gcount()] = 0;
    this->TestsToRunString = buff;
    }
}

//----------------------------------------------------------------------
bool cmCTestTestHandler::CleanTestOutput(std::string& output, size_t remove_threshold)
{
  if ( remove_threshold == 0 )
    {
    return true;
    }
  if ( output.find("CTEST_FULL_OUTPUT") != output.npos )
    {
    return true;
    }
  cmOStringStream ostr;
  std::string::size_type cc;
  std::string::size_type skipsize = 0;
  int inTag = 0;
  int skipped = 0;
  for ( cc = 0; cc < output.size(); cc ++ )
    {
    int ch = output[cc];
    if ( ch < 0 || ch > 255 )
      {
      break;
      }
    if ( ch == '<' )
      {
      inTag = 1;
      }
    if ( !inTag )
      {
      int notskip = 0;
      // Skip
      if ( skipsize < remove_threshold )
        {
        ostr << static_cast<char>(ch);
        notskip = 1;
        }
      skipsize ++;
      if ( notskip && skipsize >= remove_threshold )
        {
        skipped = 1;
        }
      }
    else
      {
      ostr << static_cast<char>(ch);
      }
    if ( ch == '>' )
      {
      inTag = 0;
      }
    }
  if ( skipped )
    {
    ostr << "..." << std::endl << "The rest of the test output was removed since it exceeds the threshold of "
      << remove_threshold << " characters." << std::endl;
    }
  output = ostr.str();
  return true;
}

//----------------------------------------------------------------------
bool cmCTestTestHandler::AddTest(const std::vector<std::string>& args)
{
  const std::string& testname = args[0];
  if (this->m_UseExcludeRegExp &&
    this->m_UseExcludeRegExpFirst &&
    m_ExcludeTestsRegularExpression.find(testname.c_str()))
    {
    return true;
    }
  if ( m_MemCheck )
    {
    std::vector<cmStdString>::iterator it;
    bool found = false;
    for ( it = m_CustomTestsIgnore.begin(); 
      it != m_CustomTestsIgnore.end(); ++ it )
      {
      if ( *it == testname )
        {
        found = true;
        break;
        }
      }
    if ( found )
      {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Ignore memcheck: " << *it << std::endl);
      return true;
      }
    }
  else
    {
    std::vector<cmStdString>::iterator it;
    bool found = false;
    for ( it = m_CustomTestsIgnore.begin(); 
      it != m_CustomTestsIgnore.end(); ++ it )
      {
      if ( *it == testname )
        {
        found = true;
        break;
        }
      }
    if ( found )
      {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Ignore test: " << *it << std::endl);
      return true;
      }
    }

  cmCTestTestProperties test;
  test.m_Name = testname;
  test.m_Args = args;
  test.m_Directory = cmSystemTools::GetCurrentWorkingDirectory();
  test.m_IsInBasedOnREOptions = true;
  if (this->m_UseIncludeRegExp && !m_IncludeTestsRegularExpression.find(testname.c_str()))
    {
    test.m_IsInBasedOnREOptions = false;
    }
  else if (this->m_UseExcludeRegExp &&
    !this->m_UseExcludeRegExpFirst &&
    m_ExcludeTestsRegularExpression.find(testname.c_str()))
    {
    test.m_IsInBasedOnREOptions = false;
    }
  m_TestList.push_back(test);
  return true;
}

