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

#include "cmCTestMemCheckHandler.h"

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

//----------------------------------------------------------------------
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


//----------------------------------------------------------------------
cmCTestMemCheckHandler::cmCTestMemCheckHandler()
{
  m_MemCheck = true;
}

//----------------------------------------------------------------------
int cmCTestMemCheckHandler::PreProcessHandler()
{
  if ( !this->InitializeMemoryChecking() )
    {
    return 0;
    }

  if ( !this->ExecuteCommands(m_CustomPreMemCheck) )
    {
    std::cerr << "Problem executing pre-memcheck command(s)." << std::endl;
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCTestMemCheckHandler::PostProcessHandler()
{
  if ( !this->ExecuteCommands(m_CustomPostMemCheck) )
    {
    std::cerr << "Problem executing post-memcheck command(s)." << std::endl;
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::GenerateTestCommand(std::vector<const char*>& args)
{
  std::vector<cmStdString>::size_type pp;
  args.push_back(m_MemoryTester.c_str());
  std::string memcheckcommand = "";
  memcheckcommand = m_MemoryTester;
  for ( pp = 0; pp < m_MemoryTesterOptionsParsed.size(); pp ++ )
    {
    args.push_back(m_MemoryTesterOptionsParsed[pp].c_str());
    memcheckcommand += " ";
    memcheckcommand += cmSystemTools::EscapeSpaces(m_MemoryTesterOptionsParsed[pp].c_str());
    }
  if ( m_Verbose )
    {
    std::cout << "Memory check command: " << memcheckcommand << std::endl;
    }
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::PopulateCustomVectors(cmMakefile *mf)
{
  this->cmCTestTestHandler::PopulateCustomVectors(mf);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_MEMCHECK", 
                                m_CustomPreMemCheck);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_POST_MEMCHECK", 
                                m_CustomPostMemCheck);

  cmCTest::PopulateCustomVector(mf, 
                             "CTEST_CUSTOM_MEMCHECK_IGNORE", 
                             m_CustomTestsIgnore);
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::GenerateDartOutput(std::ostream& os)
{
  if ( !m_CTest->GetProduceXML() )
    {
    return;
    }

  m_CTest->StartXML(os);
  os << "<DynamicAnalysis Checker=\"";
  switch ( m_MemoryTesterStyle )
    {
    case cmCTestMemCheckHandler::VALGRIND:
      os << "Valgrind";
      break;
    case cmCTestMemCheckHandler::PURIFY:
      os << "Purify";
      break;
    case cmCTestMemCheckHandler::BOUNDS_CHECKER:
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
    int memcheckresults[cmCTestMemCheckHandler::NO_MEMORY_FAULT];
    int kk;
    bool res = this->ProcessMemCheckOutput(result->m_Output, memcheckstr, memcheckresults);
    if ( res && result->m_Status == cmCTestMemCheckHandler::COMPLETED )
      {
      continue;
      }
    os << "\t<Test Status=\"";
    if ( result->m_Status == cmCTestMemCheckHandler::COMPLETED )
      {
      os << "passed";
      }
    else if ( result->m_Status == cmCTestMemCheckHandler::NOT_RUN )
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

//----------------------------------------------------------------------
bool cmCTestMemCheckHandler::InitializeMemoryChecking()
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
    m_MemoryTesterStyle = cmCTestMemCheckHandler::VALGRIND;
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
    m_MemoryTesterStyle = cmCTestMemCheckHandler::PURIFY;
#ifdef _WIN32
    m_MemoryTesterOptions += " /SAVETEXTDATA=" + m_MemoryTesterOutputFile;
#else
    m_MemoryTesterOptions += " -log-file=" + m_MemoryTesterOutputFile;
#endif
    }
  else if ( m_MemoryTester.find("boundschecker") != std::string::npos )
    {
    m_MemoryTesterStyle = cmCTestMemCheckHandler::BOUNDS_CHECKER;
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

//----------------------------------------------------------------------
bool cmCTestMemCheckHandler::ProcessMemCheckOutput(const std::string& str, 
                                               std::string& log, int* results)
{
  std::string::size_type cc;
  for ( cc = 0; cc < cmCTestMemCheckHandler::NO_MEMORY_FAULT; cc ++ )
    {
    results[cc] = 0;
    }

  if ( m_MemoryTesterStyle == cmCTestMemCheckHandler::VALGRIND )
    {
    return this->ProcessMemCheckValgrindOutput(str, log, results);
    }
  else if ( m_MemoryTesterStyle == cmCTestMemCheckHandler::PURIFY )
    {
    return this->ProcessMemCheckPurifyOutput(str, log, results);
    }
  else if ( m_MemoryTesterStyle == cmCTestMemCheckHandler::BOUNDS_CHECKER )
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

//----------------------------------------------------------------------
bool cmCTestMemCheckHandler::ProcessMemCheckPurifyOutput(
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
    int failure = cmCTestMemCheckHandler::NO_MEMORY_FAULT;
    if ( pfW.find(line) )
      {
      int cc;
      for ( cc = 0; cc < cmCTestMemCheckHandler::NO_MEMORY_FAULT; cc ++ )
        {
        if ( pfW.match(1) == cmCTestMemCheckResultStrings[cc] )
          {
          failure = cc;
          break;
          }
        }
      if ( cc == cmCTestMemCheckHandler::NO_MEMORY_FAULT )
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

//----------------------------------------------------------------------
bool cmCTestMemCheckHandler::ProcessMemCheckValgrindOutput(
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
      int failure = cmCTestMemCheckHandler::NO_MEMORY_FAULT;
      if ( vgFIM.find(lines[cc]) ) { failure = cmCTestMemCheckHandler::FIM; }
      else if ( vgFMM.find(lines[cc]) ) { failure = cmCTestMemCheckHandler::FMM; }
      else if ( vgMLK.find(lines[cc]) ) { failure = cmCTestMemCheckHandler::MLK; }
      else if ( vgPAR.find(lines[cc]) ) { failure = cmCTestMemCheckHandler::PAR; }
      else if ( vgMPK1.find(lines[cc]) ){ failure = cmCTestMemCheckHandler::MPK; }
      else if ( vgMPK2.find(lines[cc]) ){ failure = cmCTestMemCheckHandler::MPK; }
      else if ( vgUMC.find(lines[cc]) ) { failure = cmCTestMemCheckHandler::UMC; }
      else if ( vgUMR1.find(lines[cc]) ){ failure = cmCTestMemCheckHandler::UMR; }
      else if ( vgUMR2.find(lines[cc]) ){ failure = cmCTestMemCheckHandler::UMR; }
      else if ( vgUMR3.find(lines[cc]) ){ failure = cmCTestMemCheckHandler::UMR; }
      else if ( vgUMR4.find(lines[cc]) ){ failure = cmCTestMemCheckHandler::UMR; }
      else if ( vgIPW.find(lines[cc]) ) { failure = cmCTestMemCheckHandler::IPW; }
      else if ( vgABR.find(lines[cc]) ) { failure = cmCTestMemCheckHandler::ABR; }

      if ( failure != cmCTestMemCheckHandler::NO_MEMORY_FAULT )
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
