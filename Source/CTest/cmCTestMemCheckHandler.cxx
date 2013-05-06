/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCTestMemCheckHandler.h"
#include "cmXMLParser.h"
#include "cmCTest.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include <cmsys/Process.h>
#include <cmsys/RegularExpression.hxx>
#include <cmsys/Base64.h>
#include "cmMakefile.h"
#include "cmXMLSafe.h"

#include <stdlib.h>
#include <math.h>
#include <float.h>

struct CatToErrorType
{
  const char* ErrorCategory;
  int ErrorCode;
};


static CatToErrorType cmCTestMemCheckBoundsChecker[] = {
  // Error tags
  {"Write Overrun", cmCTestMemCheckHandler::ABW},
  {"Read Overrun",  cmCTestMemCheckHandler::ABR},
  {"Memory Overrun", cmCTestMemCheckHandler::ABW},
  {"Allocation Conflict",  cmCTestMemCheckHandler::FMM},
  {"Bad Pointer Use", cmCTestMemCheckHandler::FMW},
  {"Dangling Pointer", cmCTestMemCheckHandler::FMR},
  {0,0}
};

// parse the xml file storing the installed version of Xcode on
// the machine
class cmBoundsCheckerParser : public cmXMLParser
{
public:
  cmBoundsCheckerParser(cmCTest* c) { this->CTest = c;}
  void StartElement(const char* name, const char** atts)
    {
      if(strcmp(name, "MemoryLeak") == 0)
        {
        this->Errors.push_back(cmCTestMemCheckHandler::MLK);
        }
      if(strcmp(name, "ResourceLeak") == 0)
        {
        this->Errors.push_back(cmCTestMemCheckHandler::MLK);
        }
      if(strcmp(name, "Error") == 0)
        {
        this->ParseError(atts);
        }
      if(strcmp(name, "Dangling Pointer") == 0)
        {
        this->ParseError(atts);
        }
      // Create the log
      cmOStringStream ostr;
      ostr << name << ":\n";
      int i = 0;
      for(; atts[i] != 0; i+=2)
        {
        ostr << "   " << cmXMLSafe(atts[i])
             << " - " << cmXMLSafe(atts[i+1]) << "\n";
        }
      ostr << "\n";
      this->Log += ostr.str();
    }
  void EndElement(const char* )
    {
    }

  const char* GetAttribute(const char* name, const char** atts)
    {
      int i = 0;
      for(; atts[i] != 0; ++i)
        {
        if(strcmp(name, atts[i]) == 0)
          {
          return atts[i+1];
          }
        }
      return 0;
    }
  void ParseError(const char** atts)
    {
      CatToErrorType* ptr = cmCTestMemCheckBoundsChecker;
      const char* cat = this->GetAttribute("ErrorCategory", atts);
      if(!cat)
        {
        this->Errors.push_back(cmCTestMemCheckHandler::ABW); // do not know
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "No Category found in Bounds checker XML\n" );
        return;
        }
      while(ptr->ErrorCategory && cat)
        {
        if(strcmp(ptr->ErrorCategory, cat) == 0)
          {
          this->Errors.push_back(ptr->ErrorCode);
          return; // found it we are done
          }
        ptr++;
        }
      if(ptr->ErrorCategory)
        {
        this->Errors.push_back(cmCTestMemCheckHandler::ABW); // do not know
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "Found unknown Bounds Checker error "
                   << ptr->ErrorCategory << std::endl);
        }
    }
  cmCTest* CTest;
  std::vector<int> Errors;
  std::string Log;
};

#define BOUNDS_CHECKER_MARKER \
"******######*****Begin BOUNDS CHECKER XML******######******"
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
  this->CustomMaximumPassedTestOutputSize = 0;
  this->CustomMaximumFailedTestOutputSize = 0;
  this->MemoryTester = "";
  this->MemoryTesterOptions.clear();
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
  std::vector<std::string>& args)
{
  std::vector<cmStdString>::size_type pp;
  std::string memcheckcommand = "";
  memcheckcommand
    = cmSystemTools::ConvertToOutputPath(this->MemoryTester.c_str());
  for ( pp = 0; pp < this->MemoryTesterOptions.size(); pp ++ )
    {
    args.push_back(this->MemoryTesterOptions[pp]);
    memcheckcommand += " \"";
    memcheckcommand += this->MemoryTesterOptions[pp];
    memcheckcommand += "\"";
    }
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Memory check command: "
    << memcheckcommand << std::endl);
}

