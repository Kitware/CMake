/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestMemCheckHandler.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>
#include <utility>

#include <cmext/algorithm>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmCTest.h"
#include "cmDuration.h"
#include "cmSystemTools.h"
#include "cmXMLParser.h"
#include "cmXMLWriter.h"

struct CatToErrorType
{
  const char* ErrorCategory;
  int ErrorCode;
};

static CatToErrorType cmCTestMemCheckBoundsChecker[] = {
  // Error tags
  { "Write Overrun", cmCTestMemCheckHandler::ABW },
  { "Read Overrun", cmCTestMemCheckHandler::ABR },
  { "Memory Overrun", cmCTestMemCheckHandler::ABW },
  { "Allocation Conflict", cmCTestMemCheckHandler::FMM },
  { "Bad Pointer Use", cmCTestMemCheckHandler::FMW },
  { "Dangling Pointer", cmCTestMemCheckHandler::FMR },
  { nullptr, 0 }
};

static void xmlReportError(int line, const char* msg, void* data)
{
  cmCTest* ctest = static_cast<cmCTest*>(data);
  cmCTestLog(ctest, ERROR_MESSAGE,
             "Error parsing XML in stream at line " << line << ": " << msg
                                                    << std::endl);
}

// parse the xml file containing the results of last BoundsChecker run
class cmBoundsCheckerParser : public cmXMLParser
{
public:
  cmBoundsCheckerParser(cmCTest* c)
  {
    this->CTest = c;
    this->SetErrorCallback(xmlReportError, c);
  }
  void StartElement(const std::string& name, const char** atts) override
  {
    if (name == "MemoryLeak" || name == "ResourceLeak") {
      this->Errors.push_back(cmCTestMemCheckHandler::MLK);
    } else if (name == "Error" || name == "Dangling Pointer") {
      this->ParseError(atts);
    }
    // Create the log
    std::ostringstream ostr;
    ostr << name << ":\n";
    int i = 0;
    for (; atts[i] != nullptr; i += 2) {
      ostr << "   " << atts[i] << " - " << atts[i + 1] << "\n";
    }
    ostr << "\n";
    this->Log += ostr.str();
  }
  void EndElement(const std::string& /*name*/) override {}

  const char* GetAttribute(const char* name, const char** atts)
  {
    int i = 0;
    for (; atts[i] != nullptr; ++i) {
      if (strcmp(name, atts[i]) == 0) {
        return atts[i + 1];
      }
    }
    return nullptr;
  }
  void ParseError(const char** atts)
  {
    CatToErrorType* ptr = cmCTestMemCheckBoundsChecker;
    const char* cat = this->GetAttribute("ErrorCategory", atts);
    if (!cat) {
      this->Errors.push_back(cmCTestMemCheckHandler::ABW); // do not know
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "No Category found in Bounds checker XML\n");
      return;
    }
    while (ptr->ErrorCategory && cat) {
      if (strcmp(ptr->ErrorCategory, cat) == 0) {
        this->Errors.push_back(ptr->ErrorCode);
        return; // found it we are done
      }
      ptr++;
    }
    if (ptr->ErrorCategory) {
      this->Errors.push_back(cmCTestMemCheckHandler::ABW); // do not know
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Found unknown Bounds Checker error " << ptr->ErrorCategory
                                                       << std::endl);
    }
  }
  cmCTest* CTest;
  std::vector<int> Errors;
  std::string Log;
};

#define BOUNDS_CHECKER_MARKER                                                 \
  "******######*****Begin BOUNDS CHECKER XML******######******"

cmCTestMemCheckHandler::cmCTestMemCheckHandler()
{
  this->MemCheck = true;
  this->CustomMaximumPassedTestOutputSize = 0;
  this->CustomMaximumFailedTestOutputSize = 0;
  this->LogWithPID = false;
}

void cmCTestMemCheckHandler::Initialize()
{
  this->Superclass::Initialize();
  this->LogWithPID = false;
  this->CustomMaximumPassedTestOutputSize = 0;
  this->CustomMaximumFailedTestOutputSize = 0;
  this->MemoryTester.clear();
  this->MemoryTesterDynamicOptions.clear();
  this->MemoryTesterOptions.clear();
  this->MemoryTesterStyle = UNKNOWN;
  this->MemoryTesterOutputFile.clear();
  this->DefectCount = 0;
}

int cmCTestMemCheckHandler::PreProcessHandler()
{
  if (!this->InitializeMemoryChecking()) {
    return 0;
  }

  if (!this->ExecuteCommands(this->CustomPreMemCheck)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Problem executing pre-memcheck command(s)." << std::endl);
    return 0;
  }
  return 1;
}

int cmCTestMemCheckHandler::PostProcessHandler()
{
  if (!this->ExecuteCommands(this->CustomPostMemCheck)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Problem executing post-memcheck command(s)." << std::endl);
    return 0;
  }
  return 1;
}

void cmCTestMemCheckHandler::GenerateTestCommand(
  std::vector<std::string>& args, int test)
{
  std::string index = std::to_string(test);
  std::string memcheckcommand =
    cmSystemTools::ConvertToOutputPath(this->MemoryTester);

  std::vector<std::string> dirs;
  bool nextArgIsDir = false;

  for (std::string arg : this->MemoryTesterDynamicOptions) {
    std::string::size_type pos = arg.find("??");
    if (pos != std::string::npos) {
      arg.replace(pos, 2, index);
    }
    args.push_back(arg);
    memcheckcommand += " \"";
    memcheckcommand += arg;
    memcheckcommand += "\"";

    if (nextArgIsDir) {
      nextArgIsDir = false;
      dirs.push_back(arg);
    }

    if (this->MemoryTesterStyle == cmCTestMemCheckHandler::DRMEMORY &&
        (arg == "-logdir" || arg == "-symcache_dir")) {
      nextArgIsDir = true;
    }
  }
  // Create a copy of the memory tester environment variable.
  // This is used for memory testing programs that pass options
  // via environment variables.
  std::string memTesterEnvironmentVariable =
    this->MemoryTesterEnvironmentVariable;
  for (std::string const& arg : this->MemoryTesterOptions) {
    if (!memTesterEnvironmentVariable.empty()) {
      // If we are using env to pass options, append all the options to
      // this string with space separation.
      memTesterEnvironmentVariable += " " + arg;
    }
    // for regular options just add them to args and memcheckcommand
    // which is just used for display
    else {
      args.push_back(arg);
      memcheckcommand += " \"";
      memcheckcommand += arg;
      memcheckcommand += "\"";
    }
  }
  // if this is an env option type, then add the env string as a single
  // argument.
  if (!memTesterEnvironmentVariable.empty()) {
    std::string::size_type pos = memTesterEnvironmentVariable.find("??");
    if (pos != std::string::npos) {
      memTesterEnvironmentVariable.replace(pos, 2, index);
    }
    memcheckcommand += " " + memTesterEnvironmentVariable;
    args.push_back(memTesterEnvironmentVariable);
  }

  for (std::string const& dir : dirs) {
    cmSystemTools::MakeDirectory(dir);
  }

  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Memory check command: " << memcheckcommand << std::endl,
                     this->Quiet);
}

