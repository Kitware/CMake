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
#include <cmsys/Process.h>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/Base64.h>
#include "cmMakefile.h"

#include <stdlib.h> 
#include <math.h>
#include <float.h>

static const char* cmCTestMemCheckResultStrings[] = {
  "ABR",
  "ABW",
  "ABWL",
  "COR",
  "EXU",
  "FFM",
  "FIM",
  "FMM",
  "FMR",
  "FMW",
  "FUM",
  "IPR",
  "IPW",
  "MAF",
  "MLK",
  "MPK",
  "NPR",
  "ODS",
  "PAR",
  "PLK",
  "UMC",
  "UMR",
  0
};

static const char* cmCTestMemCheckResultLongStrings[] = {
  "Threading Problem",
  "ABW",
  "ABWL",
  "COR",
  "EXU",
  "FFM",
  "FIM",
  "Mismatched deallocation",
  "FMR",
  "FMW",
  "FUM",
  "IPR",
  "IPW",
  "MAF",
  "Memory Leak",
  "Potential Memory Leak",
  "NPR",
  "ODS",
  "Invalid syscall param",
  "PLK",
  "Uninitialized Memory Conditional",
  "Uninitialized Memory Read",
  0
};


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
  m_Verbose = false; 
  m_CTest = 0;

  m_UseIncludeRegExp       = false;
  m_UseExcludeRegExp       = false;
  m_UseExcludeRegExpFirst  = false;
}

//----------------------------------------------------------------------
void cmCTestTestHandler::PopulateCustomVectors(cmMakefile *mf)
{
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_TEST", 
                                m_CustomPreTest);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_POST_TEST", 
                                m_CustomPostTest);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_MEMCHECK", 
                                m_CustomPreMemCheck);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_POST_MEMCHECK", 
                                m_CustomPostMemCheck);

  cmCTest::PopulateCustomVector(mf,
                             "CTEST_CUSTOM_TESTS_IGNORE", 
                             m_CustomTestsIgnore);
  cmCTest::PopulateCustomVector(mf, 
                             "CTEST_CUSTOM_MEMCHECK_IGNORE", 
                             m_CustomMemCheckIgnore);
}


//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestTestHandler::TestDirectory(cmCTest *ctest_inst, bool memcheck)
{
  m_CTest = ctest_inst;

  m_TestResults.clear();
  std::cout << (memcheck ? "Memory check" : "Test") << " project" << std::endl;
  if ( memcheck )
    {
    if ( !this->InitializeMemoryChecking() )
      {
      return 1;
      }
    }

  if ( memcheck )
    {
    if ( !this->ExecuteCommands(m_CustomPreMemCheck) )
      {
      std::cerr << "Problem executing pre-memcheck command(s)." << std::endl;
      return 1;
      }
    }
  else
    {
    if ( !this->ExecuteCommands(m_CustomPreTest) )
      {
      std::cerr << "Problem executing pre-test command(s)." << std::endl;
      return 1;
      }
    }

  std::vector<cmStdString> passed;
  std::vector<cmStdString> failed;
  int total;

  this->ProcessDirectory(passed, failed, memcheck);

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
      std::ofstream ofs;

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
    std::ofstream xmlfile;
    if( !m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), 
        (memcheck ? "DynamicAnalysis.xml" : "Test.xml"), xmlfile) )
      {
      std::cerr << "Cannot create " << (memcheck ? "memory check" : "testing")
        << " XML file" << std::endl;
      return 1;
      }
    if ( memcheck )
      {
      this->GenerateDartMemCheckOutput(xmlfile);
      }
    else
      {
      this->GenerateDartTestOutput(xmlfile);
      }
    }

  if ( memcheck )
    {
    if ( !this->ExecuteCommands(m_CustomPostMemCheck) )
      {
      std::cerr << "Problem executing post-memcheck command(s)." << std::endl;
      return 1;
      }
    }
  else
    {
    if ( !this->ExecuteCommands(m_CustomPostTest) )
      {
      std::cerr << "Problem executing post-test command(s)." << std::endl;
      return 1;
      }
    }

  return int(failed.size());
}