//----------------------------------------------------------------------
void cmCTestMemCheckHandler::PopulateCustomVectors(cmMakefile *mf)
{
  this->cmCTestTestHandler::PopulateCustomVectors(mf);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_MEMCHECK",
                                this->CustomPreMemCheck);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_POST_MEMCHECK",
                                this->CustomPostMemCheck);

  this->CTest->PopulateCustomVector(mf,
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

  this->CTest->StartXML(os, this->AppendXML);
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
     << "\t<StartTestTime>" << this->StartTestTime << "</StartTestTime>\n"
     << "\t<TestList>\n";
  cmCTestMemCheckHandler::TestResultsVector::size_type cc;
  for ( cc = 0; cc < this->TestResults.size(); cc ++ )
    {
    cmCTestTestResult *result = &this->TestResults[cc];
    std::string testPath = result->Path + "/" + result->Name;
    os << "\t\t<Test>" << cmXMLSafe(
      this->CTest->GetShortPathToFile(testPath.c_str()))
      << "</Test>" << std::endl;
    }
  os << "\t</TestList>\n";
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
    "-- Processing memory checking output: ");
  size_t total = this->TestResults.size();
  size_t step = total / 10;
  size_t current = 0;
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
    this->WriteTestResultHeader(os, result);
    os << "\t\t<Results>" << std::endl;
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

    std::string logTag;
    if(this->CTest->ShouldCompressMemCheckOutput())
      {
      this->CTest->CompressString(memcheckstr);
      logTag = "\t<Log compression=\"gzip\" encoding=\"base64\">\n";
      }
    else
      {
      logTag = "\t<Log>\n";
      }

    os
      << "\t\t</Results>\n"
      << logTag << cmXMLSafe(memcheckstr) << std::endl
      << "\t</Log>\n";
    this->WriteTestResultFooter(os, result);
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
  os << "\t<EndTestTime>" << this->EndTestTime
     << "</EndTestTime>" << std::endl;
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
      = this->CTest->GetCTestConfiguration("MemoryCheckCommand").c_str();

    // determine the checker type
    if ( this->MemoryTester.find("valgrind") != std::string::npos )
      {
        this->MemoryTesterStyle = cmCTestMemCheckHandler::VALGRIND;
      }
    else if ( this->MemoryTester.find("purify") != std::string::npos )
      {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::PURIFY;
      }
    else if ( this->MemoryTester.find("BC") != std::string::npos )
      {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::BOUNDS_CHECKER;
      }
    else
      {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::UNKNOWN;
      }
    }
  else if ( cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
        "PurifyCommand").c_str()) )
    {
    this->MemoryTester
      = this->CTest->GetCTestConfiguration("PurifyCommand").c_str();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::PURIFY;
    }
  else if ( cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
        "ValgrindCommand").c_str()) )
    {
    this->MemoryTester
      = this->CTest->GetCTestConfiguration("ValgrindCommand").c_str();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::VALGRIND;
    }
  else if ( cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
        "BoundsCheckerCommand").c_str()) )
    {
    this->MemoryTester
      = this->CTest->GetCTestConfiguration("BoundsCheckerCommand").c_str();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::BOUNDS_CHECKER;
    }
  else
    {
    cmCTestLog(this->CTest, WARNING,
      "Memory checker (MemoryCheckCommand) "
      "not set, or cannot find the specified program."
      << std::endl);
    return false;
    }

  // Setup the options
  std::string memoryTesterOptions;
  if ( this->CTest->GetCTestConfiguration(
      "MemoryCheckCommandOptions").size() )
    {
    memoryTesterOptions = this->CTest->GetCTestConfiguration(
      "MemoryCheckCommandOptions");
    }
  else if ( this->CTest->GetCTestConfiguration(
      "ValgrindCommandOptions").size() )
    {
    memoryTesterOptions = this->CTest->GetCTestConfiguration(
      "ValgrindCommandOptions");
    }
  this->MemoryTesterOptions
    = cmSystemTools::ParseArguments(memoryTesterOptions.c_str());

  this->MemoryTesterOutputFile
    = this->CTest->GetBinaryDir() + "/Testing/Temporary/MemoryChecker.log";

  switch ( this->MemoryTesterStyle )
    {
    case cmCTestMemCheckHandler::VALGRIND:
      {
      if ( this->MemoryTesterOptions.empty() )
        {
        this->MemoryTesterOptions.push_back("-q");
        this->MemoryTesterOptions.push_back("--tool=memcheck");
        this->MemoryTesterOptions.push_back("--leak-check=yes");
        this->MemoryTesterOptions.push_back("--show-reachable=yes");
        this->MemoryTesterOptions.push_back("--workaround-gcc296-bugs=yes");
        this->MemoryTesterOptions.push_back("--num-callers=50");
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
        std::string suppressions = "--suppressions="
          + this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile");
        this->MemoryTesterOptions.push_back(suppressions);
        }
      std::string outputFile = "--log-file="
        + this->MemoryTesterOutputFile;
      this->MemoryTesterOptions.push_back(outputFile);
      break;
      }
    case cmCTestMemCheckHandler::PURIFY:
      {
      std::string outputFile;
#ifdef _WIN32
      if( this->CTest->GetCTestConfiguration(
            "MemoryCheckSuppressionFile").size() )
        {
        if( !cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
                                       "MemoryCheckSuppressionFile").c_str()) )
          {
          cmCTestLog(this->CTest, ERROR_MESSAGE,
                     "Cannot find memory checker suppression file: "
                     << this->CTest->GetCTestConfiguration(
                       "MemoryCheckSuppressionFile").c_str() << std::endl);
          return false;
          }
        std::string filterFiles = "/FilterFiles="
          + this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile");
        this->MemoryTesterOptions.push_back(filterFiles);
        }
      outputFile = "/SAVETEXTDATA=";