void cmCTestMemCheckHandler::InitializeResultsVectors()
{
  // fill these members
  //  cmsys::vector<std::string> ResultStrings;
  //  cmsys::vector<std::string> ResultStringsLong;
  //  cmsys::vector<int>         GlobalResults;
  this->ResultStringsLong.clear();
  this->ResultStrings.clear();
  this->GlobalResults.clear();
  // If we are working with style checkers that dynamically fill
  // the results strings then return.
  if (this->MemoryTesterStyle > cmCTestMemCheckHandler::BOUNDS_CHECKER) {
    return;
  }

  // define the standard set of errors
  //----------------------------------------------------------------------
  static const char* cmCTestMemCheckResultStrings[] = {
    "ABR", "ABW", "ABWL", "COR", "EXU", "FFM", "FIM",  "FMM",
    "FMR", "FMW", "FUM",  "IPR", "IPW", "MAF", "MLK",  "MPK",
    "NPR", "ODS", "PAR",  "PLK", "UMC", "UMR", nullptr
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
    nullptr
  };
  this->GlobalResults.clear();
  for (int i = 0; cmCTestMemCheckResultStrings[i] != nullptr; ++i) {
    this->ResultStrings.emplace_back(cmCTestMemCheckResultStrings[i]);
    this->ResultStringsLong.emplace_back(cmCTestMemCheckResultLongStrings[i]);
    this->GlobalResults.push_back(0);
  }
}

void cmCTestMemCheckHandler::PopulateCustomVectors(cmMakefile* mf)
{
  this->cmCTestTestHandler::PopulateCustomVectors(mf);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_PRE_MEMCHECK",
                                    this->CustomPreMemCheck);
  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_POST_MEMCHECK",
                                    this->CustomPostMemCheck);

  this->CTest->PopulateCustomVector(mf, "CTEST_CUSTOM_MEMCHECK_IGNORE",
                                    this->CustomTestsIgnore);
}

int cmCTestMemCheckHandler::GetDefectCount() const
{
  return this->DefectCount;
}

void cmCTestMemCheckHandler::GenerateCTestXML(cmXMLWriter& xml)
{
  if (!this->CTest->GetProduceXML()) {
    return;
  }
  this->CTest->StartXML(xml, this->AppendXML);
  this->CTest->GenerateSubprojectsOutput(xml);
  xml.StartElement("DynamicAnalysis");
  switch (this->MemoryTesterStyle) {
    case cmCTestMemCheckHandler::VALGRIND:
      xml.Attribute("Checker", "Valgrind");
      break;
    case cmCTestMemCheckHandler::DRMEMORY:
      xml.Attribute("Checker", "DrMemory");
      break;
    case cmCTestMemCheckHandler::PURIFY:
      xml.Attribute("Checker", "Purify");
      break;
    case cmCTestMemCheckHandler::BOUNDS_CHECKER:
      xml.Attribute("Checker", "BoundsChecker");
      break;
    case cmCTestMemCheckHandler::CUDA_SANITIZER:
      xml.Attribute("Checker", "CudaSanitizer");
      break;
    case cmCTestMemCheckHandler::ADDRESS_SANITIZER:
      xml.Attribute("Checker", "AddressSanitizer");
      break;
    case cmCTestMemCheckHandler::LEAK_SANITIZER:
      xml.Attribute("Checker", "LeakSanitizer");
      break;
    case cmCTestMemCheckHandler::THREAD_SANITIZER:
      xml.Attribute("Checker", "ThreadSanitizer");
      break;
    case cmCTestMemCheckHandler::MEMORY_SANITIZER:
      xml.Attribute("Checker", "MemorySanitizer");
      break;
    case cmCTestMemCheckHandler::UB_SANITIZER:
      xml.Attribute("Checker", "UndefinedBehaviorSanitizer");
      break;
    default:
      xml.Attribute("Checker", "Unknown");
  }

  xml.Element("StartDateTime", this->StartTest);
  xml.Element("StartTestTime", this->StartTestTime);
  xml.StartElement("TestList");
  cmCTestMemCheckHandler::TestResultsVector::size_type cc;
  for (cmCTestTestResult const& result : this->TestResults) {
    std::string testPath = result.Path + "/" + result.Name;
    xml.Element("Test", this->CTest->GetShortPathToFile(testPath));
  }
  xml.EndElement(); // TestList
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     "-- Processing memory checking output:\n", this->Quiet);
  size_t total = this->TestResults.size();
  for (cc = 0; cc < this->TestResults.size(); cc++) {
    cmCTestTestResult const& result = this->TestResults[cc];
    std::string memcheckstr;
    std::vector<int> memcheckresults(this->ResultStrings.size(), 0);
    bool res =
      this->ProcessMemCheckOutput(result.Output, memcheckstr, memcheckresults);
    if (res && result.Status == cmCTestMemCheckHandler::COMPLETED) {
      continue;
    }
    this->CleanTestOutput(
      memcheckstr,
      static_cast<size_t>(this->CustomMaximumFailedTestOutputSize));
    this->WriteTestResultHeader(xml, result);
    xml.StartElement("Results");
    int memoryErrors = 0;
    for (std::vector<int>::size_type kk = 0; kk < memcheckresults.size();
         ++kk) {
      if (memcheckresults[kk]) {
        xml.StartElement("Defect");
        xml.Attribute("type", this->ResultStringsLong[kk]);
        xml.Content(memcheckresults[kk]);
        memoryErrors += memcheckresults[kk];
        xml.EndElement(); // Defect
      }
      this->GlobalResults[kk] += memcheckresults[kk];
    }
    xml.EndElement(); // Results
    if (memoryErrors > 0) {
      const int maxTestNameWidth = this->CTest->GetMaxTestNameWidth();
      std::string outname = result.Name + " ";
      outname.resize(maxTestNameWidth + 4, '.');
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                         cc + 1 << "/" << total << " MemCheck: #"
                                << result.TestCount << ": " << outname
                                << "   Defects: " << memoryErrors << std::endl,
                         this->Quiet);
    }
    xml.StartElement("Log");
    if (this->CTest->ShouldCompressTestOutput()) {
      this->CTest->CompressString(memcheckstr);
      xml.Attribute("compression", "gzip");
      xml.Attribute("encoding", "base64");
    }
    xml.Content(memcheckstr);
    xml.EndElement(); // Log

    this->WriteTestResultFooter(xml, result);
  }
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     "MemCheck log files can be found here: "
                     "(<#> corresponds to test number)"
                       << std::endl,
                     this->Quiet);
  std::string output = this->MemoryTesterOutputFile;
  cmSystemTools::ReplaceString(output, "??", "<#>");
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, output << std::endl,
                     this->Quiet);
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     "Memory checking results:" << std::endl, this->Quiet);
  xml.StartElement("DefectList");
  for (cc = 0; cc < this->GlobalResults.size(); cc++) {
    if (this->GlobalResults[cc]) {
      std::cerr.width(35);
      cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                         this->ResultStringsLong[cc]
                           << " - " << this->GlobalResults[cc] << std::endl,
                         this->Quiet);
      xml.StartElement("Defect");
      xml.Attribute("Type", this->ResultStringsLong[cc]);
      xml.EndElement();
    }
  }
  xml.EndElement(); // DefectList

  xml.Element("EndDateTime", this->EndTest);
  xml.Element("EndTestTime", this->EndTestTime);
  xml.Element(
    "ElapsedMinutes",
    std::chrono::duration_cast<std::chrono::minutes>(this->ElapsedTestingTime)
      .count());

  xml.EndElement(); // DynamicAnalysis
  this->CTest->EndXML(xml);
}