void cmCTestTestHandler::ProcessDirectory(std::vector<cmStdString> &passed, 
                                          std::vector<cmStdString> &failed,
                                          bool memcheck)
{
  std::string current_dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmsys::RegularExpression dartStuff("(<DartMeasurement.*/DartMeasurement[a-zA-Z]*>)");
  tm_ListOfTests testlist;
  this->GetListOfTests(&testlist, memcheck);
  tm_ListOfTests::size_type tmsize = testlist.size();

  std::ofstream ofs;
  std::ofstream *olog = 0;
  if ( !m_CTest->GetShowOnly() && tmsize > 0 && 
    m_CTest->OpenOutputFile("Temporary", 
      (memcheck?"LastMemCheck.log":"LastTest.log"), ofs) )
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

  // expand the test list
  this->ExpandTestsToRunInformation((int)tmsize);
  
  int cnt = 0;
  tm_ListOfTests::iterator it;
  std::string last_directory = "";
  for ( it = testlist.begin(); it != testlist.end(); it ++ )
    {
    cnt ++;
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
    if(m_TestsToRun.size() && 
       std::find(m_TestsToRun.begin(), m_TestsToRun.end(), cnt) == m_TestsToRun.end())
      {
      continue;
      }

    if ( m_CTest->GetShowOnly() )
      {
      std::cerr.width(3);
      std::cerr << cnt << "/";
      std::cerr.width(3);
      std::cerr << tmsize << " Testing ";
      std::string outname = testname;
      outname.resize(30, ' ');
      std::cerr << outname.c_str() << "\n";
     }
    else
      {
      std::cerr.width(3);
      std::cerr << cnt << "/";
      std::cerr.width(3);
      std::cerr << tmsize << " Testing ";
      std::string outname = testname;
      outname.resize(30, ' ');
      std::cerr << outname.c_str();
      std::cerr.flush();
      }
    
    //std::cerr << "Testing " << args[0] << " ... ";
    // find the test executable
    std::string actualCommand = this->FindTheExecutable(args[1].Value.c_str());
    std::string testCommand = cmSystemTools::ConvertToOutputPath(actualCommand.c_str());
    std::string memcheckcommand = "";

    // continue if we did not find the executable
    if (testCommand == "")
      {
      std::cerr << "Unable to find executable: " <<
        args[1].Value.c_str() << "\n";
      if ( !m_CTest->GetShowOnly() )
        {
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
    if ( memcheck )
      {
      std::vector<cmStdString>::size_type pp;
      arguments.push_back(m_MemoryTester.c_str());
      memcheckcommand = m_MemoryTester;
      for ( pp = 0; pp < m_MemoryTesterOptionsParsed.size(); pp ++ )
        {
        arguments.push_back(m_MemoryTesterOptionsParsed[pp].c_str());
        memcheckcommand += " ";
        memcheckcommand += cmSystemTools::EscapeSpaces(m_MemoryTesterOptionsParsed[pp].c_str());
        }
      }
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
      std::cout << std::endl << (memcheck?"MemCheck":"Test") << " command: " << testCommand << std::endl;
      if ( memcheck )
        {
        std::cout << "Memory check command: " << memcheckcommand << std::endl;
        }
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
    cres.m_Output = output;
    cres.m_ReturnValue = retVal;
    cres.m_Path = it->m_Directory.c_str();
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

void cmCTestTestHandler::GenerateDartMemCheckOutput(std::ostream& os)
{
  if ( !m_CTest->GetProduceXML() )
    {
    return;
    }

  m_CTest->StartXML(os);
  os << "<DynamicAnalysis Checker=\"";
  switch ( m_MemoryTesterStyle )
    {
    case cmCTestTestHandler::VALGRIND:
      os << "Valgrind";
      break;
    case cmCTestTestHandler::PURIFY:
      os << "Purify";
      break;
    case cmCTestTestHandler::BOUNDS_CHECKER:
      os << "BoundsChecker";
      break;
    default:
      os << "Unknown";
    }
  os << "\">" << std::endl;

  os << "\t<StartDateTime>" << m_StartTest << "</StartDateTime>\n"
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
  std::cout << "-- Processing memory checking output: ";
  unsigned int total = m_TestResults.size();
  unsigned int step = total / 10;
  unsigned int current = 0;
  for ( cc = 0; cc < m_TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &m_TestResults[cc];
    std::string memcheckstr;
    int memcheckresults[cmCTestTestHandler::NO_MEMORY_FAULT];
    int kk;
    bool res = this->ProcessMemCheckOutput(result->m_Output, memcheckstr, memcheckresults);
    if ( res && result->m_Status == cmCTestTestHandler::COMPLETED )
      {
      continue;
      }
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
    for ( kk = 0; cmCTestMemCheckResultLongStrings[kk]; kk ++ )
      {
      if ( memcheckresults[kk] )
        {
        os << "\t\t\t<Defect type=\"" << cmCTestMemCheckResultLongStrings[kk] << "\">"
           << memcheckresults[kk] 
           << "</Defect>" << std::endl;
        }
      m_MemoryTesterGlobalResults[kk] += memcheckresults[kk];
      }
    os 
      << "\t\t</Results>\n"
      << "\t<Log>\n" << memcheckstr << std::endl
      << "\t</Log>\n"
      << "\t</Test>" << std::endl;
    if ( current < cc )
      {
      std::cout << "#";
      std::cout.flush();
      current += step;
      }
    }
  std::cout << std::endl;
  std::cerr << "Memory checking results:" << std::endl;
  os << "\t<DefectList>" << std::endl;
  for ( cc = 0; cmCTestMemCheckResultStrings[cc]; cc ++ )
    {
    if ( m_MemoryTesterGlobalResults[cc] )
      {
      std::cerr.width(35);
      std::cerr << cmCTestMemCheckResultLongStrings[cc] << " - " 
        << m_MemoryTesterGlobalResults[cc] << std::endl;
      os << "\t\t<Defect Type=\"" << cmCTestMemCheckResultLongStrings[cc] << "\"/>" << std::endl;
      }
    }
  os << "\t</DefectList>" << std::endl;

  os << "\t<EndDateTime>" << m_EndTest << "</EndDateTime>" << std::endl;
  os << "<ElapsedMinutes>" 
     << static_cast<int>(m_ElapsedTestingTime/6)/10.0 
     << "</ElapsedMinutes>\n";
  
  os << "</DynamicAnalysis>" << std::endl;
  m_CTest->EndXML(os);


}

void cmCTestTestHandler::GenerateDartTestOutput(std::ostream& os)
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

bool cmCTestTestHandler::InitializeMemoryChecking()
{
  // Setup the command
  if ( cmSystemTools::FileExists(m_CTest->GetDartConfiguration("MemoryCheckCommand").c_str()) )
    {
    m_MemoryTester 
      = cmSystemTools::ConvertToOutputPath(m_CTest->GetDartConfiguration("MemoryCheckCommand").c_str());
    }
  else if ( cmSystemTools::FileExists(m_CTest->GetDartConfiguration("PurifyCommand").c_str()) )
    {
    m_MemoryTester 
      = cmSystemTools::ConvertToOutputPath(m_CTest->GetDartConfiguration("PurifyCommand").c_str());
    }
  else if ( cmSystemTools::FileExists(m_CTest->GetDartConfiguration("ValgrindCommand").c_str()) )
    {
    m_MemoryTester 
      = cmSystemTools::ConvertToOutputPath(m_CTest->GetDartConfiguration("ValgrindCommand").c_str());
    }
  else
    {
    std::cerr << "Memory checker (MemoryCheckCommand) not set, or cannot find the specified program." 
      << std::endl;
    return false;
    }

  if ( m_MemoryTester[0] == '\"' && m_MemoryTester[m_MemoryTester.size()-1] == '\"' )
    {
    m_MemoryTester = m_MemoryTester.substr(1, m_MemoryTester.size()-2);
    }

  // Setup the options
  if ( m_CTest->GetDartConfiguration("MemoryCheckCommandOptions").size() )
    {
    m_MemoryTesterOptions = m_CTest->GetDartConfiguration("MemoryCheckCommandOptions");
    }
  else if ( m_CTest->GetDartConfiguration("ValgrindCommandOptions").size() )
    {
    m_MemoryTesterOptions = m_CTest->GetDartConfiguration("ValgrindCommandOptions");
    }

  m_MemoryTesterOutputFile = m_CTest->GetToplevelPath() + "/Testing/Temporary/MemoryChecker.log";
  m_MemoryTesterOutputFile = cmSystemTools::EscapeSpaces(m_MemoryTesterOutputFile.c_str());

  if ( m_MemoryTester.find("valgrind") != std::string::npos )
    {
    m_MemoryTesterStyle = cmCTestTestHandler::VALGRIND;
    if ( !m_MemoryTesterOptions.size() )
      {
      m_MemoryTesterOptions = "-q --skin=memcheck --leak-check=yes --show-reachable=yes --workaround-gcc296-bugs=yes --num-callers=100";
      }
    if ( m_CTest->GetDartConfiguration("MemoryCheckSuppressionFile").size() )
      {
      if ( !cmSystemTools::FileExists(m_CTest->GetDartConfiguration("MemoryCheckSuppressionFile").c_str()) )
        {
        std::cerr << "Cannot find memory checker suppression file: " 
          << m_CTest->GetDartConfiguration("MemoryCheckSuppressionFile").c_str() << std::endl;
        return false;
        }
      m_MemoryTesterOptions += " --suppressions=" + cmSystemTools::EscapeSpaces(m_CTest->GetDartConfiguration("MemoryCheckSuppressionFile").c_str()) + "";
      }
    }
  else if ( m_MemoryTester.find("purify") != std::string::npos )
    {
    m_MemoryTesterStyle = cmCTestTestHandler::PURIFY;
#ifdef _WIN32
    m_MemoryTesterOptions += " /SAVETEXTDATA=" + m_MemoryTesterOutputFile;
#else
    m_MemoryTesterOptions += " -log-file=" + m_MemoryTesterOutputFile;
#endif
    }
  else if ( m_MemoryTester.find("boundschecker") != std::string::npos )
    {
    m_MemoryTesterStyle = cmCTestTestHandler::BOUNDS_CHECKER;
    std::cerr << "Bounds checker not yet implemented" << std::endl;
    return false;
    }
  else
    {
    std::cerr << "Do not understand memory checker: " << m_MemoryTester.c_str() << std::endl;
    return false;
    }

  m_MemoryTesterOptionsParsed = cmSystemTools::ParseArguments(m_MemoryTesterOptions.c_str());
  std::vector<cmStdString>::size_type cc;
  for ( cc = 0; cmCTestMemCheckResultStrings[cc]; cc ++ )
    {
    m_MemoryTesterGlobalResults[cc] = 0;
    }
  return true;
}

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


void cmCTestTestHandler::GetListOfTests(tm_ListOfTests* testlist, 
                                        bool memcheck)
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
          this->GetListOfTests(testlist, memcheck);
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
      if ( memcheck )
        {
        std::vector<cmStdString>::iterator it;
        bool found = false;
        for ( it = m_CustomMemCheckIgnore.begin(); 
          it != m_CustomMemCheckIgnore.end(); ++ it )
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


      if (this->m_UseIncludeRegExp && !ireg.find(testname.c_str()))
        {
        continue;
        }
      if (this->m_UseExcludeRegExp &&
        !this->m_UseExcludeRegExpFirst &&
        ereg.find(testname.c_str()))
        {
        continue;
        }

      cmCTestTestProperties test;
      test.m_Name = testname;
      test.m_Args = args;
      test.m_Directory = cmSystemTools::GetCurrentWorkingDirectory();
      testlist->push_back(test);
      }
    }
}


void cmCTestTestHandler::UseIncludeRegExp()
{
  this->m_UseIncludeRegExp = true;  
}

void cmCTestTestHandler::UseExcludeRegExp()
{
  this->m_UseExcludeRegExp = true;
  this->m_UseExcludeRegExpFirst = this->m_UseIncludeRegExp ? false : true;
}
  
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
  std::cout << "Running tests: ";
  for(unsigned int i =0; i < m_TestsToRun.size(); ++i)
    {
    std::cout << m_TestsToRun[i] << " ";
    }
  std::cout << "\n";
}

#define SPACE_REGEX "[ \t\r\n]"

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

bool cmCTestTestHandler::ProcessMemCheckOutput(const std::string& str, 
                                               std::string& log, int* results)
{
  std::string::size_type cc;
  for ( cc = 0; cc < cmCTestTestHandler::NO_MEMORY_FAULT; cc ++ )
    {
    results[cc] = 0;
    }

  if ( m_MemoryTesterStyle == cmCTestTestHandler::VALGRIND )
    {
    return this->ProcessMemCheckValgrindOutput(str, log, results);
    }
  else if ( m_MemoryTesterStyle == cmCTestTestHandler::PURIFY )
    {
    return this->ProcessMemCheckPurifyOutput(str, log, results);
    }
  else if ( m_MemoryTesterStyle == cmCTestTestHandler::BOUNDS_CHECKER )
    {
    log.append("\nMemory checking style used was: ");
    log.append("Bounds Checker");
    }
  else
    {
    log.append("\nMemory checking style used was: ");
    log.append("None that I know");
    log = str;
    }


  return true;
}

bool cmCTestTestHandler::ProcessMemCheckPurifyOutput(
  const std::string&, std::string& log, 
  int* results)
{
  if ( !cmSystemTools::FileExists(m_MemoryTesterOutputFile.c_str()) )
    {
    log = "Cannot find Purify output file: " + m_MemoryTesterOutputFile;
    std::cerr << log.c_str() << std::endl;
    return false;
    }

  std::ifstream ifs(m_MemoryTesterOutputFile.c_str());
  if ( !ifs )
    {
    log = "Cannot read Purify output file: " + m_MemoryTesterOutputFile;
    std::cerr << log.c_str() << std::endl;
    return false;
    }

  cmOStringStream ostr;
  log = "";

  cmsys::RegularExpression pfW("^\\[[WEI]\\] ([A-Z][A-Z][A-Z][A-Z]*): ");

  int defects = 0;

  std::string line;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    int failure = cmCTestTestHandler::NO_MEMORY_FAULT;
    if ( pfW.find(line) )
      {
      int cc;
      for ( cc = 0; cc < cmCTestTestHandler::NO_MEMORY_FAULT; cc ++ )
        {
        if ( pfW.match(1) == cmCTestMemCheckResultStrings[cc] )
          {
          failure = cc;
          break;
          }
        }
      if ( cc == cmCTestTestHandler::NO_MEMORY_FAULT )
        {
        std::cerr<< "Unknown Purify memory fault: " << pfW.match(1) << std::endl;
        ostr << "*** Unknown Purify memory fault: " << pfW.match(1) << std::endl;
        }
      }
    if ( failure != NO_MEMORY_FAULT )
      {
      ostr << "<b>" << cmCTestMemCheckResultStrings[failure] << "</b> ";
      results[failure] ++;
      defects ++;
      }
    ostr << cmCTest::MakeXMLSafe(line) << std::endl;
    }

  log = ostr.str();
  if ( defects )
    {
    return false;
    }
  return true;
}