#else
      outputFile = "-log-file=";
#endif
      outputFile += this->MemoryTesterOutputFile;
      this->MemoryTesterOptions.push_back(outputFile);
      break;
      }
    case cmCTestMemCheckHandler::BOUNDS_CHECKER:
      {
      this->BoundsCheckerXMLFile = this->MemoryTesterOutputFile;
      std::string dpbdFile = this->CTest->GetBinaryDir()
        + "/Testing/Temporary/MemoryChecker.DPbd";
      this->BoundsCheckerDPBDFile = dpbdFile;
      this->MemoryTesterOptions.push_back("/B");
      this->MemoryTesterOptions.push_back(dpbdFile);
      this->MemoryTesterOptions.push_back("/X");
      this->MemoryTesterOptions.push_back(this->MemoryTesterOutputFile);
      this->MemoryTesterOptions.push_back("/M");
      break;
      }
    default:
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Do not understand memory checker: " << this->MemoryTester.c_str()
        << std::endl);
      return false;
    }

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
    return this->ProcessMemCheckBoundsCheckerOutput(str, log, results);
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
  const std::string& str, std::string& log,
  int* results)
{
  std::vector<cmStdString> lines;
  cmSystemTools::Split(str.c_str(), lines);
  cmOStringStream ostr;
  log = "";

  cmsys::RegularExpression pfW("^\\[[WEI]\\] ([A-Z][A-Z][A-Z][A-Z]*): ");

  int defects = 0;

  for( std::vector<cmStdString>::iterator i = lines.begin();
       i != lines.end(); ++i)
    {
    int failure = cmCTestMemCheckHandler::NO_MEMORY_FAULT;
    if ( pfW.find(*i) )
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
    ostr << cmXMLSafe(*i) << std::endl;
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
  bool unlimitedOutput = false;
  if(str.find("CTEST_FULL_OUTPUT") != str.npos ||
    this->CustomMaximumFailedTestOutputSize == 0)
    {
    unlimitedOutput = true;
    }

  std::string::size_type cc;

  cmOStringStream ostr;
  log = "";

  int defects = 0;

  cmsys::RegularExpression valgrindLine("^==[0-9][0-9]*==");

  cmsys::RegularExpression vgFIM(
    "== .*Invalid free\\(\\) / delete / delete\\[\\]");
  cmsys::RegularExpression vgFMM(
    "== .*Mismatched free\\(\\) / delete / delete \\[\\]");
  cmsys::RegularExpression vgMLK1(
    "== .*[0-9,]+ bytes in [0-9,]+ blocks are definitely lost"
   " in loss record [0-9,]+ of [0-9,]+");
  cmsys::RegularExpression vgMLK2(
    "== .*[0-9,]+ \\([0-9,]+ direct, [0-9,]+ indirect\\)"
    " bytes in [0-9,]+ blocks are definitely lost"
    " in loss record [0-9,]+ of [0-9,]+");
  cmsys::RegularExpression vgPAR(
    "== .*Syscall param .* (contains|points to) unaddressable byte\\(s\\)");
  cmsys::RegularExpression vgMPK1(
    "== .*[0-9,]+ bytes in [0-9,]+ blocks are possibly lost in"
    " loss record [0-9,]+ of [0-9,]+");
  cmsys::RegularExpression vgMPK2(
    "== .*[0-9,]+ bytes in [0-9,]+ blocks are still reachable"
    " in loss record [0-9,]+ of [0-9,]+");
  cmsys::RegularExpression vgUMC(
    "== .*Conditional jump or move depends on uninitialised value\\(s\\)");
  cmsys::RegularExpression vgUMR1(
    "== .*Use of uninitialised value of size [0-9,]+");
  cmsys::RegularExpression vgUMR2("== .*Invalid read of size [0-9,]+");
  cmsys::RegularExpression vgUMR3("== .*Jump to the invalid address ");
  cmsys::RegularExpression vgUMR4("== .*Syscall param .* contains "
    "uninitialised or unaddressable byte\\(s\\)");
  cmsys::RegularExpression vgUMR5("== .*Syscall param .* uninitialised");
  cmsys::RegularExpression vgIPW("== .*Invalid write of size [0-9,]+");
  cmsys::RegularExpression vgABR("== .*pthread_mutex_unlock: mutex is "
    "locked by a different thread");
  std::vector<std::string::size_type> nonValGrindOutput;
  double sttime = cmSystemTools::GetTime();
  cmCTestLog(this->CTest, DEBUG, "Start test: " << lines.size() << std::endl);
  std::string::size_type totalOutputSize = 0;
  bool outputFull = false;
  for ( cc = 0; cc < lines.size(); cc ++ )
    {
    cmCTestLog(this->CTest, DEBUG, "test line "
               << lines[cc] << std::endl);

    if ( valgrindLine.find(lines[cc]) )
      {
      cmCTestLog(this->CTest, DEBUG, "valgrind  line "
                 << lines[cc] << std::endl);
      int failure = cmCTestMemCheckHandler::NO_MEMORY_FAULT;
      if ( vgFIM.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::FIM;
        }
      else if ( vgFMM.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::FMM;
        }
      else if ( vgMLK1.find(lines[cc]) )
        {
        failure = cmCTestMemCheckHandler::MLK;
        }
      else if ( vgMLK2.find(lines[cc]) )
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
      else if ( vgUMR5.find(lines[cc]) )
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
      totalOutputSize += lines[cc].size();
      ostr << cmXMLSafe(lines[cc]) << std::endl;
      }
    else
      {
      nonValGrindOutput.push_back(cc);
      }
    }
  // Now put all all the non valgrind output into the test output
  if(!outputFull)
    {
    for(std::vector<std::string::size_type>::iterator i =
          nonValGrindOutput.begin(); i != nonValGrindOutput.end(); ++i)
      {
      totalOutputSize += lines[*i].size();
      cmCTestLog(this->CTest, DEBUG, "before xml safe "
                 << lines[*i] << std::endl);
      cmCTestLog(this->CTest, DEBUG, "after  xml safe "
                 <<  cmXMLSafe(lines[*i]) << std::endl);

      ostr << cmXMLSafe(lines[*i]) << std::endl;
      if(!unlimitedOutput && totalOutputSize >
         static_cast<size_t>(this->CustomMaximumFailedTestOutputSize))
        {
        outputFull = true;
        ostr << "....\n";
        ostr << "Test Output for this test has been truncated see testing"
          " machine logs for full output,\n";
        ostr << "or put CTEST_FULL_OUTPUT in the output of "
          "this test program.\n";
        }
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



//----------------------------------------------------------------------
bool cmCTestMemCheckHandler::ProcessMemCheckBoundsCheckerOutput(
  const std::string& str, std::string& log,
  int* results)
{
  log = "";
  double sttime = cmSystemTools::GetTime();
  std::vector<cmStdString> lines;
  cmSystemTools::Split(str.c_str(), lines);
  cmCTestLog(this->CTest, DEBUG, "Start test: " << lines.size() << std::endl);
  std::vector<cmStdString>::size_type cc;
  for ( cc = 0; cc < lines.size(); cc ++ )
    {
    if(lines[cc] == BOUNDS_CHECKER_MARKER)
      {
      break;
      }
    }
  cmBoundsCheckerParser parser(this->CTest);
  parser.InitializeParser();
  if(cc < lines.size())
    {
    for(cc++; cc < lines.size(); ++cc)
      {
      std::string& theLine = lines[cc];
      // check for command line arguments that are not escaped
      // correctly by BC
      if(theLine.find("TargetArgs=") != theLine.npos)
        {
        // skip this because BC gets it wrong and we can't parse it
        }
      else if(!parser.ParseChunk(theLine.c_str(), theLine.size()))
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "Error in ParseChunk: " << theLine.c_str()
                   << std::endl);
        }
      }
    }
  int defects = 0;
  for(cc =0; cc < parser.Errors.size(); ++cc)
    {
    results[parser.Errors[cc]]++;
    defects++;
    }
  cmCTestLog(this->CTest, DEBUG, "End test (elapsed: "
    << (cmSystemTools::GetTime() - sttime) << std::endl);
  if(defects)
    {
    // only put the output of Bounds Checker if there were
    // errors or leaks detected
    log = parser.Log;
    return false;
    }
  return true;
}

// This method puts the bounds checker output file into the output
// for the test
void
cmCTestMemCheckHandler::PostProcessBoundsCheckerTest(cmCTestTestResult& res)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "PostProcessBoundsCheckerTest for : "
             << res.Name.c_str() << std::endl);
  if ( !cmSystemTools::FileExists(this->MemoryTesterOutputFile.c_str()) )
    {
    std::string log = "Cannot find memory tester output file: "
      + this->MemoryTesterOutputFile;
    cmCTestLog(this->CTest, ERROR_MESSAGE, log.c_str() << std::endl);
    return;
    }
  // put a scope around this to close ifs so the file can be removed
  {
  std::ifstream ifs(this->MemoryTesterOutputFile.c_str());
  if ( !ifs )
    {
    std::string log = "Cannot read memory tester output file: "
      + this->MemoryTesterOutputFile;
    cmCTestLog(this->CTest, ERROR_MESSAGE, log.c_str() << std::endl);
    return;
    }
  res.Output += BOUNDS_CHECKER_MARKER;
  res.Output += "\n";
  std::string line;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    res.Output += line;
    res.Output += "\n";
    }
  }
  cmSystemTools::Delay(1000);
  cmSystemTools::RemoveFile(this->BoundsCheckerDPBDFile.c_str());
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Remove: "
    << this->BoundsCheckerDPBDFile.c_str() << std::endl);
  cmSystemTools::RemoveFile(this->BoundsCheckerXMLFile.c_str());
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Remove: "
    << this->BoundsCheckerXMLFile.c_str() << std::endl);
}