bool cmCTestMemCheckHandler::InitializeMemoryChecking()
{
  this->MemoryTesterEnvironmentVariable.clear();
  this->MemoryTester.clear();
  // Setup the command
  if (cmSystemTools::FileExists(
        this->CTest->GetCTestConfiguration("MemoryCheckCommand"))) {
    this->MemoryTester =
      this->CTest->GetCTestConfiguration("MemoryCheckCommand");
    std::string testerName =
      cmSystemTools::GetFilenameName(this->MemoryTester);
    // determine the checker type
    if (testerName.find("valgrind") != std::string::npos ||
        this->CTest->GetCTestConfiguration("MemoryCheckType") == "Valgrind") {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::VALGRIND;
    } else if (testerName.find("drmemory") != std::string::npos ||
               this->CTest->GetCTestConfiguration("MemoryCheckType") ==
                 "DrMemory") {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::DRMEMORY;
    } else if (testerName.find("purify") != std::string::npos) {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::PURIFY;
    } else if (testerName.find("BC") != std::string::npos) {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::BOUNDS_CHECKER;
    } else if (testerName.find("cuda-memcheck") != std::string::npos ||
               testerName.find("compute-sanitizer") != std::string::npos) {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::CUDA_SANITIZER;
    } else {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::UNKNOWN;
    }
  } else if (cmSystemTools::FileExists(
               this->CTest->GetCTestConfiguration("PurifyCommand"))) {
    this->MemoryTester = this->CTest->GetCTestConfiguration("PurifyCommand");
    this->MemoryTesterStyle = cmCTestMemCheckHandler::PURIFY;
  } else if (cmSystemTools::FileExists(
               this->CTest->GetCTestConfiguration("ValgrindCommand"))) {
    this->MemoryTester = this->CTest->GetCTestConfiguration("ValgrindCommand");
    this->MemoryTesterStyle = cmCTestMemCheckHandler::VALGRIND;
  } else if (cmSystemTools::FileExists(
               this->CTest->GetCTestConfiguration("DrMemoryCommand"))) {
    this->MemoryTester = this->CTest->GetCTestConfiguration("DrMemoryCommand");
    this->MemoryTesterStyle = cmCTestMemCheckHandler::DRMEMORY;
  } else if (cmSystemTools::FileExists(
               this->CTest->GetCTestConfiguration("BoundsCheckerCommand"))) {
    this->MemoryTester =
      this->CTest->GetCTestConfiguration("BoundsCheckerCommand");
    this->MemoryTesterStyle = cmCTestMemCheckHandler::BOUNDS_CHECKER;
  } else if (cmSystemTools::FileExists(
               this->CTest->GetCTestConfiguration("CudaSanitizerCommand"))) {
    this->MemoryTester =
      this->CTest->GetCTestConfiguration("CudaSanitizerCommand");
    this->MemoryTesterStyle = cmCTestMemCheckHandler::CUDA_SANITIZER;
  }
  if (this->CTest->GetCTestConfiguration("MemoryCheckType") ==
      "AddressSanitizer") {
    this->MemoryTester = cmSystemTools::GetCMakeCommand();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::ADDRESS_SANITIZER;
    this->LogWithPID = true; // even if we give the log file the pid is added
  }
  if (this->CTest->GetCTestConfiguration("MemoryCheckType") ==
      "LeakSanitizer") {
    this->MemoryTester = cmSystemTools::GetCMakeCommand();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::LEAK_SANITIZER;
    this->LogWithPID = true; // even if we give the log file the pid is added
  }
  if (this->CTest->GetCTestConfiguration("MemoryCheckType") ==
      "ThreadSanitizer") {
    this->MemoryTester = cmSystemTools::GetCMakeCommand();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::THREAD_SANITIZER;
    this->LogWithPID = true; // even if we give the log file the pid is added
  }
  if (this->CTest->GetCTestConfiguration("MemoryCheckType") ==
      "MemorySanitizer") {
    this->MemoryTester = cmSystemTools::GetCMakeCommand();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::MEMORY_SANITIZER;
    this->LogWithPID = true; // even if we give the log file the pid is added
  }
  if (this->CTest->GetCTestConfiguration("MemoryCheckType") ==
      "UndefinedBehaviorSanitizer") {
    this->MemoryTester = cmSystemTools::GetCMakeCommand();
    this->MemoryTesterStyle = cmCTestMemCheckHandler::UB_SANITIZER;
    this->LogWithPID = true; // even if we give the log file the pid is added
  }
  // Check the MemoryCheckType
  if (this->MemoryTesterStyle == cmCTestMemCheckHandler::UNKNOWN) {
    std::string checkType =
      this->CTest->GetCTestConfiguration("MemoryCheckType");
    if (checkType == "Purify") {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::PURIFY;
    } else if (checkType == "BoundsChecker") {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::BOUNDS_CHECKER;
    } else if (checkType == "Valgrind") {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::VALGRIND;
    } else if (checkType == "DrMemory") {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::DRMEMORY;
    } else if (checkType == "CudaSanitizer") {
      this->MemoryTesterStyle = cmCTestMemCheckHandler::CUDA_SANITIZER;
    }
  }
  if (this->MemoryTester.empty()) {
    cmCTestOptionalLog(this->CTest, WARNING,
                       "Memory checker (MemoryCheckCommand) "
                       "not set, or cannot find the specified program."
                         << std::endl,
                       this->Quiet);
    return false;
  }

  // Setup the options
  std::string memoryTesterOptions;
  if (!this->CTest->GetCTestConfiguration("MemoryCheckCommandOptions")
         .empty()) {
    memoryTesterOptions =
      this->CTest->GetCTestConfiguration("MemoryCheckCommandOptions");
  } else if (!this->CTest->GetCTestConfiguration("ValgrindCommandOptions")
                .empty()) {
    memoryTesterOptions =
      this->CTest->GetCTestConfiguration("ValgrindCommandOptions");
  } else if (!this->CTest->GetCTestConfiguration("DrMemoryCommandOptions")
                .empty()) {
    memoryTesterOptions =
      this->CTest->GetCTestConfiguration("DrMemoryCommandOptions");
  } else if (!this->CTest->GetCTestConfiguration("CudaSanitizerCommandOptions")
                .empty()) {
    memoryTesterOptions =
      this->CTest->GetCTestConfiguration("CudaSanitizerCommandOptions");
  }
  this->MemoryTesterOptions =
    cmSystemTools::ParseArguments(memoryTesterOptions);

  this->MemoryTesterOutputFile =
    this->CTest->GetBinaryDir() + "/Testing/Temporary/MemoryChecker.??.log";

  switch (this->MemoryTesterStyle) {
    case cmCTestMemCheckHandler::VALGRIND: {
      if (this->MemoryTesterOptions.empty()) {
        this->MemoryTesterOptions.emplace_back("-q");
        this->MemoryTesterOptions.emplace_back("--tool=memcheck");
        this->MemoryTesterOptions.emplace_back("--leak-check=yes");
        this->MemoryTesterOptions.emplace_back("--show-reachable=yes");
        this->MemoryTesterOptions.emplace_back("--num-callers=50");
      }
      if (!this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile")
             .empty()) {
        if (!cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
              "MemoryCheckSuppressionFile"))) {
          cmCTestLog(this->CTest, ERROR_MESSAGE,
                     "Cannot find memory checker suppression file: "
                       << this->CTest->GetCTestConfiguration(
                            "MemoryCheckSuppressionFile")
                       << std::endl);
          return false;
        }
        this->MemoryTesterOptions.push_back(
          "--suppressions=" +
          this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile"));
      }
      this->MemoryTesterDynamicOptions.push_back("--log-file=" +
                                                 this->MemoryTesterOutputFile);
      break;
    }
    case cmCTestMemCheckHandler::DRMEMORY: {
      std::string tempDrMemoryDir =
        this->CTest->GetBinaryDir() + "/Testing/Temporary/DrMemory";

      if (!cm::contains(this->MemoryTesterOptions, "-quiet")) {
        this->MemoryTesterOptions.emplace_back("-quiet");
      }

      if (!cm::contains(this->MemoryTesterOptions, "-batch")) {
        this->MemoryTesterOptions.emplace_back("-batch");
      }

      this->MemoryTesterDynamicOptions.emplace_back("-logdir");
      auto logdirOption =
        std::find(this->MemoryTesterOptions.begin(),
                  this->MemoryTesterOptions.end(), "-logdir");
      if (logdirOption == this->MemoryTesterOptions.end()) {
        // No logdir found in memory tester options
        std::string drMemoryLogDir = tempDrMemoryDir + "/??";
        this->MemoryTesterDynamicOptions.push_back(drMemoryLogDir);
        this->MemoryTesterOutputFile = drMemoryLogDir;
      } else {
        // Use logdir found in memory tester options
        auto logdirLocation = std::next(logdirOption);
        this->MemoryTesterOutputFile = *logdirLocation;
        this->MemoryTesterDynamicOptions.push_back(*logdirLocation);
        this->MemoryTesterOptions.erase(logdirOption, logdirLocation + 1);
      }
      this->MemoryTesterOutputFile += "/*/results.txt";

      if (std::find(this->MemoryTesterOptions.begin(),
                    this->MemoryTesterOptions.end(),
                    "-symcache_dir") == this->MemoryTesterOptions.end()) {
        this->MemoryTesterDynamicOptions.emplace_back("-symcache_dir");
        std::string drMemoryCacheDir = tempDrMemoryDir + "/cache";
        this->MemoryTesterDynamicOptions.push_back(drMemoryCacheDir);
      }

      if (!this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile")
             .empty()) {
        if (!cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
              "MemoryCheckSuppressionFile"))) {
          cmCTestLog(this->CTest, ERROR_MESSAGE,
                     "Cannot find memory checker suppression file: "
                       << this->CTest->GetCTestConfiguration(
                            "MemoryCheckSuppressionFile")
                       << std::endl);
          return false;
        }
        this->MemoryTesterOptions.emplace_back("-suppress");
        this->MemoryTesterOptions.push_back(
          this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile"));
      }

      this->MemoryTesterOptions.emplace_back("--");

      break;
    }
    case cmCTestMemCheckHandler::PURIFY: {
      std::string outputFile;
#ifdef _WIN32
      if (this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile")
            .size()) {
        if (!cmSystemTools::FileExists(this->CTest->GetCTestConfiguration(
              "MemoryCheckSuppressionFile"))) {
          cmCTestLog(
            this->CTest, ERROR_MESSAGE,
            "Cannot find memory checker suppression file: "
              << this->CTest
                   ->GetCTestConfiguration("MemoryCheckSuppressionFile")
                   .c_str()
              << std::endl);
          return false;
        }
        std::string filterFiles = "/FilterFiles=" +
          this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile");
        this->MemoryTesterOptions.push_back(filterFiles);
      }
      outputFile = "/SAVETEXTDATA=";
#else
      outputFile = "-log-file=";
#endif
      outputFile += this->MemoryTesterOutputFile;
      this->MemoryTesterDynamicOptions.push_back(outputFile);
      break;
    }
    case cmCTestMemCheckHandler::BOUNDS_CHECKER: {
      this->BoundsCheckerXMLFile = this->MemoryTesterOutputFile;
      std::string dpbdFile = this->CTest->GetBinaryDir() +
        "/Testing/Temporary/MemoryChecker.??.DPbd";
      this->BoundsCheckerDPBDFile = dpbdFile;
      this->MemoryTesterDynamicOptions.emplace_back("/B");
      this->MemoryTesterDynamicOptions.push_back(std::move(dpbdFile));
      this->MemoryTesterDynamicOptions.emplace_back("/X");
      this->MemoryTesterDynamicOptions.push_back(this->MemoryTesterOutputFile);
      this->MemoryTesterOptions.emplace_back("/M");
      break;
    }
    case cmCTestMemCheckHandler::CUDA_SANITIZER: {
      // cuda sanitizer separates flags from arguments by spaces
      if (this->MemoryTesterOptions.empty()) {
        this->MemoryTesterOptions.emplace_back("--tool");
        this->MemoryTesterOptions.emplace_back("memcheck");
        this->MemoryTesterOptions.emplace_back("--leak-check");
        this->MemoryTesterOptions.emplace_back("full");
      }
      this->MemoryTesterDynamicOptions.emplace_back("--log-file");
      this->MemoryTesterDynamicOptions.push_back(this->MemoryTesterOutputFile);
      break;
    }
    // these are almost the same but the env var used is different
    case cmCTestMemCheckHandler::ADDRESS_SANITIZER:
    case cmCTestMemCheckHandler::LEAK_SANITIZER:
    case cmCTestMemCheckHandler::THREAD_SANITIZER:
    case cmCTestMemCheckHandler::MEMORY_SANITIZER:
    case cmCTestMemCheckHandler::UB_SANITIZER: {
      // To pass arguments to ThreadSanitizer the environment variable
      // TSAN_OPTIONS is used. This is done with the cmake -E env command.
      // The MemoryTesterDynamicOptions is setup with the -E env
      // Then the MemoryTesterEnvironmentVariable gets the
      // TSAN_OPTIONS string with the log_path in it.
      this->MemoryTesterDynamicOptions.emplace_back("-E");
      this->MemoryTesterDynamicOptions.emplace_back("env");
      std::string envVar;
      std::string extraOptions;
      std::string suppressionsOption;
      if (!this->CTest->GetCTestConfiguration("MemoryCheckSanitizerOptions")
             .empty()) {
        extraOptions = ":" +
          this->CTest->GetCTestConfiguration("MemoryCheckSanitizerOptions");
      }
      if (!this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile")
             .empty()) {
        suppressionsOption = ":suppressions=" +
          this->CTest->GetCTestConfiguration("MemoryCheckSuppressionFile");
      }
      if (this->MemoryTesterStyle ==
          cmCTestMemCheckHandler::ADDRESS_SANITIZER) {
        envVar = "ASAN_OPTIONS";
      } else if (this->MemoryTesterStyle ==
                 cmCTestMemCheckHandler::LEAK_SANITIZER) {
        envVar = "LSAN_OPTIONS";
      } else if (this->MemoryTesterStyle ==
                 cmCTestMemCheckHandler::THREAD_SANITIZER) {
        envVar = "TSAN_OPTIONS";
      } else if (this->MemoryTesterStyle ==
                 cmCTestMemCheckHandler::MEMORY_SANITIZER) {
        envVar = "MSAN_OPTIONS";
      } else if (this->MemoryTesterStyle ==
                 cmCTestMemCheckHandler::UB_SANITIZER) {
        envVar = "UBSAN_OPTIONS";
      }
      // Quote log_path with single quotes; see
      // https://bugs.chromium.org/p/chromium/issues/detail?id=467936
      std::string outputFile =
        envVar + "=log_path='" + this->MemoryTesterOutputFile + "'";
      this->MemoryTesterEnvironmentVariable =
        outputFile + suppressionsOption + extraOptions;
      break;
    }
    default:
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Do not understand memory checker: " << this->MemoryTester
                                                      << std::endl);
      return false;
  }

  this->InitializeResultsVectors();
  // std::vector<std::string>::size_type cc;
  // for ( cc = 0; cmCTestMemCheckResultStrings[cc]; cc ++ )
  //   {
  //   this->MemoryTesterGlobalResults[cc] = 0;
  //   }
  return true;
}