bool cmCTestTestHandler::ProcessMemCheckValgrindOutput(
  const std::string& str, std::string& log, 
  int* results)
{
  std::vector<cmStdString> lines;
  cmSystemTools::Split(str.c_str(), lines);
 
  std::string::size_type cc;

  cmOStringStream ostr;
  log = "";

  int defects = 0;

  cmsys::RegularExpression valgrindLine("^==[0-9][0-9]*==");

  cmsys::RegularExpression vgFIM(
    "== .*Invalid free\\(\\) / delete / delete\\[\\]");
  cmsys::RegularExpression vgFMM(
    "== .*Mismatched free\\(\\) / delete / delete \\[\\]");
  cmsys::RegularExpression vgMLK(
    "== .*[0-9][0-9]* bytes in [0-9][0-9]* blocks are definitely lost"
    " in loss record [0-9][0-9]* of [0-9]");
  cmsys::RegularExpression vgPAR(
    "== .*Syscall param .* contains unaddressable byte\\(s\\)");
  cmsys::RegularExpression vgMPK1(
    "== .*[0-9][0-9]* bytes in [0-9][0-9]* blocks are possibly lost in"
    " loss record [0-9][0-9]* of [0-9]");
  cmsys::RegularExpression vgMPK2(
    "== .*[0-9][0-9]* bytes in [0-9][0-9]* blocks are still reachable"
    " in loss record [0-9][0-9]* of [0-9]");
  cmsys::RegularExpression vgUMC(
    "== .*Conditional jump or move depends on uninitialised value\\(s\\)");
  cmsys::RegularExpression vgUMR1("== .*Use of uninitialised value of size [0-9][0-9]*");
  cmsys::RegularExpression vgUMR2("== .*Invalid read of size [0-9][0-9]*");
  cmsys::RegularExpression vgUMR3("== .*Jump to the invalid address ");
  cmsys::RegularExpression vgUMR4(
    "== .*Syscall param .* contains uninitialised or unaddressable byte\\(s\\)");
  cmsys::RegularExpression vgIPW("== .*Invalid write of size [0-9]");
  cmsys::RegularExpression vgABR("== .*pthread_mutex_unlock: mutex is locked by a different thread");

  //double sttime = cmSystemTools::GetTime();
  //std::cout << "Start test: " << lines.size() << std::endl;
  for ( cc = 0; cc < lines.size(); cc ++ )
    {
    if ( valgrindLine.find(lines[cc]) )
      {
      int failure = cmCTestTestHandler::NO_MEMORY_FAULT;
      if ( vgFIM.find(lines[cc]) ) { failure = cmCTestTestHandler::FIM; }
      else if ( vgFMM.find(lines[cc]) ) { failure = cmCTestTestHandler::FMM; }
      else if ( vgMLK.find(lines[cc]) ) { failure = cmCTestTestHandler::MLK; }
      else if ( vgPAR.find(lines[cc]) ) { failure = cmCTestTestHandler::PAR; }
      else if ( vgMPK1.find(lines[cc]) ){ failure = cmCTestTestHandler::MPK; }
      else if ( vgMPK2.find(lines[cc]) ){ failure = cmCTestTestHandler::MPK; }
      else if ( vgUMC.find(lines[cc]) ) { failure = cmCTestTestHandler::UMC; }
      else if ( vgUMR1.find(lines[cc]) ){ failure = cmCTestTestHandler::UMR; }
      else if ( vgUMR2.find(lines[cc]) ){ failure = cmCTestTestHandler::UMR; }
      else if ( vgUMR3.find(lines[cc]) ){ failure = cmCTestTestHandler::UMR; }
      else if ( vgUMR4.find(lines[cc]) ){ failure = cmCTestTestHandler::UMR; }
      else if ( vgIPW.find(lines[cc]) ) { failure = cmCTestTestHandler::IPW; }
      else if ( vgABR.find(lines[cc]) ) { failure = cmCTestTestHandler::ABR; }

      if ( failure != cmCTestTestHandler::NO_MEMORY_FAULT )
        {
        ostr << "<b>" << cmCTestMemCheckResultStrings[failure] << "</b> ";
        results[failure] ++;
        defects ++;
        }
      ostr << cmCTest::MakeXMLSafe(lines[cc]) << std::endl;
      }
    }
  //std::cout << "End test (elapsed: " << (cmSystemTools::GetTime() - sttime) << std::endl;
  log = ostr.str();
  if ( defects )
    {
    return false;
    }
  return true;
}

void cmCTestTestHandler::SetIncludeRegExp(const char *arg)
{
  m_IncludeRegExp = arg;
}

void cmCTestTestHandler::SetExcludeRegExp(const char *arg)
{
  m_ExcludeRegExp = arg;
}

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