void
cmCTestMemCheckHandler::PostProcessPurifyTest(cmCTestTestResult& res)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "PostProcessPurifyTest for : "
             << res.Name.c_str() << std::endl);
  appendMemTesterOutput(res);
}

void
cmCTestMemCheckHandler::PostProcessValgrindTest(cmCTestTestResult& res)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "PostProcessValgrindTest for : "
             << res.Name.c_str() << std::endl);
  appendMemTesterOutput(res);
}

void
cmCTestMemCheckHandler::appendMemTesterOutput(cmCTestTestResult& res)
{
  if ( !cmSystemTools::FileExists(this->MemoryTesterOutputFile.c_str()) )
    {
    std::string log = "Cannot find memory tester output file: "
      + this->MemoryTesterOutputFile;
    cmCTestLog(this->CTest, ERROR_MESSAGE, log.c_str() << std::endl);
    return;
    }
  std::ifstream ifs(this->MemoryTesterOutputFile.c_str());
  if ( !ifs )
    {
    std::string log = "Cannot read memory tester output file: "
      + this->MemoryTesterOutputFile;
    cmCTestLog(this->CTest, ERROR_MESSAGE, log.c_str() << std::endl);
    return;
    }
  std::string line;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    res.Output += line;
    res.Output += "\n";
    }
}
