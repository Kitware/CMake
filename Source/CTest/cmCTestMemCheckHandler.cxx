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
  this->MemCheck = true;
  this->CustomMaximumPassedTestOutputSize = 0;
  this->CustomMaximumFailedTestOutputSize = 0;
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::Initialize()
{
  this->Superclass::Initialize();
  this->MemoryTester = "";
  this->MemoryTesterOptionsParsed.clear();
  this->MemoryTesterOptions = "";
  this->MemoryTesterStyle = UNKNOWN;
  this->MemoryTesterOutputFile = "";
  int cc;
  for ( cc = 0; cc < NO_MEMORY_FAULT; cc ++ )
    {
    this->MemoryTesterGlobalResults[cc] = 0;
    }

}

//----------------------------------------------------------------------
int cmCTestMemCheckHandler::PreProcessHandler()
{
  if ( !this->InitializeMemoryChecking() )
    {
    return 0;
    }

  if ( !this->ExecuteCommands(this->CustomPreMemCheck) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Problem executing pre-memcheck command(s)." << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCTestMemCheckHandler::PostProcessHandler()
{
  if ( !this->ExecuteCommands(this->CustomPostMemCheck) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Problem executing post-memcheck command(s)." << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::GenerateTestCommand(
  std::vector<const char*>& args)
{
  std::vector<cmStdString>::size_type pp;
  args.push_back(this->MemoryTester.c_str());
  std::string memcheckcommand = "";
  memcheckcommand = this->MemoryTester;
  for ( pp = 0; pp < this->MemoryTesterOptionsParsed.size(); pp ++ )
    {
    args.push_back(this->MemoryTesterOptionsParsed[pp].c_str());
    memcheckcommand += " ";
    memcheckcommand += cmSystemTools::EscapeSpaces(
      this->MemoryTesterOptionsParsed[pp].c_str());
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Memory check command: "
    << memcheckcommand << std::endl);
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::PopulateCustomVectors(cmMakefile *mf)
{
  this->cmCTestTestHandler::PopulateCustomVectors(mf);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_MEMCHECK",
                                this->CustomPreMemCheck);
  cmCTest::PopulateCustomVector(mf, "CTEST_CUSTOM_POST_MEMCHECK",
                                this->CustomPostMemCheck);

  cmCTest::PopulateCustomVector(mf,
                             "CTEST_CUSTOM_MEMCHECK_IGNORE",
                             this->CustomTestsIgnore);
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::GenerateDartOutput(std::ostream& os)
{
  if ( !this->CTest->GetProduceXML() )
    {
    return;
    }

  this->CTest->StartXML(os);
  os << "<DynamicAnalysis Checker=\"";
  switch ( this->MemoryTesterStyle )
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

  os << "\t<StartDateTime>" << this->StartTest << "</StartDateTime>\n"
    << "\t<TestList>\n";
  cmCTestMemCheckHandler::TestResultsVector::size_type cc;
  for ( cc = 0; cc < this->TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &this->TestResults[cc];
    std::string testPath = result->Path + "/" + result->Name;
    os << "\t\t<Test>" << cmCTest::MakeXMLSafe(
      this->CTest->GetShortPathToFile(testPath.c_str()))
      << "</Test>" << std::endl;
    }
  os << "\t</TestList>\n";
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
    "-- Processing memory checking output: ");
  unsigned int total = this->TestResults.size();
  unsigned int step = total / 10;
  unsigned int current = 0;
  for ( cc = 0; cc < this->TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &this->TestResults[cc];
    std::string memcheckstr;
    int memcheckresults[cmCTestMemCheckHandler::NO_MEMORY_FAULT];
    int kk;
    bool res = this->ProcessMemCheckOutput(result->Output, memcheckstr,
      memcheckresults);
    if ( res && result->Status == cmCTestMemCheckHandler::COMPLETED )
      {
      continue;
      }
    this->CleanTestOutput(memcheckstr,
      static_cast<size_t>(this->CustomMaximumFailedTestOutputSize));
    os << "\t<Test Status=\"";
    if ( result->Status == cmCTestMemCheckHandler::COMPLETED )
      {
      os << "passed";
      }
    else if ( result->Status == cmCTestMemCheckHandler::NOT_RUN )
      {
      os << "notrun";
      }
    else
      {
      os << "failed";
      }
    std::string testPath = result->Path + "/" + result->Name;
    os << "\">\n"
      << "\t\t<Name>" << cmCTest::MakeXMLSafe(result->Name) << "</Name>\n"
      << "\t\t<Path>" << cmCTest::MakeXMLSafe(
        this->CTest->GetShortPathToFile(result->Path.c_str())) << "</Path>\n"
      << "\t\t<FullName>" << cmCTest::MakeXMLSafe(
        this->CTest->GetShortPathToFile(testPath.c_str())) << "</FullName>\n"
      << "\t\t<FullCommandLine>"
      << cmCTest::MakeXMLSafe(result->FullCommandLine)
      << "</FullCommandLine>\n"
      << "\t\t<Results>" << std::endl;
    for ( kk = 0; cmCTestMemCheckResultLongStrings[kk]; kk ++ )
      {
      if ( memcheckresults[kk] )
        {
        os << "\t\t\t<Defect type=\"" << cmCTestMemCheckResultLongStrings[kk]
          << "\">"
           << memcheckresults[kk]
           << "</Defect>" << std::endl;
        }
      this->MemoryTesterGlobalResults[kk] += memcheckresults[kk];
      }
    os
      << "\t\t</Results>\n"
      << "\t<Log>\n" << memcheckstr << std::endl
      << "\t</Log>\n"
      << "\t</Test>" << std::endl;
    if ( current < cc )
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "#" << std::flush);
      current += step;
      }
    }
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::endl);
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "Memory checking results:"
    << std::endl);
  os << "\t<DefectList>" << std::endl;
  for ( cc = 0; cmCTestMemCheckResultStrings[cc]; cc ++ )
    {
    if ( this->MemoryTesterGlobalResults[cc] )
      {
#ifdef cerr
#  undef cerr
#endif
      std::cerr.width(35);
#define cerr no_cerr
      cmCTestLog(this->CTest, HANDLER_OUTPUT,
        cmCTestMemCheckResultLongStrings[cc] << " - "
        << this->MemoryTesterGlobalResults[cc] << std::endl);
      os << "\t\t<Defect Type=\"" << cmCTestMemCheckResultLongStrings[cc]
        << "\"/>" << std::endl;
      }
    }
  os << "\t</DefectList>" << std::endl;

  os << "\t<EndDateTime>" << this->EndTest << "</EndDateTime>" << std::endl;
  os << "<ElapsedMinutes>"
     << static_cast<int>(this->ElapsedTestingTime/6)/10.0
     << "</ElapsedMinutes>\n";

  os << "</DynamicAnalysis>" << std::endl;
  this->CTest->EndXML(os);


}