bool cmCTestMemCheckHandler::ProcessMemCheckOutput(const std::string& str,
                                                   std::string& log,
                                                   std::vector<int>& results)
{
  switch (this->MemoryTesterStyle) {
    case cmCTestMemCheckHandler::VALGRIND:
      return this->ProcessMemCheckValgrindOutput(str, log, results);
    case cmCTestMemCheckHandler::DRMEMORY:
      return this->ProcessMemCheckDrMemoryOutput(str, log, results);
    case cmCTestMemCheckHandler::PURIFY:
      return this->ProcessMemCheckPurifyOutput(str, log, results);
    case cmCTestMemCheckHandler::ADDRESS_SANITIZER:
    case cmCTestMemCheckHandler::LEAK_SANITIZER:
    case cmCTestMemCheckHandler::THREAD_SANITIZER:
    case cmCTestMemCheckHandler::MEMORY_SANITIZER:
    case cmCTestMemCheckHandler::UB_SANITIZER:
      return this->ProcessMemCheckSanitizerOutput(str, log, results);
    case cmCTestMemCheckHandler::BOUNDS_CHECKER:
      return this->ProcessMemCheckBoundsCheckerOutput(str, log, results);
    case cmCTestMemCheckHandler::CUDA_SANITIZER:
      return this->ProcessMemCheckCudaOutput(str, log, results);
    default:
      log.append("\nMemory checking style used was: ");
      log.append("None that I know");
      log = str;
      return true;
  }
}

