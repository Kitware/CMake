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

#include <stdlib.h> 
#include <math.h>
#include <float.h>

//----------------------------------------------------------------------
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
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }
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
    std::cerr << "Problem executing pre-test command(s)." << std::endl;
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCTestTestHandler::PostProcessHandler()
{
  if ( !this->ExecuteCommands(m_CustomPostTest) )
    {
    std::cerr << "Problem executing post-test command(s)." << std::endl;
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestTestHandler::ProcessHandler()
{
  m_TestResults.clear();

  std::cout << (m_MemCheck ? "Memory check" : "Test") << " project" << std::endl;
  if ( ! this->PreProcessHandler() )
    {
    return -1;
    }

  std::vector<cmStdString> passed;
  std::vector<cmStdString> failed;
  int total;

  this->ProcessDirectory(passed, failed);

  total = int(passed.size()) + int(failed.size());

  if (total == 0)
    {
    if ( !m_CTest->GetShowOnly() )
      {
      std::cerr << "No tests were found!!!\n";
      }
    }
  else
    {
    if (m_Verbose && passed.size() && 
      (m_UseIncludeRegExp || m_UseExcludeRegExp)) 
      {
      std::cerr << "\nThe following tests passed:\n";
      for(std::vector<cmStdString>::iterator j = passed.begin();
          j != passed.end(); ++j)
        {   
        std::cerr << "\t" << *j << "\n";
        }
      }

    float percent = float(passed.size()) * 100.0f / total;
    if ( failed.size() > 0 &&  percent > 99)
      {
      percent = 99;
      }
    fprintf(stderr,"\n%.0f%% tests passed, %i tests failed out of %i\n",
      percent, int(failed.size()), total);

    if (failed.size()) 
      {
      cmGeneratedFileStream ofs;

      std::cerr << "\nThe following tests FAILED:\n";
      m_CTest->OpenOutputFile("Temporary", "LastTestsFailed.log", ofs);

      std::vector<cmCTestTestHandler::cmCTestTestResult>::iterator ftit;
      for(ftit = m_TestResults.begin();
        ftit != m_TestResults.end(); ++ftit)
        {
        if ( ftit->m_Status != cmCTestTestHandler::COMPLETED )
          {
          ofs << ftit->m_TestCount << ":" << ftit->m_Name << std::endl;
          fprintf(stderr, "\t%3d - %s (%s)\n", ftit->m_TestCount, ftit->m_Name.c_str(),
            this->GetTestStatus(ftit->m_Status));
          }
        }

      }
    }

  if ( m_CTest->GetProduceXML() )
    {
    cmGeneratedFileStream xmlfile;
    if( !m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), 
        (m_MemCheck ? "DynamicAnalysis.xml" : "Test.xml"), xmlfile, true) )
      {
      std::cerr << "Cannot create " << (m_MemCheck ? "memory check" : "testing")
        << " XML file" << std::endl;
      return 1;
      }
    this->GenerateDartOutput(xmlfile);
    }

  if ( ! this->PostProcessHandler() )
    {
    return -1;
    }

  if ( !failed.empty() )
    {
    return -1;
    }
  return 0;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::ProcessDirectory(std::vector<cmStdString> &passed, 
                                          std::vector<cmStdString> &failed)
{
  std::string current_dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmsys::RegularExpression dartStuff("(<DartMeasurement.*/DartMeasurement[a-zA-Z]*>)");
  tm_ListOfTests testlist;
  this->GetListOfTests(&testlist);
  tm_ListOfTests::size_type tmsize = testlist.size();

  cmGeneratedFileStream ofs;
  cmGeneratedFileStream *olog = 0;
  if ( !m_CTest->GetShowOnly() && tmsize > 0 && 
    m_CTest->OpenOutputFile("Temporary", 
      (m_MemCheck?"LastMemCheck.log":"LastTest.log"), ofs) )
    {
    olog = &ofs;
    }

  m_StartTest = m_CTest->CurrentTime();
  double elapsed_time_start = cmSystemTools::GetTime();

  if ( olog )
    {
    *olog << "Start testing: " << m_StartTest << std::endl
      << "----------------------------------------------------------"
      << std::endl;
    }

  // how many tests are in based on RegExp?
  int inREcnt = 0;
  tm_ListOfTests::iterator it;
  for ( it = testlist.begin(); it != testlist.end(); it ++ )
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
  for ( it = testlist.begin(); it != testlist.end(); it ++ )
    {
    cnt ++;
    if (it->m_IsInBasedOnREOptions)
      {
      inREcnt++;
      }
    const std::string& testname = it->m_Name;
    tm_VectorOfListFileArgs& args = it->m_Args;
    cmCTestTestResult cres;
    cres.m_Status = cmCTestTestHandler::NOT_RUN;
    cres.m_TestCount = cnt;

    if (!(last_directory == it->m_Directory))
      {
      if ( m_Verbose )
        {
        std::cerr << "Changing directory into " 
          << it->m_Directory.c_str() << "\n";
        }
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
    
    std::cerr.width(3);
    std::cerr << cnt << "/";
    std::cerr.width(3);
    std::cerr << tmsize << " Testing ";
    std::string outname = testname;
    outname.resize(30, ' ');

    if ( m_CTest->GetShowOnly() )
      {
      std::cerr << outname.c_str() << "\n";
      }
    else
      {
      std::cerr << outname.c_str();
      std::cerr.flush();
      }
    
    //std::cerr << "Testing " << args[0] << " ... ";
    // find the test executable
    std::string actualCommand = this->FindTheExecutable(args[1].Value.c_str());
    std::string testCommand = cmSystemTools::ConvertToOutputPath(actualCommand.c_str());

    // continue if we did not find the executable
    if (testCommand == "")
      {
      std::cerr << "Unable to find executable: " <<
        args[1].Value.c_str() << "\n";
      if ( !m_CTest->GetShowOnly() )
        {
        cres.m_FullCommandLine = actualCommand;
        m_TestResults.push_back( cres ); 
        failed.push_back(testname);
        continue;
        }
      }

    // add the arguments
    tm_VectorOfListFileArgs::const_iterator j = args.begin();
    ++j;
    ++j;
    std::vector<const char*> arguments;
    this->GenerateTestCommand(arguments);
    arguments.push_back(actualCommand.c_str());
    for(;j != args.end(); ++j)
      {
      testCommand += " ";
      testCommand += cmSystemTools::EscapeSpaces(j->Value.c_str());
      arguments.push_back(j->Value.c_str());
      }
    arguments.push_back(0);

    /**
     * Run an executable command and put the stdout in output.
     */
    std::string output;
    int retVal = 0;


    if ( m_Verbose )
      {
      std::cout << std::endl << (m_MemCheck?"MemCheck":"Test") << " command: " << testCommand << std::endl;
      }
    if ( olog )
      {
      *olog << cnt << "/" << tmsize 
        << " Test: " << testname.c_str() << std::endl;
      *olog << "Command: ";
      std::vector<cmStdString>::size_type ll;
      for ( ll = 0; ll < arguments.size()-1; ll ++ )
        {
        *olog << "\"" << arguments[ll] << "\" ";
        }
      *olog 
        << std::endl 
        << "Directory: " << it->m_Directory << std::endl 
        << "\"" << testname.c_str() << "\" start time: " 
        << m_CTest->CurrentTime() << std::endl
        << "Output:" << std::endl 
        << "----------------------------------------------------------"
        << std::endl;
      }
    int res = 0;
    double clock_start, clock_finish;
    clock_start = cmSystemTools::GetTime();

    if ( !m_CTest->GetShowOnly() )
      {
      res = m_CTest->RunTest(arguments, &output, &retVal, olog);
      }

    clock_finish = cmSystemTools::GetTime();

    if ( olog )
      {
      double ttime = clock_finish - clock_start;
      int hours = static_cast<int>(ttime / (60 * 60));
      int minutes = static_cast<int>(ttime / 60) % 60;
      int seconds = static_cast<int>(ttime) % 60;
      char buffer[100];
      sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
      *olog 
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
        std::cerr <<   "   Passed\n";
        passed.push_back(testname);
        cres.m_Status = cmCTestTestHandler::COMPLETED;
        }
      else
        {
        cres.m_Status = cmCTestTestHandler::FAILED;
        if ( res == cmsysProcess_State_Expired )
          {
          std::cerr << "***Timeout\n";
          cres.m_Status = cmCTestTestHandler::TIMEOUT;
          }
        else if ( res == cmsysProcess_State_Exception )
          {
          std::cerr << "***Exception: ";
          switch ( retVal )
            {
          case cmsysProcess_Exception_Fault:
            std::cerr << "SegFault";
            cres.m_Status = cmCTestTestHandler::SEGFAULT;
            break;
          case cmsysProcess_Exception_Illegal:
            std::cerr << "Illegal";
            cres.m_Status = cmCTestTestHandler::ILLEGAL;
            break;
          case cmsysProcess_Exception_Interrupt:
            std::cerr << "Interrupt";
            cres.m_Status = cmCTestTestHandler::INTERRUPT;
            break;
          case cmsysProcess_Exception_Numerical:
            std::cerr << "Numerical";
            cres.m_Status = cmCTestTestHandler::NUMERICAL;
            break;
          default:
            std::cerr << "Other";
            cres.m_Status = cmCTestTestHandler::OTHER_FAULT;
            }
           std::cerr << "\n";
          }
        else if ( res == cmsysProcess_State_Error )
          {
          std::cerr << "***Bad command " << res << "\n";
          cres.m_Status = cmCTestTestHandler::BAD_COMMAND;
          }
        else
          {
          std::cerr << "***Failed\n";
          }
        failed.push_back(testname);
        }
      if (output != "")
        {
        if (dartStuff.find(output.c_str()))
          {
          std::string dartString = dartStuff.match(1);
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
  if ( olog )
    {
    *olog << "End testing: " << m_EndTest << std::endl;
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
    if ( m_Verbose )
      {
      std::cout << "Run command: " << *it << std::endl;
      }
    if ( !cmSystemTools::RunSingleCommand(it->c_str(), 0, &retVal, 0, true /*m_Verbose*/) || 
      retVal != 0 )
      {
      std::cerr << "Problem running command: " << *it << std::endl;
      return 0;
      }
    }
  return 1;
}


//----------------------------------------------------------------------
std::string cmCTestTestHandler::FindTheExecutable(const char *exe)
{
  std::string fullPath = "";
  std::string dir;
  std::string file;

  cmSystemTools::SplitProgramPath(exe, dir, file);
  if(m_CTest->GetConfigType() != "" && 
    ::TryExecutable(dir.c_str(), file.c_str(), &fullPath, 
      m_CTest->GetConfigType().c_str()))
    {
    return fullPath;
    }

  if (::TryExecutable(dir.c_str(),file.c_str(),&fullPath,"."))
    {
    return fullPath;
    }

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

  // if everything else failed, check the users path
  if (dir != "")
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
void cmCTestTestHandler::GetListOfTests(tm_ListOfTests* testlist)
{
  // does the DartTestfile.txt exist ?
  if(!cmSystemTools::FileExists("DartTestfile.txt"))
    {
    return;
    }

  // parse the file
  std::ifstream fin("DartTestfile.txt");
  if(!fin)
    {
    return;
    }

  cmsys::RegularExpression ireg(this->m_IncludeRegExp.c_str());
  cmsys::RegularExpression ereg(this->m_ExcludeRegExp.c_str());

  cmListFileCache cache;
  cmListFile* listFile = cache.GetFileCache("DartTestfile.txt", false);
  for(std::vector<cmListFileFunction>::const_iterator f =
    listFile->m_Functions.begin(); f != listFile->m_Functions.end(); ++f)
    {
    const cmListFileFunction& lff = *f;
    const std::string& name = lff.m_Name;
    const tm_VectorOfListFileArgs& args = lff.m_Arguments;
    if (name == "SUBDIRS")
      {
      std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
      for(tm_VectorOfListFileArgs::const_iterator j = args.begin();
        j != args.end(); ++j)
        {
        std::string nwd = cwd + "/";
        nwd += j->Value;
        if (cmSystemTools::FileIsDirectory(nwd.c_str()))
          {
          cmSystemTools::ChangeDirectory(nwd.c_str());
          this->GetListOfTests(testlist);
          }
        }
      // return to the original directory
      cmSystemTools::ChangeDirectory(cwd.c_str());
      }

    if (name == "ADD_TEST")
      {
      const std::string& testname = args[0].Value;
      if (this->m_UseExcludeRegExp &&
        this->m_UseExcludeRegExpFirst &&
        ereg.find(testname.c_str()))
        {
        continue;
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
          if ( m_Verbose )
            {
            std::cout << "Ignore memcheck: " << *it << std::endl;
            }
          continue;
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
          if ( m_Verbose )
            {
            std::cout << "Ignore test: " << *it << std::endl;
            }
          continue;
          }
        }

      cmCTestTestProperties test;
      test.m_Name = testname;
      test.m_Args = args;
      test.m_Directory = cmSystemTools::GetCurrentWorkingDirectory();
      test.m_IsInBasedOnREOptions = true;
      if (this->m_UseIncludeRegExp && !ireg.find(testname.c_str()))
        {
        test.m_IsInBasedOnREOptions = false;
        }
      else if (this->m_UseExcludeRegExp &&
               !this->m_UseExcludeRegExpFirst &&
               ereg.find(testname.c_str()))
        {
        test.m_IsInBasedOnREOptions = false;
        }
      testlist->push_back(test);
      }
    }
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
        std::cout << "File \"" << filename.c_str() << "\" not found." << std::endl;
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