//----------------------------------------------------------------------
bool cmCTestMemCheckHandler::InitializeMemoryChecking()
{
  // Setup the command
  if ( cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
        "MemoryCheckCommand").c_str()) )
    {
    this->MemoryTester
      = cmSystemTools::ConvertToOutputPath(this->CTest->GetCTestConfiguration(
          "MemoryCheckCommand").c_str());
    }
  else if ( cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
        "PurifyCommand").c_str()) )
    {
    this->MemoryTester
      = cmSystemTools::ConvertToOutputPath(this->CTest->GetCTestConfiguration(
          "PurifyCommand").c_str());
    }
  else if ( cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
        "ValgrindCommand").c_str()) )
    {
    this->MemoryTester
      = cmSystemTools::ConvertToOutputPath(this->CTest->GetCTestConfiguration(
          "ValgrindCommand").c_str());
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Memory checker (MemoryCheckCommand) "
      "not set, or cannot find the specified program."
      << std::endl);
    return false;
    }

  if ( this->MemoryTester[0] == '\"' &&
    this->MemoryTester[this->MemoryTester.size()-1] == '\"' )
    {
    this->MemoryTester
      = this->MemoryTester.substr(1, this->MemoryTester.size()-2);
    }

  // Setup the options
  if ( this->CTest->GetCTestConfiguration(
      "MemoryCheckCommandOptions").size() )
    {
    this->MemoryTesterOptions = this->CTest->GetCTestConfiguration(
      "MemoryCheckCommandOptions");
    }
  else if ( this->CTest->GetCTestConfiguration(
      "ValgrindCommandOptions").size() )
    {
    this->MemoryTesterOptions = this->CTest->GetCTestConfiguration(
      "ValgrindCommandOptions");
    }

  this->MemoryTesterOutputFile
    = this->CTest->GetBinaryDir() + "/Testing/Temporary/MemoryChecker.log";
  this->MemoryTesterOutputFile
    = cmSystemTools::EscapeSpaces(this->MemoryTesterOutputFile.c_str());

  if ( this->MemoryTester.find("valgrind") != std::string::npos )
    {
    this->MemoryTesterStyle = cmCTestMemCheckHandler::VALGRIND;
    if ( !this->MemoryTesterOptions.size() )
      {
      this->MemoryTesterOptions = "-q --tool=memcheck --leak-check=yes "
        "--show-reachable=yes --workaround-gcc296-bugs=yes --num-callers=100";
      }
    if ( this->CTest->GetCTestConfiguration(
        "MemoryCheckSuppressionFile").size() )
      {
      if ( !cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
            "MemoryCheckSuppressionFile").c_str()) )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "Cannot find memory checker suppression file: "
          << this->CTest->GetCTestConfiguration(
            "MemoryCheckSuppressionFile").c_str() << std::endl);
        return false;
        }
      this->MemoryTesterOptions += " --suppressions=" +
        cmSystemTools::EscapeSpaces(this->CTest->GetCTestConfiguration(
            "MemoryCheckSuppressionFile").c_str()) + "";
      }
    }
  else if ( this->MemoryTester.find("purify") != std::string::npos )
    {
    this->MemoryTesterStyle = cmCTestMemCheckHandler::PURIFY;
#ifdef _WIN32
    this->MemoryTesterOptions += " /SAVETEXTDATA=" +
      this->MemoryTesterOutputFile;
#else
    this->MemoryTesterOptions += " -log-file=" + this->MemoryTesterOutputFile;
#endif
    }
  else if ( this->MemoryTester.find("boundschecker") != std::string::npos )
    {
    this->MemoryTesterStyle = cmCTestMemCheckHandler::BOUNDS_CHECKER;
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Bounds checker not yet implemented" << std::endl);
    return false;
    }
  else
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Do not understand memory checker: " << this->MemoryTester.c_str()
      << std::endl);
    return false;
    }

  this->MemoryTesterOptionsParsed
    = cmSystemTools::ParseArguments(this->MemoryTesterOptions.c_str());
  std::vector<cmStdString>::size_type cc;
  for ( cc = 0; cmCTestMemCheckResultStrings[cc]; cc ++ )
    {
    this->MemoryTesterGlobalResults[cc] = 0;
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

  if ( this->MemoryTesterStyle == cmCTestMemCheckHandler::VALGRIND )
    {
    return this->ProcessMemCheckValgrindOutput(str, log, results);
    }
  else if ( this->MemoryTesterStyle == cmCTestMemCheckHandler::PURIFY )
    {
    return this->ProcessMemCheckPurifyOutput(str, log, results);
    }
  else if ( this->MemoryTesterStyle ==
    cmCTestMemCheckHandler::BOUNDS_CHECKER )
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
  if ( !cmSystemTools::FileExists(this->MemoryTesterOutputFile.c_str()) )
    {
    log = "Cannot find Purify output file: " + this->MemoryTesterOutputFile;
    cmCTestLog(this->CTest, ERROR_MESSAGE, log.c_str() << std::endl);
    return false;
    }

  std::ifstream ifs(this->MemoryTesterOutputFile.c_str());
  if ( !ifs )
    {
    log = "Cannot read Purify output file: " + this->MemoryTesterOutputFile;
    cmCTestLog(this->CTest, ERROR_MESSAGE, log.c_str() << std::endl);
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
        cmCTestLog(this->CTest, ERROR_MESSAGE, "Unknown Purify memory fault: "
          << pfW.match(1) << std::endl);
        ostr << "*** Unknown Purify memory fault: " << pfW.match(1)
          << std::endl;
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
  cmsys::RegularExpression vgUMR1(
    "== .*Use of uninitialised value of size [0-9][0-9]*");
  cmsys::RegularExpression vgUMR2("== .*Invalid read of size [0-9][0-9]*");
  cmsys::RegularExpression vgUMR3("== .*Jump to the invalid address ");
  cmsys::RegularExpression vgUMR4("== .*Syscall param .* contains "
    "uninitialised or unaddressable byte\\(s\\)");
  cmsys::RegularExpression vgIPW("== .*Invalid write of size [0-9]");
  cmsys::RegularExpression vgABR("== .*pthread_mutex_unlock: mutex is "
    "locked by a different thread");

  double sttime = cmSystemTools::GetTime();
  cmCTestLog(this->CTest, DEBUG, "Start test: " << lines.size() << std::endl);
  for ( cc = 0; cc < lines.size(); cc ++ )
    {
    if ( valgrindLine.find(lines[cc]) )
      {
      int failure = cmCTestMemCheckHandler::NO_MEMORY_FAULT;
      if ( vgFIM.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::FIM;
        }
      else if ( vgFMM.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::FMM;
        }
      else if ( vgMLK.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::MLK;
        }
      else if ( vgPAR.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::PAR;
        }
      else if ( vgMPK1.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::MPK;
        }
      else if ( vgMPK2.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::MPK;
        }
      else if ( vgUMC.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::UMC;
        }
      else if ( vgUMR1.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::UMR;
        }
      else if ( vgUMR2.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::UMR;
        }
      else if ( vgUMR3.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::UMR;
        }
      else if ( vgUMR4.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::UMR;
        }
      else if ( vgIPW.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::IPW;
        }
      else if ( vgABR.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::ABR;
        }

      if ( failure != cmCTestMemCheckHandler::NO_MEMORY_FAULT )
        {
        ostr << "<b>" << cmCTestMemCheckResultStrings[failure] << "</b> ";
        results[failure] ++;
        defects ++;
        }
      ostr << cmCTest::MakeXMLSafe(lines[cc]) << std::endl;
      }
    }
  cmCTestLog(this->CTest, DEBUG, "End test (elapsed: "
    << (cmSystemTools::GetTime() - sttime) << std::endl);
  log = ostr.str();
  if ( defects )
    {
    return false;
    }
  return true;
}