std::vector<int>::size_type cmCTestMemCheckHandler::FindOrAddWarning(
  const std::string& warning)
{
  for (std::vector<std::string>::size_type i = 0;
       i < this->ResultStrings.size(); ++i) {
    if (this->ResultStrings[i] == warning) {
      return i;
    }
  }
  this->GlobalResults.push_back(0); // this must stay the same size
  this->ResultStrings.push_back(warning);
  this->ResultStringsLong.push_back(warning);
  return this->ResultStrings.size() - 1;
}
bool cmCTestMemCheckHandler::ProcessMemCheckSanitizerOutput(
  const std::string& str, std::string& log, std::vector<int>& result)
{
  std::string regex;
  switch (this->MemoryTesterStyle) {
    case cmCTestMemCheckHandler::ADDRESS_SANITIZER:
      regex = "ERROR: AddressSanitizer: (.*) on.*";
      break;
    case cmCTestMemCheckHandler::LEAK_SANITIZER:
      // use leakWarning regex
      break;
    case cmCTestMemCheckHandler::THREAD_SANITIZER:
      regex = "WARNING: ThreadSanitizer: (.*) \\(pid=.*\\)";
      break;
    case cmCTestMemCheckHandler::MEMORY_SANITIZER:
      regex = "WARNING: MemorySanitizer: (.*)";
      break;
    case cmCTestMemCheckHandler::UB_SANITIZER:
      regex = "runtime error: (.*)";
      break;
    default:
      break;
  }
  cmsys::RegularExpression sanitizerWarning(regex);
  cmsys::RegularExpression leakWarning("(Direct|Indirect) leak of .*");
  int defects = 0;
  std::vector<std::string> lines;
  cmsys::SystemTools::Split(str, lines);
  std::ostringstream ostr;
  log.clear();
  for (std::string const& l : lines) {
    std::string resultFound;
    if (leakWarning.find(l)) {
      resultFound = leakWarning.match(1) + " leak";
    } else if (sanitizerWarning.find(l)) {
      resultFound = sanitizerWarning.match(1);
    }
    if (!resultFound.empty()) {
      std::vector<int>::size_type idx = this->FindOrAddWarning(resultFound);
      if (result.empty() || idx > result.size() - 1) {
        result.push_back(1);
      } else {
        result[idx]++;
      }
      defects++;
      ostr << "<b>" << this->ResultStrings[idx] << "</b> ";
    }
    ostr << l << std::endl;
  }
  log = ostr.str();
  this->DefectCount += defects;
  return defects == 0;
}
bool cmCTestMemCheckHandler::ProcessMemCheckPurifyOutput(
  const std::string& str, std::string& log, std::vector<int>& results)
{
  std::vector<std::string> lines;
  cmsys::SystemTools::Split(str, lines);
  std::ostringstream ostr;
  log.clear();

  cmsys::RegularExpression pfW("^\\[[WEI]\\] ([A-Z][A-Z][A-Z][A-Z]*): ");

  int defects = 0;

  for (std::string const& l : lines) {
    std::vector<int>::size_type failure = this->ResultStrings.size();
    if (pfW.find(l)) {
      std::vector<int>::size_type cc;
      for (cc = 0; cc < this->ResultStrings.size(); cc++) {
        if (pfW.match(1) == this->ResultStrings[cc]) {
          failure = cc;
          break;
        }
      }
      if (cc == this->ResultStrings.size()) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "Unknown Purify memory fault: " << pfW.match(1)
                                                   << std::endl);
        ostr << "*** Unknown Purify memory fault: " << pfW.match(1)
             << std::endl;
      }
    }
    if (failure != this->ResultStrings.size()) {
      ostr << "<b>" << this->ResultStrings[failure] << "</b> ";
      results[failure]++;
      defects++;
    }
    ostr << l << std::endl;
  }

  log = ostr.str();
  this->DefectCount += defects;
  return defects == 0;
}

bool cmCTestMemCheckHandler::ProcessMemCheckValgrindOutput(
  const std::string& str, std::string& log, std::vector<int>& results)
{
  std::vector<std::string> lines;
  cmsys::SystemTools::Split(str, lines);
  bool unlimitedOutput = false;
  if (str.find("CTEST_FULL_OUTPUT") != std::string::npos ||
      this->CustomMaximumFailedTestOutputSize == 0) {
    unlimitedOutput = true;
  }

  std::string::size_type cc;

  std::ostringstream ostr;
  log.clear();

  int defects = 0;

  cmsys::RegularExpression valgrindLine("^==[0-9][0-9]*==");

  cmsys::RegularExpression vgFIM(
    R"(== .*Invalid free\(\) / delete / delete\[\])");
  cmsys::RegularExpression vgFMM(
    R"(== .*Mismatched free\(\) / delete / delete \[\])");
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
  cmsys::RegularExpression vgUMR4(
    "== .*Syscall param .* contains "
    "uninitialised or unaddressable byte\\(s\\)");
  cmsys::RegularExpression vgUMR5("== .*Syscall param .* uninitialised");
  cmsys::RegularExpression vgIPW("== .*Invalid write of size [0-9,]+");
  cmsys::RegularExpression vgABR("== .*pthread_mutex_unlock: mutex is "
                                 "locked by a different thread");
  std::vector<std::string::size_type> nonValGrindOutput;
  auto sttime = std::chrono::steady_clock::now();
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "Start test: " << lines.size() << std::endl, this->Quiet);
  std::string::size_type totalOutputSize = 0;
  for (cc = 0; cc < lines.size(); cc++) {
    cmCTestOptionalLog(this->CTest, DEBUG,
                       "test line " << lines[cc] << std::endl, this->Quiet);

    if (valgrindLine.find(lines[cc])) {
      cmCTestOptionalLog(this->CTest, DEBUG,
                         "valgrind  line " << lines[cc] << std::endl,
                         this->Quiet);
      int failure = cmCTestMemCheckHandler::NO_MEMORY_FAULT;
      auto& line = lines[cc];
      if (vgFIM.find(line)) {
        failure = cmCTestMemCheckHandler::FIM;
      } else if (vgFMM.find(line)) {
        failure = cmCTestMemCheckHandler::FMM;
      } else if (vgMLK1.find(line) || vgMLK2.find(line)) {
        failure = cmCTestMemCheckHandler::MLK;
      } else if (vgPAR.find(line)) {
        failure = cmCTestMemCheckHandler::PAR;
      } else if (vgMPK1.find(line) || vgMPK2.find(line)) {
        failure = cmCTestMemCheckHandler::MPK;
      } else if (vgUMC.find(line)) {
        failure = cmCTestMemCheckHandler::UMC;
      } else if (vgUMR1.find(line) || vgUMR2.find(line) || vgUMR3.find(line) ||
                 vgUMR4.find(line) || vgUMR5.find(line)) {
        failure = cmCTestMemCheckHandler::UMR;
      } else if (vgIPW.find(line)) {
        failure = cmCTestMemCheckHandler::IPW;
      } else if (vgABR.find(line)) {
        failure = cmCTestMemCheckHandler::ABR;
      }

      if (failure != cmCTestMemCheckHandler::NO_MEMORY_FAULT) {
        ostr << "<b>" << this->ResultStrings[failure] << "</b> ";
        results[failure]++;
        defects++;
      }
      totalOutputSize += lines[cc].size();
      ostr << lines[cc] << std::endl;
    } else {
      nonValGrindOutput.push_back(cc);
    }
  }
  // Now put all all the non valgrind output into the test output
  // This should be last in case it gets truncated by the output
  // limiting code
  for (std::string::size_type i : nonValGrindOutput) {
    totalOutputSize += lines[i].size();
    ostr << lines[i] << std::endl;
    if (!unlimitedOutput &&
        totalOutputSize >
          static_cast<size_t>(this->CustomMaximumFailedTestOutputSize)) {
      ostr << "....\n";
      ostr << "Test Output for this test has been truncated see testing"
              " machine logs for full output,\n";
      ostr << "or put CTEST_FULL_OUTPUT in the output of "
              "this test program.\n";
      break; // stop the copy of output if we are full
    }
  }
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "End test (elapsed: "
                       << cmDurationTo<unsigned int>(
                            std::chrono::steady_clock::now() - sttime)
                       << "s)" << std::endl,
                     this->Quiet);
  log = ostr.str();
  this->DefectCount += defects;
  return defects == 0;
}

bool cmCTestMemCheckHandler::ProcessMemCheckDrMemoryOutput(
  const std::string& str, std::string& log, std::vector<int>& results)
{
  std::vector<std::string> lines;
  cmsys::SystemTools::Split(str, lines);

  cmsys::RegularExpression drMemoryError("^Error #[0-9]+");

  cmsys::RegularExpression unaddressableAccess("UNADDRESSABLE ACCESS");
  cmsys::RegularExpression uninitializedRead("UNINITIALIZED READ");
  cmsys::RegularExpression invalidHeapArgument("INVALID HEAP ARGUMENT");
  cmsys::RegularExpression leak("LEAK");
  cmsys::RegularExpression handleLeak("HANDLE LEAK");

  int defects = 0;

  std::ostringstream ostr;
  for (const auto& l : lines) {
    ostr << l << std::endl;
    if (drMemoryError.find(l)) {
      defects++;
      if (unaddressableAccess.find(l) || uninitializedRead.find(l)) {
        results[cmCTestMemCheckHandler::UMR]++;
      } else if (leak.find(l) || handleLeak.find(l)) {
        results[cmCTestMemCheckHandler::MLK]++;
      } else if (invalidHeapArgument.find(l)) {
        results[cmCTestMemCheckHandler::FMM]++;
      }
    }
  }

  log = ostr.str();

  this->DefectCount += defects;
  return defects == 0;
}

bool cmCTestMemCheckHandler::ProcessMemCheckBoundsCheckerOutput(
  const std::string& str, std::string& log, std::vector<int>& results)
{
  log.clear();
  auto sttime = std::chrono::steady_clock::now();
  std::vector<std::string> lines;
  cmsys::SystemTools::Split(str, lines);
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "Start test: " << lines.size() << std::endl, this->Quiet);
  std::vector<std::string>::size_type cc;
  for (cc = 0; cc < lines.size(); cc++) {
    if (lines[cc] == BOUNDS_CHECKER_MARKER) {
      break;
    }
  }
  cmBoundsCheckerParser parser(this->CTest);
  parser.InitializeParser();
  if (cc < lines.size()) {
    for (cc++; cc < lines.size(); ++cc) {
      std::string& theLine = lines[cc];
      // check for command line arguments that are not escaped
      // correctly by BC
      if (theLine.find("TargetArgs=") != std::string::npos) {
        // skip this because BC gets it wrong and we can't parse it
      } else if (!parser.ParseChunk(theLine.c_str(), theLine.size())) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "Error in ParseChunk: " << theLine << std::endl);
      }
    }
  }
  int defects = 0;
  for (int err : parser.Errors) {
    results[err]++;
    defects++;
  }
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "End test (elapsed: "
                       << cmDurationTo<unsigned int>(
                            std::chrono::steady_clock::now() - sttime)
                       << "s)" << std::endl,
                     this->Quiet);
  if (defects) {
    // only put the output of Bounds Checker if there were
    // errors or leaks detected
    log = parser.Log;
  }
  this->DefectCount += defects;
  return defects == 0;
}

bool cmCTestMemCheckHandler::ProcessMemCheckCudaOutput(
  const std::string& str, std::string& log, std::vector<int>& results)
{
  std::vector<std::string> lines;
  cmsys::SystemTools::Split(str, lines);
  bool unlimitedOutput = false;
  if (str.find("CTEST_FULL_OUTPUT") != std::string::npos ||
      this->CustomMaximumFailedTestOutputSize == 0) {
    unlimitedOutput = true;
  }

  std::string::size_type cc;

  std::ostringstream ostr;
  log.clear();

  int defects = 0;

  cmsys::RegularExpression memcheckLine("^========");

  cmsys::RegularExpression leakExpr("== Leaked [0-9,]+ bytes at");

  // list of matchers for output messages that contain variable content
  // (addresses, sizes, ...) or can be shortened in general. the first match is
  // used as a error name.
  std::vector<cmsys::RegularExpression> matchers{
    // API errors
    "== Malloc/Free error encountered: (.*)",
    "== Program hit error ([^ ]*).* on CUDA API call to",
    "== Program hit ([^ ]*).* on CUDA API call to",
    // memcheck
    "== (Invalid .*) of size [0-9,]+", "== (Fatal UVM [CG]PU fault)",
    // racecheck
    "== .* (Potential .* hazard detected)", "== .* (Race reported)",
    // synccheck
    "== (Barrier error)",
    // initcheck
    "== (Uninitialized .* memory read)", "== (Unused memory)",
    "== (Host API memory access error)",
    // generic error: ignore ERROR SUMMARY, CUDA-MEMCHECK and others
    "== ([A-Z][a-z].*)"
  };

  std::vector<std::string::size_type> nonMemcheckOutput;
  auto sttime = std::chrono::steady_clock::now();
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "Start test: " << lines.size() << std::endl, this->Quiet);
  std::string::size_type totalOutputSize = 0;
  for (cc = 0; cc < lines.size(); cc++) {
    cmCTestOptionalLog(this->CTest, DEBUG,
                       "test line " << lines[cc] << std::endl, this->Quiet);

    if (memcheckLine.find(lines[cc])) {
      cmCTestOptionalLog(this->CTest, DEBUG,
                         "cuda sanitizer line " << lines[cc] << std::endl,
                         this->Quiet);
      int failure = -1;
      auto& line = lines[cc];
      if (leakExpr.find(line)) {
        failure = static_cast<int>(this->FindOrAddWarning("Memory leak"));
      } else {
        for (auto& matcher : matchers) {
          if (matcher.find(line)) {
            failure =
              static_cast<int>(this->FindOrAddWarning(matcher.match(1)));
            break;
          }
        }
      }

      if (failure >= 0) {
        ostr << "<b>" << this->ResultStrings[failure] << "</b> ";
        if (results.empty() || unsigned(failure) > results.size() - 1) {
          results.push_back(1);
        } else {
          results[failure]++;
        }
        defects++;
      }
      totalOutputSize += lines[cc].size();
      ostr << lines[cc] << std::endl;
    } else {
      nonMemcheckOutput.push_back(cc);
    }
  }
  // Now put all all the non cuda sanitizer output into the test output
  // This should be last in case it gets truncated by the output
  // limiting code
  for (std::string::size_type i : nonMemcheckOutput) {
    totalOutputSize += lines[i].size();
    ostr << lines[i] << std::endl;
    if (!unlimitedOutput &&
        totalOutputSize >
          static_cast<size_t>(this->CustomMaximumFailedTestOutputSize)) {
      ostr << "....\n";
      ostr << "Test Output for this test has been truncated see testing"
              " machine logs for full output,\n";
      ostr << "or put CTEST_FULL_OUTPUT in the output of "
              "this test program.\n";
      break; // stop the copy of output if we are full
    }
  }
  cmCTestOptionalLog(this->CTest, DEBUG,
                     "End test (elapsed: "
                       << cmDurationTo<unsigned int>(
                            std::chrono::steady_clock::now() - sttime)
                       << "s)" << std::endl,
                     this->Quiet);
  log = ostr.str();
  this->DefectCount += defects;
  return defects == 0;
}

// PostProcessTest memcheck results
void cmCTestMemCheckHandler::PostProcessTest(cmCTestTestResult& res, int test)
{
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "PostProcessTest memcheck results for : " << res.Name
                                                               << std::endl,
                     this->Quiet);
  if (this->MemoryTesterStyle == cmCTestMemCheckHandler::BOUNDS_CHECKER) {
    this->PostProcessBoundsCheckerTest(res, test);
  } else if (this->MemoryTesterStyle == cmCTestMemCheckHandler::DRMEMORY) {
    this->PostProcessDrMemoryTest(res, test);
  } else {
    std::vector<std::string> files;
    this->TestOutputFileNames(test, files);
    for (std::string const& f : files) {
      this->AppendMemTesterOutput(res, f);
    }
  }
}

// This method puts the bounds checker output file into the output
// for the test
void cmCTestMemCheckHandler::PostProcessBoundsCheckerTest(
  cmCTestTestResult& res, int test)
{
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "PostProcessBoundsCheckerTest for : " << res.Name
                                                           << std::endl,
                     this->Quiet);
  std::vector<std::string> files;
  this->TestOutputFileNames(test, files);
  if (files.empty()) {
    return;
  }
  std::string ofile = files[0];
  if (ofile.empty()) {
    return;
  }
  // put a scope around this to close ifs so the file can be removed
  {
    cmsys::ifstream ifs(ofile.c_str());
    if (!ifs) {
      std::string log = "Cannot read memory tester output file: " + ofile;
      cmCTestLog(this->CTest, ERROR_MESSAGE, log << std::endl);
      return;
    }
    res.Output += BOUNDS_CHECKER_MARKER;
    res.Output += "\n";
    std::string line;
    while (cmSystemTools::GetLineFromStream(ifs, line)) {
      res.Output += line;
      res.Output += "\n";
    }
  }
  cmSystemTools::Delay(1000);
  cmSystemTools::RemoveFile(this->BoundsCheckerDPBDFile);
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Remove: " << this->BoundsCheckerDPBDFile << std::endl,
                     this->Quiet);
  cmSystemTools::RemoveFile(this->BoundsCheckerXMLFile);
  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Remove: " << this->BoundsCheckerXMLFile << std::endl,
                     this->Quiet);
}

void cmCTestMemCheckHandler::PostProcessDrMemoryTest(
  cmCTestTestHandler::cmCTestTestResult& res, int test)
{
  std::string drMemoryLogDir = this->MemoryTesterOutputFile.substr(
    0, this->MemoryTesterOutputFile.find("/*/results.txt"));

  // replace placeholder of test
  std::string::size_type pos = drMemoryLogDir.find("??");
  if (pos != std::string::npos) {
    drMemoryLogDir.replace(pos, 2, std::to_string(test));
  }

  cmsys::Glob g;
  g.FindFiles(drMemoryLogDir + "/resfile.*");
  const std::vector<std::string>& files = g.GetFiles();

  for (const std::string& f : files) {
    cmsys::ifstream ifs(f.c_str());
    if (!ifs) {
      std::string log = "Cannot read memory tester output file: " + f;
      cmCTestLog(this->CTest, ERROR_MESSAGE, log << std::endl);
      return;
    }
    std::string resultFileLocation;
    cmSystemTools::GetLineFromStream(ifs, resultFileLocation);
    this->AppendMemTesterOutput(res, resultFileLocation);
    ifs.close();
    cmSystemTools::RemoveFile(f);
  }
}

void cmCTestMemCheckHandler::AppendMemTesterOutput(cmCTestTestResult& res,
                                                   std::string const& ofile)
{
  if (ofile.empty()) {
    return;
  }
  // put ifs in scope so file can be deleted if needed
  {
    cmsys::ifstream ifs(ofile.c_str());
    if (!ifs) {
      std::string log = "Cannot read memory tester output file: " + ofile;
      cmCTestLog(this->CTest, ERROR_MESSAGE, log << std::endl);
      return;
    }
    std::string line;
    while (cmSystemTools::GetLineFromStream(ifs, line)) {
      res.Output += line;
      res.Output += "\n";
    }
  }
  if (this->LogWithPID) {
    auto pos = ofile.find_last_of('.');
    if (pos != std::string::npos) {
      auto ofileWithoutPid = ofile.substr(0, pos);
      cmSystemTools::RenameFile(ofile, ofileWithoutPid);
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "Renaming: " << ofile << " to: " << ofileWithoutPid
                                      << "\n",
                         this->Quiet);
    }
  }
}

void cmCTestMemCheckHandler::TestOutputFileNames(
  int test, std::vector<std::string>& files)
{
  std::string index = std::to_string(test);
  std::string ofile = this->MemoryTesterOutputFile;
  std::string::size_type pos = ofile.find("??");
  ofile.replace(pos, 2, index);
  if (this->LogWithPID) {
    ofile += ".*";
    cmsys::Glob g;
    g.FindFiles(ofile);
    if (g.GetFiles().empty()) {
      std::string log = "Cannot find memory tester output file: " + ofile;
      cmCTestLog(this->CTest, WARNING, log << std::endl);
      ofile.clear();
    } else {
      files = g.GetFiles();
      return;
    }
  } else if (!cmSystemTools::FileExists(ofile)) {
    std::string log = "Cannot find memory tester output file: " + ofile;
    cmCTestLog(this->CTest, WARNING, log << std::endl);
    ofile.clear();
  }
  files.push_back(std::move(ofile));
}
