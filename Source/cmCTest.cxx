/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTest.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <initializer_list>
#include <iostream>
#include <map>
#include <ratio>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/curl/curl.h>
#include <cm3p/json/value.h>
#include <cm3p/uv.h>
#include <cm3p/zlib.h>

#include "cmsys/Base64.h"
#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"
#include "cmsys/SystemInformation.hxx"
#if defined(_WIN32)
#  include <windows.h> // IWYU pragma: keep
#else
#  include <unistd.h> // IWYU pragma: keep
#endif

#include "cmCMakePresetsGraph.h"
#include "cmCTestBuildAndTest.h"
#include "cmCTestScriptHandler.h"
#include "cmCTestTestHandler.h"
#include "cmCTestTypes.h"
#include "cmCommandLineArgument.h"
#include "cmDynamicLoader.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmInstrumentation.h"
#include "cmInstrumentationQuery.h"
#include "cmJSONState.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmProcessOutput.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmVersionConfig.h"
#include "cmWorkingDirectory.h"
#include "cmXMLWriter.h"
#include "cmake.h"

#if defined(__BEOS__) || defined(__HAIKU__)
#  include <be/kernel/OS.h> /* disable_debugger() API. */
#endif

struct cmCTest::Private
{
  Private(cmCTest* ctest)
    : BuildAndTest(ctest)
  {
  }

  /** Representation of one part.  */
  struct PartInfo
  {
    void SetName(std::string const& name) { this->Name = name; }
    std::string const& GetName() const { return this->Name; }

    void Enable() { this->Enabled = true; }
    explicit operator bool() const { return this->Enabled; }

    std::vector<std::string> SubmitFiles;

  private:
    bool Enabled = false;
    std::string Name;
  };

  int RepeatCount = 1; // default to run each test once
  cmCTest::Repeat RepeatMode = cmCTest::Repeat::Never;
  std::string ConfigType;
  std::string ScheduleType;
  std::chrono::system_clock::time_point StopTime;
  bool StopOnFailure = false;
  bool TestProgressOutput = false;
  bool Verbose = false;
  bool ExtraVerbose = false;
  bool ProduceXML = false;
  bool LabelSummary = true;
  bool SubprojectSummary = true;
  bool UseHTTP10 = false;
  bool PrintLabels = false;
  bool Failover = false;
  bool UseVerboseInstrumentation = false;
  cmJSONState parseState;

  bool FlushTestProgressLine = false;

  // these are helper classes
  cmCTestBuildAndTest BuildAndTest;

  bool ShowOnly = false;
  bool OutputAsJson = false;
  int OutputAsJsonVersion = 1;

  // TODO: The ctest configuration should be a hierarchy of
  // configuration option sources: command-line, script, ini file.
  // Then the ini file can get re-loaded whenever it changes without
  // affecting any higher-precedence settings.
  std::map<std::string, std::string> CTestConfiguration;
  std::map<std::string, std::string> CTestConfigurationOverwrites;

  PartInfo Parts[PartCount];
  std::map<std::string, Part> PartMap;

  std::string CurrentTag;
  bool TomorrowTag = false;

  int TestModel = cmCTest::EXPERIMENTAL;
  std::string SpecificGroup;

  cmDuration TimeOut = cmDuration::zero();

  cmDuration GlobalTimeout = cmDuration::zero();

  std::chrono::steady_clock::time_point StartTime =
    std::chrono::steady_clock::now();
  cmDuration TimeLimit = cmCTest::MaxDuration();

  int MaxTestNameWidth = 30;

  cm::optional<size_t> ParallelLevel = 1;
  bool ParallelLevelSetInCli = false;

  unsigned long TestLoad = 0;

  int CompatibilityMode;

  // information for the --build-and-test options
  std::string BinaryDir;
  std::string TestDir;

  std::string NotesFiles;

  bool InteractiveDebugMode = true;

  bool ShortDateFormat = true;

  bool CompressXMLFiles = false;
  bool CompressTestOutput = true;

  bool Debug = false;
  bool Quiet = false;

  std::string BuildID;

  std::vector<std::string> InitialCommandLineArguments;

  int SubmitIndex = 0;

  std::unique_ptr<cmGeneratedFileStream> OutputLogFile;
  cm::optional<cmCTest::LogType> OutputLogFileLastTag;

  bool OutputTestOutputOnTestFailure = false;
  bool OutputColorCode = cmCTest::ColoredOutputSupportedByConsole();

  std::map<std::string, std::string> Definitions;

  cmCTest::NoTests NoTestsMode = cmCTest::NoTests::Legacy;
  bool NoTestsModeSetInCli = false;

  cmCTestTestOptions TestOptions;
  std::vector<std::string> CommandLineHttpHeaders;

  std::unique_ptr<cmInstrumentation> Instrumentation;
};

struct tm* cmCTest::GetNightlyTime(std::string const& str, bool tomorrowtag)
{
  struct tm* lctime;
  time_t tctime = time(nullptr);
  lctime = gmtime(&tctime);
  char buf[1024];
  // add todays year day and month to the time in str because
  // curl_getdate no longer assumes the day is today
  std::snprintf(buf, sizeof(buf), "%d%02d%02d %s", lctime->tm_year + 1900,
                lctime->tm_mon + 1, lctime->tm_mday, str.c_str());
  cmCTestLog(this, OUTPUT,
             "Determine Nightly Start Time" << std::endl
                                            << "   Specified time: " << str
                                            << std::endl);
  // Convert the nightly start time to seconds. Since we are
  // providing only a time and a timezone, the current date of
  // the local machine is assumed. Consequently, nightlySeconds
  // is the time at which the nightly dashboard was opened or
  // will be opened on the date of the current client machine.
  // As such, this time may be in the past or in the future.
  time_t ntime = curl_getdate(buf, &tctime);
  cmCTestLog(this, DEBUG, "   Get curl time: " << ntime << std::endl);
  tctime = time(nullptr);
  cmCTestLog(this, DEBUG, "   Get the current time: " << tctime << std::endl);

  int const dayLength = 24 * 60 * 60;
  cmCTestLog(this, DEBUG, "Seconds: " << tctime << std::endl);
  while (ntime > tctime) {
    // If nightlySeconds is in the past, this is the current
    // open dashboard, then return nightlySeconds.  If
    // nightlySeconds is in the future, this is the next
    // dashboard to be opened, so subtract 24 hours to get the
    // time of the current open dashboard
    ntime -= dayLength;
    cmCTestLog(this, DEBUG, "Pick yesterday" << std::endl);
    cmCTestLog(this, DEBUG,
               "   Future time, subtract day: " << ntime << std::endl);
  }
  while (tctime > (ntime + dayLength)) {
    ntime += dayLength;
    cmCTestLog(this, DEBUG, "   Past time, add day: " << ntime << std::endl);
  }
  cmCTestLog(this, DEBUG, "nightlySeconds: " << ntime << std::endl);
  cmCTestLog(this, DEBUG,
             "   Current time: " << tctime << " Nightly time: " << ntime
                                 << std::endl);
  if (tomorrowtag) {
    cmCTestLog(this, OUTPUT, "   Use future tag, Add a day" << std::endl);
    ntime += dayLength;
  }
  lctime = gmtime(&ntime);
  return lctime;
}

bool cmCTest::GetTomorrowTag() const
{
  return this->Impl->TomorrowTag;
}

std::string cmCTest::CleanString(std::string const& str,
                                 std::string::size_type spos)
{
  spos = str.find_first_not_of(" \n\t\r\f\v", spos);
  std::string::size_type epos = str.find_last_not_of(" \n\t\r\f\v");
  if (spos == std::string::npos) {
    return std::string();
  }
  if (epos != std::string::npos) {
    epos = epos - spos + 1;
  }
  return str.substr(spos, epos);
}

std::string cmCTest::CurrentTime()
{
  time_t currenttime = time(nullptr);
  struct tm* t = localtime(&currenttime);
  // return ::CleanString(ctime(&currenttime));
  char current_time[1024];
  if (this->Impl->ShortDateFormat) {
    strftime(current_time, 1000, "%b %d %H:%M %Z", t);
  } else {
    strftime(current_time, 1000, "%a %b %d %H:%M:%S %Z %Y", t);
  }
  cmCTestLog(this, DEBUG, "   Current_Time: " << current_time << std::endl);
  return cmCTest::CleanString(current_time);
}

std::string cmCTest::GetCostDataFile()
{
  std::string fname = this->GetCTestConfiguration("CostDataFile");
  if (fname.empty()) {
    fname = this->GetBinaryDir() + "/Testing/Temporary/CTestCostData.txt";
  }
  return fname;
}

std::string cmCTest::DecodeURL(std::string const& in)
{
  std::string out;
  for (char const* c = in.c_str(); *c; ++c) {
    if (*c == '%' && isxdigit(*(c + 1)) && isxdigit(*(c + 2))) {
      char buf[3] = { *(c + 1), *(c + 2), 0 };
      out.append(1, static_cast<char>(strtoul(buf, nullptr, 16)));
      c += 2;
    } else {
      out.append(1, *c);
    }
  }
  return out;
}

cmCTest::cmCTest()
  : Impl(cm::make_unique<Private>(this))
{
  std::string envValue;
  if (cmSystemTools::GetEnv("CTEST_OUTPUT_ON_FAILURE", envValue)) {
    this->Impl->OutputTestOutputOnTestFailure = !cmIsOff(envValue);
  }
  envValue.clear();
  if (cmSystemTools::GetEnv("CTEST_PROGRESS_OUTPUT", envValue)) {
    this->Impl->TestProgressOutput = !cmIsOff(envValue);
  }
  envValue.clear();
  if (cmSystemTools::GetEnv("CTEST_USE_VERBOSE_INSTRUMENTATION", envValue)) {
    this->Impl->UseVerboseInstrumentation = !cmIsOff(envValue);
  }
  envValue.clear();

  this->Impl->Parts[PartStart].SetName("Start");
  this->Impl->Parts[PartUpdate].SetName("Update");
  this->Impl->Parts[PartConfigure].SetName("Configure");
  this->Impl->Parts[PartBuild].SetName("Build");
  this->Impl->Parts[PartTest].SetName("Test");
  this->Impl->Parts[PartCoverage].SetName("Coverage");
  this->Impl->Parts[PartMemCheck].SetName("MemCheck");
  this->Impl->Parts[PartSubmit].SetName("Submit");
  this->Impl->Parts[PartNotes].SetName("Notes");
  this->Impl->Parts[PartExtraFiles].SetName("ExtraFiles");
  this->Impl->Parts[PartUpload].SetName("Upload");
  this->Impl->Parts[PartDone].SetName("Done");

  // Fill the part name-to-id map.
  for (Part p = PartStart; p != PartCount; p = static_cast<Part>(p + 1)) {
    this->Impl
      ->PartMap[cmSystemTools::LowerCase(this->Impl->Parts[p].GetName())] = p;
  }

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();
}

cmCTest::~cmCTest() = default;

cm::optional<size_t> cmCTest::GetParallelLevel() const
{
  return this->Impl->ParallelLevel;
}

void cmCTest::SetParallelLevel(cm::optional<size_t> level)
{
  this->Impl->ParallelLevel = level;
}

unsigned long cmCTest::GetTestLoad() const
{
  return this->Impl->TestLoad;
}

void cmCTest::SetTestLoad(unsigned long load)
{
  this->Impl->TestLoad = load;
}

bool cmCTest::ShouldCompressTestOutput()
{
  return this->Impl->CompressTestOutput;
}

cmCTest::Part cmCTest::GetPartFromName(std::string const& name)
{
  // Look up by lower-case to make names case-insensitive.
  std::string lower_name = cmSystemTools::LowerCase(name);
  auto const i = this->Impl->PartMap.find(lower_name);
  if (i != this->Impl->PartMap.end()) {
    return i->second;
  }

  // The string does not name a valid part.
  return PartCount;
}

void cmCTest::Initialize(std::string const& binary_dir)
{
  this->Impl->BuildID = "";
  for (Part p = PartStart; p != PartCount; p = static_cast<Part>(p + 1)) {
    this->Impl->Parts[p].SubmitFiles.clear();
  }

  if (!this->Impl->InteractiveDebugMode) {
    this->BlockTestErrorDiagnostics();
  } else {
    cmSystemTools::PutEnv("CTEST_INTERACTIVE_DEBUG_MODE=1");
  }

  this->Impl->BinaryDir = binary_dir;
  cmSystemTools::ConvertToUnixSlashes(this->Impl->BinaryDir);
}

bool cmCTest::CreateNewTag(bool quiet)
{
  std::string const testingDir = this->Impl->BinaryDir + "/Testing";
  std::string const tagfile = testingDir + "/TAG";

  auto const result = cmSystemTools::MakeDirectory(testingDir);
  if (!result.IsSuccess()) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Cannot create directory \""
                 << testingDir << "\": " << result.GetString() << std::endl);
    return false;
  }

  cmCTestOptionalLog(this, DEBUG,
                     "TestModel: " << this->GetTestGroupString() << std::endl,
                     quiet);
  cmCTestOptionalLog(
    this, DEBUG, "TestModel: " << this->Impl->TestModel << std::endl, quiet);

  struct tm* lctime = [this]() -> tm* {
    if (this->Impl->TestModel == cmCTest::NIGHTLY) {
      return this->GetNightlyTime(
        this->GetCTestConfiguration("NightlyStartTime"),
        this->Impl->TomorrowTag);
    }
    time_t tctime = time(nullptr);
    if (this->Impl->TomorrowTag) {
      tctime += (24 * 60 * 60);
    }
    return gmtime(&tctime);
  }();

  char datestring[100];
  snprintf(datestring, sizeof(datestring), "%04d%02d%02d-%02d%02d",
           lctime->tm_year + 1900, lctime->tm_mon + 1, lctime->tm_mday,
           lctime->tm_hour, lctime->tm_min);
  this->Impl->CurrentTag = datestring;

  cmsys::ofstream ofs(tagfile.c_str());
  ofs << this->Impl->CurrentTag << std::endl;
  ofs << this->GetTestGroupString() << std::endl;
  ofs << this->GetTestModelString() << std::endl;

  return true;
}

bool cmCTest::ReadExistingTag(bool quiet)
{
  std::string const testingDir = this->Impl->BinaryDir + "/Testing";
  std::string const tagfile = testingDir + "/TAG";

  std::string tag;
  std::string group;
  std::string modelStr;
  int model = cmCTest::UNKNOWN;

  cmsys::ifstream tfin(tagfile.c_str());
  if (tfin) {
    cmSystemTools::GetLineFromStream(tfin, tag);
    cmSystemTools::GetLineFromStream(tfin, group);
    if (cmSystemTools::GetLineFromStream(tfin, modelStr)) {
      model = GetTestModelFromString(modelStr);
    }
    tfin.close();
  }

  if (tag.empty()) {
    if (!quiet) {
      cmCTestLog(this, ERROR_MESSAGE,
                 "Cannot read existing TAG file in " << testingDir
                                                     << std::endl);
    }
    return false;
  }

  if (this->Impl->TestModel == cmCTest::UNKNOWN) {
    if (model == cmCTest::UNKNOWN) {
      cmCTestLog(this, ERROR_MESSAGE,
                 "TAG file does not contain model and "
                 "no model specified in start command"
                   << std::endl);
      return false;
    }

    this->SetTestModel(model);
  }

  if (model != this->Impl->TestModel && model != cmCTest::UNKNOWN &&
      this->Impl->TestModel != cmCTest::UNKNOWN) {
    cmCTestOptionalLog(this, WARNING,
                       "Model given in TAG does not match "
                       "model given in ctest_start()"
                         << std::endl,
                       quiet);
  }

  if (!this->Impl->SpecificGroup.empty() &&
      group != this->Impl->SpecificGroup) {
    cmCTestOptionalLog(this, WARNING,
                       "Group given in TAG does not match "
                       "group given in ctest_start()"
                         << std::endl,
                       quiet);
  } else {
    this->Impl->SpecificGroup = group;
  }

  cmCTestOptionalLog(this, OUTPUT,
                     "  Use existing tag: " << tag << " - "
                                            << this->GetTestGroupString()
                                            << std::endl,
                     quiet);

  this->Impl->CurrentTag = tag;
  return true;
}

bool cmCTest::UpdateCTestConfiguration()
{
  std::string fileName = this->Impl->BinaryDir + "/CTestConfiguration.ini";
  if (!cmSystemTools::FileExists(fileName)) {
    fileName = this->Impl->BinaryDir + "/DartConfiguration.tcl";
  }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
             "UpdateCTestConfiguration  from :" << fileName << "\n");
  if (!cmSystemTools::FileExists(fileName)) {
    // No need to exit if we are not producing XML
    if (this->Impl->ProduceXML) {
      cmCTestLog(this, WARNING, "Cannot find file: " << fileName << std::endl);
      return false;
    }
  } else {
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
               "Parse Config file:" << fileName << "\n");
    // parse the dart test file
    cmsys::ifstream fin(fileName.c_str());

    if (!fin) {
      return false;
    }

    char buffer[1024];
    while (fin) {
      buffer[0] = 0;
      fin.getline(buffer, 1023);
      buffer[1023] = 0;
      std::string line = cmCTest::CleanString(buffer);
      if (line.empty()) {
        continue;
      }
      while (fin && (line.back() == '\\')) {
        line.resize(line.size() - 1);
        buffer[0] = 0;
        fin.getline(buffer, 1023);
        buffer[1023] = 0;
        line += cmCTest::CleanString(buffer);
      }
      if (line[0] == '#') {
        continue;
      }
      std::string::size_type cpos = line.find_first_of(':');
      if (cpos == std::string::npos) {
        continue;
      }
      std::string key = line.substr(0, cpos);
      std::string value = cmCTest::CleanString(line, cpos + 1);
      this->Impl->CTestConfiguration[key] = value;
    }
    fin.close();
  }
  if (!this->GetCTestConfiguration("BuildDirectory").empty()) {
    this->Impl->BinaryDir = this->GetCTestConfiguration("BuildDirectory");
    if (this->Impl->TestDir.empty()) {
      cmSystemTools::SetLogicalWorkingDirectory(this->Impl->BinaryDir);
    }
  }
  this->Impl->TimeOut =
    std::chrono::seconds(atoi(this->GetCTestConfiguration("TimeOut").c_str()));
  std::string const& testLoad = this->GetCTestConfiguration("TestLoad");
  if (!testLoad.empty()) {
    unsigned long load;
    if (cmStrToULong(testLoad, &load)) {
      this->SetTestLoad(load);
    } else {
      cmCTestLog(this, WARNING,
                 "Invalid value for 'Test Load' : " << testLoad << std::endl);
    }
  }
  if (this->Impl->ProduceXML) {
    this->Impl->CompressXMLFiles =
      cmIsOn(this->GetCTestConfiguration("CompressSubmission"));
  }
  return true;
}

void cmCTest::BlockTestErrorDiagnostics()
{
  cmSystemTools::PutEnv("DART_TEST_FROM_DART=1");
  cmSystemTools::PutEnv("DASHBOARD_TEST_FROM_CTEST=" CMake_VERSION);
#if defined(_WIN32)
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#elif defined(__BEOS__) || defined(__HAIKU__)
  disable_debugger(1);
#endif
}

void cmCTest::SetTestModel(int mode)
{
  this->Impl->InteractiveDebugMode = false;
  this->Impl->TestModel = mode;
}

int cmCTest::GetTestModel() const
{
  return this->Impl->TestModel;
}

bool cmCTest::SetTest(std::string const& ttype, bool report)
{
  if (cmSystemTools::LowerCase(ttype) == "all") {
    for (Part p = PartStart; p != PartCount; p = static_cast<Part>(p + 1)) {
      this->Impl->Parts[p].Enable();
    }
    return true;
  }
  Part p = this->GetPartFromName(ttype);
  if (p != PartCount) {
    this->Impl->Parts[p].Enable();
    return true;
  }
  if (report) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Don't know about test \"" << ttype << "\" yet..."
                                          << std::endl);
  }
  return false;
}

bool cmCTest::OpenOutputFile(std::string const& path, std::string const& name,
                             cmGeneratedFileStream& stream, bool compress)
{
  std::string testingDir = this->Impl->BinaryDir + "/Testing";
  if (!path.empty()) {
    testingDir += "/" + path;
  }
  if (cmSystemTools::FileExists(testingDir)) {
    if (!cmSystemTools::FileIsDirectory(testingDir)) {
      cmCTestLog(this, ERROR_MESSAGE,
                 "File " << testingDir
                         << " is in the place of the testing directory"
                         << std::endl);
      return false;
    }
  } else {
    if (!cmSystemTools::MakeDirectory(testingDir)) {
      cmCTestLog(this, ERROR_MESSAGE,
                 "Cannot create directory " << testingDir << std::endl);
      return false;
    }
  }
  std::string filename = testingDir + "/" + name;
  stream.Open(filename);
  if (!stream) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Problem opening file: " << filename << std::endl);
    return false;
  }
  if (compress) {
    if (this->Impl->CompressXMLFiles) {
      stream.SetCompression(true);
    }
  }
  return true;
}

bool cmCTest::AddIfExists(Part part, std::string const& file)
{
  if (this->CTestFileExists(file)) {
    this->AddSubmitFile(part, file);
  } else {
    std::string name = cmStrCat(file, ".gz");
    if (this->CTestFileExists(name)) {
      this->AddSubmitFile(part, file);
    } else {
      return false;
    }
  }
  return true;
}

bool cmCTest::CTestFileExists(std::string const& filename)
{
  std::string testingDir = this->Impl->BinaryDir + "/Testing/" +
    this->Impl->CurrentTag + "/" + filename;
  return cmSystemTools::FileExists(testingDir);
}

int cmCTest::ProcessSteps()
{
  this->Impl->ExtraVerbose = this->Impl->Verbose;
  this->Impl->Verbose = true;
  this->Impl->ProduceXML = true;

  // Minimal dashboard client script configuration.
  this->SetCTestConfiguration("BuildDirectory", this->Impl->BinaryDir);

  this->UpdateCTestConfiguration();
  this->BlockTestErrorDiagnostics();

  int res = 0;
  cmCTestScriptHandler script(this);
  script.CreateCMake();
  cmMakefile& mf = *script.GetMakefile();
  this->ReadCustomConfigurationFileTree(this->Impl->BinaryDir, &mf);
  this->SetTimeLimit(mf.GetDefinition("CTEST_TIME_LIMIT"));
  this->SetCMakeVariables(mf);
  std::vector<cmListFileArgument> args{
    cmListFileArgument("RETURN_VALUE", cmListFileArgument::Unquoted, 0),
    cmListFileArgument("return_value", cmListFileArgument::Unquoted, 0),
  };

  if (this->Impl->Parts[PartStart]) {
    auto const func = cmListFileFunction(
      "ctest_start", 0, 0,
      {
        { this->GetTestModelString(), cmListFileArgument::Unquoted, 0 },
        { "GROUP", cmListFileArgument::Unquoted, 0 },
        { this->GetTestGroupString(), cmListFileArgument::Unquoted, 0 },
      });
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status)) {
      return 12;
    }
  } else if (!this->ReadExistingTag(true) && !this->CreateNewTag(false)) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Problem initializing the dashboard." << std::endl);
    return 12;
  }

  if (this->Impl->Parts[PartUpdate] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    auto const func = cmListFileFunction("ctest_update", 0, 0, args);
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status)) {
      res |= cmCTest::UPDATE_ERRORS;
    }
  }
  if (this->Impl->TestModel == cmCTest::CONTINUOUS &&
      mf.GetDefinition("return_value").IsOff()) {
    return 0;
  }
  if (this->Impl->Parts[PartConfigure] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    auto const func = cmListFileFunction("ctest_configure", 0, 0, args);
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status) ||
        std::stoi(mf.GetDefinition("return_value")) < 0) {
      res |= cmCTest::CONFIGURE_ERRORS;
    }
  }
  if (this->Impl->Parts[PartBuild] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    this->SetCMakeVariables(mf);
    auto const func = cmListFileFunction("ctest_build", 0, 0, args);
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status) ||
        std::stoi(mf.GetDefinition("return_value")) < 0) {
      res |= cmCTest::BUILD_ERRORS;
    }
  }
  if (this->Impl->Parts[PartTest] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    this->SetCMakeVariables(mf);
    auto const func = cmListFileFunction("ctest_test", 0, 0, args);
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status) ||
        std::stoi(mf.GetDefinition("return_value")) < 0) {
      res |= cmCTest::TEST_ERRORS;
    }
  }
  if (this->Impl->Parts[PartCoverage] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    this->SetCMakeVariables(mf);
    auto const func = cmListFileFunction("ctest_coverage", 0, 0, args);
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status) ||
        std::stoi(mf.GetDefinition("return_value")) < 0) {
      res |= cmCTest::COVERAGE_ERRORS;
    }
  }
  if (this->Impl->Parts[PartMemCheck] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    this->SetCMakeVariables(mf);
    auto const func = cmListFileFunction("ctest_memcheck", 0, 0, args);
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status) ||
        std::stoi(mf.GetDefinition("return_value")) < 0) {
      res |= cmCTest::MEMORY_ERRORS;
    }
  }
  std::string notes_dir = this->Impl->BinaryDir + "/Testing/Notes";
  if (cmSystemTools::FileIsDirectory(notes_dir)) {
    cmsys::Directory d;
    d.Load(notes_dir);
    unsigned long kk;
    for (kk = 0; kk < d.GetNumberOfFiles(); kk++) {
      char const* file = d.GetFile(kk);
      std::string fullname = notes_dir + "/" + file;
      if (cmSystemTools::FileExists(fullname, true)) {
        if (!this->Impl->NotesFiles.empty()) {
          this->Impl->NotesFiles += ";";
        }
        this->Impl->NotesFiles += fullname;
        this->Impl->Parts[PartNotes].Enable();
      }
    }
  }
  if (this->Impl->Parts[PartNotes]) {
    this->UpdateCTestConfiguration();
    if (!this->Impl->NotesFiles.empty()) {
      this->GenerateNotesFile(script.GetCMake(), this->Impl->NotesFiles);
    }
  }
  if (this->Impl->Parts[PartSubmit]) {
    this->UpdateCTestConfiguration();
    this->SetCMakeVariables(mf);

    std::string count = this->GetCTestConfiguration("CTestSubmitRetryCount");
    std::string delay = this->GetCTestConfiguration("CTestSubmitRetryDelay");
    auto const func = cmListFileFunction(
      "ctest_submit", 0, 0,
      {
        cmListFileArgument("RETRY_COUNT", cmListFileArgument::Unquoted, 0),
        cmListFileArgument(count, cmListFileArgument::Quoted, 0),
        cmListFileArgument("RETRY_DELAY", cmListFileArgument::Unquoted, 0),
        cmListFileArgument(delay, cmListFileArgument::Quoted, 0),
        cmListFileArgument("RETURN_VALUE", cmListFileArgument::Unquoted, 0),
        cmListFileArgument("return_value", cmListFileArgument::Unquoted, 0),
      });
    auto status = cmExecutionStatus(mf);
    if (!mf.ExecuteCommand(func, status) ||
        std::stoi(mf.GetDefinition("return_value")) < 0) {
      res |= cmCTest::SUBMIT_ERRORS;
    }
  }
  return res;
}

std::string cmCTest::GetTestModelString() const
{
  switch (this->Impl->TestModel) {
    case cmCTest::NIGHTLY:
      return "Nightly";
    case cmCTest::CONTINUOUS:
      return "Continuous";
  }
  return "Experimental";
}

std::string cmCTest::GetTestGroupString() const
{
  if (!this->Impl->SpecificGroup.empty()) {
    return this->Impl->SpecificGroup;
  }
  return this->GetTestModelString();
}

int cmCTest::GetTestModelFromString(std::string const& str)
{
  if (str.empty()) {
    return cmCTest::EXPERIMENTAL;
  }
  std::string rstr = cmSystemTools::LowerCase(str);
  if (cmHasLiteralPrefix(rstr, "cont")) {
    return cmCTest::CONTINUOUS;
  }
  if (cmHasLiteralPrefix(rstr, "nigh")) {
    return cmCTest::NIGHTLY;
  }
  return cmCTest::EXPERIMENTAL;
}

bool cmCTest::RunMakeCommand(std::string const& command, std::string& output,
                             int* retVal, char const* dir, cmDuration timeout,
                             std::ostream& ofs, Encoding encoding)
{
  // First generate the command and arguments
  std::vector<std::string> args = cmSystemTools::ParseArguments(command);

  if (args.empty()) {
    return false;
  }

  output.clear();
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Run command:");
  for (auto const& arg : args) {
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, " \"" << arg << "\"");
  }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, std::endl);

  // Now create process object
  cmUVProcessChainBuilder builder;
  builder.AddCommand(args).SetMergedBuiltinStreams();
  if (dir) {
    builder.SetWorkingDirectory(dir);
  }
  auto chain = builder.Start();
  cm::uv_pipe_ptr outputStream;
  outputStream.init(chain.GetLoop(), 0);
  uv_pipe_open(outputStream, chain.OutputStream());

  // Initialize tick's
  std::string::size_type tick = 0;
  std::string::size_type tick_len = 1024;
  std::string::size_type tick_line_len = 50;

  cmProcessOutput processOutput(encoding);
  cmCTestLog(this, HANDLER_PROGRESS_OUTPUT,
             "   Each . represents " << tick_len
                                     << " bytes of output\n"
                                        "    "
                                     << std::flush);
  auto outputHandle = cmUVStreamRead(
    outputStream,
    [this, &processOutput, &output, &tick, &tick_len, &tick_line_len,
     &ofs](std::vector<char> data) {
      std::string strdata;
      processOutput.DecodeText(data.data(), data.size(), strdata);
      for (char& cc : strdata) {
        if (cc == 0) {
          cc = '\n';
        }
      }
      output.append(strdata);
      while (output.size() > (tick * tick_len)) {
        tick++;
        cmCTestLog(this, HANDLER_PROGRESS_OUTPUT, "." << std::flush);
        if (tick % tick_line_len == 0 && tick > 0) {
          cmCTestLog(this, HANDLER_PROGRESS_OUTPUT,
                     "  Size: " << int((double(output.size()) / 1024.0) + 1)
                                << "K\n    " << std::flush);
        }
      }
      cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, strdata);
      if (ofs) {
        ofs << strdata;
      }
    },
    [this, &processOutput, &output, &ofs]() {
      std::string strdata;
      processOutput.DecodeText(std::string(), strdata);
      if (!strdata.empty()) {
        output.append(strdata);
        cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, strdata);
        if (ofs) {
          ofs << strdata;
        }
      }
    });

  bool finished = chain.Wait(static_cast<uint64_t>(timeout.count() * 1000.0));
  cmCTestLog(this, HANDLER_PROGRESS_OUTPUT,
             " Size of output: " << int(double(output.size()) / 1024.0) << "K"
                                 << std::endl);

  if (finished) {
    auto const& status = chain.GetStatus(0);
    auto exception = status.GetException();
    switch (exception.first) {
      case cmUVProcessChain::ExceptionCode::None:
        *retVal = static_cast<int>(status.ExitStatus);
        cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
                   "Command exited with the value: " << *retVal << std::endl);
        break;
      case cmUVProcessChain::ExceptionCode::Spawn:
        output += "\n*** ERROR executing: ";
        output += exception.second;
        output += "\n***The build process failed.";
        cmCTestLog(this, ERROR_MESSAGE,
                   "There was an error: " << exception.second << std::endl);
        break;
      default:
        *retVal = static_cast<int>(exception.first);
        cmCTestLog(this, WARNING,
                   "There was an exception: " << *retVal << std::endl);
        break;
    }
  } else {
    cmCTestLog(this, WARNING, "There was a timeout" << std::endl);
  }

  return true;
}

std::string cmCTest::SafeBuildIdField(std::string const& value)
{
  std::string safevalue(value);

  if (!safevalue.empty()) {
    // Disallow non-filename and non-space whitespace characters.
    // If they occur, replace them with ""
    //
    char const* disallowed = "\\:*?\"<>|\n\r\t\f\v";

    if (safevalue.find_first_of(disallowed) != std::string::npos) {
      std::string::size_type i = 0;
      std::string::size_type n = strlen(disallowed);
      char replace[2];
      replace[1] = 0;

      for (i = 0; i < n; ++i) {
        replace[0] = disallowed[i];
        cmSystemTools::ReplaceString(safevalue, replace, "");
      }
    }
  }

  if (safevalue.empty()) {
    safevalue = "(empty)";
  }

  return safevalue;
}

void cmCTest::StartXML(cmXMLWriter& xml, cmake* cm, bool append)
{
  if (this->Impl->CurrentTag.empty()) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Current Tag empty, this may mean"
               " NightlStartTime was not set correctly."
                 << std::endl);
    cmSystemTools::SetFatalErrorOccurred();
  }

  // find out about the system
  cmsys::SystemInformation info;
  info.RunCPUCheck();
  info.RunOSCheck();
  info.RunMemoryCheck();

  std::string buildname =
    cmCTest::SafeBuildIdField(this->GetCTestConfiguration("BuildName"));
  std::string stamp = cmCTest::SafeBuildIdField(this->Impl->CurrentTag + "-" +
                                                this->GetTestGroupString());
  std::string site =
    cmCTest::SafeBuildIdField(this->GetCTestConfiguration("Site"));

  xml.StartDocument();
  xml.StartElement("Site");
  xml.Attribute("BuildName", buildname);
  xml.BreakAttributes();
  xml.Attribute("BuildStamp", stamp);
  xml.Attribute("Name", site);
  xml.Attribute("Generator",
                std::string("ctest-") + cmVersion::GetCMakeVersion());
  if (append) {
    xml.Attribute("Append", "true");
  }
  xml.Attribute("CompilerName", this->GetCTestConfiguration("Compiler"));
  xml.Attribute("CompilerVersion",
                this->GetCTestConfiguration("CompilerVersion"));
  xml.Attribute("OSName", info.GetOSName());
  xml.Attribute("Hostname", info.GetHostname());
  xml.Attribute("OSRelease", info.GetOSRelease());
  xml.Attribute("OSVersion", info.GetOSVersion());
  xml.Attribute("OSPlatform", info.GetOSPlatform());
  xml.Attribute("Is64Bits", info.Is64Bits());
  xml.Attribute("VendorString", info.GetVendorString());
  xml.Attribute("VendorID", info.GetVendorID());
  xml.Attribute("FamilyID", info.GetFamilyID());
  xml.Attribute("ModelID", info.GetModelID());
  xml.Attribute("ModelName", info.GetModelName());
  xml.Attribute("ProcessorCacheSize", info.GetProcessorCacheSize());
  xml.Attribute("NumberOfLogicalCPU", info.GetNumberOfLogicalCPU());
  xml.Attribute("NumberOfPhysicalCPU", info.GetNumberOfPhysicalCPU());
  xml.Attribute("TotalVirtualMemory", info.GetTotalVirtualMemory());
  xml.Attribute("TotalPhysicalMemory", info.GetTotalPhysicalMemory());
  xml.Attribute("LogicalProcessorsPerPhysical",
                info.GetLogicalProcessorsPerPhysical());
  xml.Attribute("ProcessorClockFrequency", info.GetProcessorClockFrequency());

  std::string changeId = this->GetCTestConfiguration("ChangeId");
  if (!changeId.empty()) {
    xml.Attribute("ChangeId", changeId);
  }

  this->AddSiteProperties(xml, cm);
}

void cmCTest::AddSiteProperties(cmXMLWriter& xml, cmake* cm)
{
  // This code should go when cdash is changed to use labels only
  cmValue subproject = cm->GetState()->GetGlobalProperty("SubProject");
  if (subproject) {
    xml.StartElement("Subproject");
    xml.Attribute("name", *subproject);
    cmValue labels = cm->GetState()->GetGlobalProperty("SubProjectLabels");
    if (labels) {
      xml.StartElement("Labels");
      cmList args{ *labels };
      for (std::string const& i : args) {
        xml.Element("Label", i);
      }
      xml.EndElement();
    }
    xml.EndElement();
  }

  // This code should stay when cdash only does label based sub-projects
  cmValue label = cm->GetState()->GetGlobalProperty("Label");
  if (label) {
    xml.StartElement("Labels");
    xml.Element("Label", *label);
    xml.EndElement();
  }
}

void cmCTest::GenerateSubprojectsOutput(cmXMLWriter& xml)
{
  for (std::string const& subproj : this->GetLabelsForSubprojects()) {
    xml.StartElement("Subproject");
    xml.Attribute("name", subproj);
    xml.Element("Label", subproj);
    xml.EndElement(); // Subproject
  }
}

std::vector<std::string> cmCTest::GetLabelsForSubprojects()
{
  std::string labelsForSubprojects =
    this->GetCTestConfiguration("LabelsForSubprojects");
  cmList subprojects{ labelsForSubprojects };

  // sort the array
  std::sort(subprojects.begin(), subprojects.end());
  // remove duplicates
  auto new_end = std::unique(subprojects.begin(), subprojects.end());
  subprojects.erase(new_end, subprojects.end());

  return std::move(subprojects.data());
}

void cmCTest::EndXML(cmXMLWriter& xml)
{
  xml.EndElement(); // Site
  xml.EndDocument();
}

int cmCTest::GenerateCTestNotesOutput(cmXMLWriter& xml, cmake* cm,
                                      std::vector<std::string> const& files)
{
  std::string buildname =
    cmCTest::SafeBuildIdField(this->GetCTestConfiguration("BuildName"));
  xml.StartDocument();
  xml.ProcessingInstruction("xml-stylesheet",
                            "type=\"text/xsl\" "
                            "href=\"Dart/Source/Server/XSL/Build.xsl "
                            "<file:///Dart/Source/Server/XSL/Build.xsl> \"");
  xml.StartElement("Site");
  xml.Attribute("BuildName", buildname);
  xml.Attribute("BuildStamp",
                this->Impl->CurrentTag + "-" + this->GetTestGroupString());
  xml.Attribute("Name", this->GetCTestConfiguration("Site"));
  xml.Attribute("Generator",
                std::string("ctest-") + cmVersion::GetCMakeVersion());
  this->AddSiteProperties(xml, cm);
  xml.StartElement("Notes");

  for (std::string const& file : files) {
    cmCTestLog(this, OUTPUT, "\tAdd file: " << file << std::endl);
    std::string note_time = this->CurrentTime();
    xml.StartElement("Note");
    xml.Attribute("Name", file);
    xml.Element("Time", std::chrono::system_clock::now());
    xml.Element("DateTime", note_time);
    xml.StartElement("Text");
    cmsys::ifstream ifs(file.c_str());
    if (ifs) {
      std::string line;
      while (cmSystemTools::GetLineFromStream(ifs, line)) {
        xml.Content(line);
        xml.Content("\n");
      }
      ifs.close();
    } else {
      xml.Content("Problem reading file: " + file + "\n");
      cmCTestLog(this, ERROR_MESSAGE,
                 "Problem reading file: " << file << " while creating notes"
                                          << std::endl);
    }
    xml.EndElement(); // Text
    xml.EndElement(); // Note
  }
  xml.EndElement(); // Notes
  xml.EndElement(); // Site
  xml.EndDocument();
  return 1;
}

int cmCTest::GenerateNotesFile(cmake* cm,
                               std::vector<std::string> const& files)
{
  cmGeneratedFileStream ofs;
  if (!this->OpenOutputFile(this->Impl->CurrentTag, "Notes.xml", ofs)) {
    cmCTestLog(this, ERROR_MESSAGE, "Cannot open notes file" << std::endl);
    return 1;
  }
  cmXMLWriter xml(ofs);
  this->GenerateCTestNotesOutput(xml, cm, files);
  return 0;
}

int cmCTest::GenerateNotesFile(cmake* cm, std::string const& cfiles)
{
  if (cfiles.empty()) {
    return 1;
  }

  cmCTestLog(this, OUTPUT, "Create notes file" << std::endl);

  std::vector<std::string> const files =
    cmSystemTools::SplitString(cfiles, ';');
  if (files.empty()) {
    return 1;
  }

  return this->GenerateNotesFile(cm, files);
}

int cmCTest::GenerateDoneFile()
{
  cmGeneratedFileStream ofs;
  if (!this->OpenOutputFile(this->Impl->CurrentTag, "Done.xml", ofs)) {
    cmCTestLog(this, ERROR_MESSAGE, "Cannot open done file" << std::endl);
    return 1;
  }
  cmXMLWriter xml(ofs);
  xml.StartDocument();
  xml.StartElement("Done");
  xml.Element("buildId", this->Impl->BuildID);
  xml.Element("time", std::chrono::system_clock::now());
  xml.EndElement(); // Done
  xml.EndDocument();

  return 0;
}

std::string cmCTest::Base64GzipEncodeFile(std::string const& file)
{
  // Temporarily change to the file's directory so the tar gets created
  // with a flat directory structure.
  cmWorkingDirectory workdir(cmSystemTools::GetParentDirectory(file));
  if (workdir.Failed()) {
    cmCTestLog(this, ERROR_MESSAGE, workdir.GetError() << std::endl);
    return "";
  }

  std::string tarFile = file + "_temp.tar.gz";
  std::vector<std::string> files;
  files.push_back(file);

  if (!cmSystemTools::CreateTar(tarFile, files, {},
                                cmSystemTools::TarCompressGZip, false)) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Error creating tar while "
               "encoding file: "
                 << file << std::endl);
    return "";
  }
  std::string base64 = this->Base64EncodeFile(tarFile);
  cmSystemTools::RemoveFile(tarFile);
  return base64;
}

std::string cmCTest::Base64EncodeFile(std::string const& file)
{
  size_t const len = cmSystemTools::FileLength(file);
  cmsys::ifstream ifs(file.c_str(),
                      std::ios::in
#ifdef _WIN32
                        | std::ios::binary
#endif
  );
  std::vector<char> file_buffer(len + 1);
  ifs.read(file_buffer.data(), len);
  ifs.close();

  std::vector<char> encoded_buffer((len * 3) / 2 + 5);

  size_t const rlen = cmsysBase64_Encode(
    reinterpret_cast<unsigned char*>(file_buffer.data()), len,
    reinterpret_cast<unsigned char*>(encoded_buffer.data()), 1);

  return std::string(encoded_buffer.data(), rlen);
}

bool cmCTest::SubmitExtraFiles(std::vector<std::string> const& files)
{
  for (std::string const& file : files) {
    if (!cmSystemTools::FileExists(file)) {
      cmCTestLog(this, ERROR_MESSAGE,
                 "Cannot find extra file: " << file << " to submit."
                                            << std::endl);
      return false;
    }
    this->AddSubmitFile(PartExtraFiles, file);
  }
  return true;
}

bool cmCTest::SubmitExtraFiles(std::string const& cfiles)
{
  if (cfiles.empty()) {
    return true;
  }

  cmCTestLog(this, OUTPUT, "Submit extra files" << std::endl);

  std::vector<std::string> const files =
    cmSystemTools::SplitString(cfiles, ';');
  if (files.empty()) {
    return true;
  }

  return this->SubmitExtraFiles(files);
}

// for a -D argument convert the next argument into
// the proper list of dashboard steps via SetTest
bool cmCTest::AddTestsForDashboardType(std::string const& targ)
{
  if (targ == "Experimental") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Start");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("Test");
    this->SetTest("Coverage");
    this->SetTest("Submit");
  } else if (targ == "ExperimentalStart") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Start");
  } else if (targ == "ExperimentalUpdate") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Update");
  } else if (targ == "ExperimentalConfigure") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Configure");
  } else if (targ == "ExperimentalBuild") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Build");
  } else if (targ == "ExperimentalTest") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Test");
  } else if (targ == "ExperimentalMemCheck" || targ == "ExperimentalPurify") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("MemCheck");
  } else if (targ == "ExperimentalCoverage") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Coverage");
  } else if (targ == "ExperimentalSubmit") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Submit");
  } else if (targ == "Continuous") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Start");
    this->SetTest("Update");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("Test");
    this->SetTest("Coverage");
    this->SetTest("Submit");
  } else if (targ == "ContinuousStart") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Start");
  } else if (targ == "ContinuousUpdate") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Update");
  } else if (targ == "ContinuousConfigure") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Configure");
  } else if (targ == "ContinuousBuild") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Build");
  } else if (targ == "ContinuousTest") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Test");
  } else if (targ == "ContinuousMemCheck" || targ == "ContinuousPurify") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("MemCheck");
  } else if (targ == "ContinuousCoverage") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Coverage");
  } else if (targ == "ContinuousSubmit") {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Submit");
  } else if (targ == "Nightly") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Start");
    this->SetTest("Update");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("Test");
    this->SetTest("Coverage");
    this->SetTest("Submit");
  } else if (targ == "NightlyStart") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Start");
  } else if (targ == "NightlyUpdate") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Update");
  } else if (targ == "NightlyConfigure") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Configure");
  } else if (targ == "NightlyBuild") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Build");
  } else if (targ == "NightlyTest") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Test");
  } else if (targ == "NightlyMemCheck" || targ == "NightlyPurify") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("MemCheck");
  } else if (targ == "NightlyCoverage") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Coverage");
  } else if (targ == "NightlySubmit") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Submit");
  } else if (targ == "MemoryCheck") {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Start");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("MemCheck");
    this->SetTest("Coverage");
    this->SetTest("Submit");
  } else if (targ == "NightlyMemoryCheck") {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Start");
    this->SetTest("Update");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("MemCheck");
    this->SetTest("Coverage");
    this->SetTest("Submit");
  } else {
    return false;
  }
  return true;
}

void cmCTest::ErrorMessageUnknownDashDValue(std::string const& val)
{
  cmCTestLog(this, ERROR_MESSAGE,
             "CTest -D called with incorrect option: " << val << '\n');

  cmCTestLog(this, ERROR_MESSAGE,
             "Available options are:\n"
             "  ctest -D Continuous\n"
             "  ctest -D Continuous(Start|Update|Configure|Build)\n"
             "  ctest -D Continuous(Test|Coverage|MemCheck|Submit)\n"
             "  ctest -D Experimental\n"
             "  ctest -D Experimental(Start|Update|Configure|Build)\n"
             "  ctest -D Experimental(Test|Coverage|MemCheck|Submit)\n"
             "  ctest -D Nightly\n"
             "  ctest -D Nightly(Start|Update|Configure|Build)\n"
             "  ctest -D Nightly(Test|Coverage|MemCheck|Submit)\n"
             "  ctest -D NightlyMemoryCheck\n");
}

bool cmCTest::CheckArgument(std::string const& arg, cm::string_view varg1,
                            char const* varg2)
{
  return (arg == varg1) || (varg2 && arg == varg2);
}

#if !defined(_WIN32)
bool cmCTest::ConsoleIsNotDumb()
{
  std::string term_env_variable;
  if (cmSystemTools::GetEnv("TERM", term_env_variable)) {
    return isatty(1) && term_env_variable != "dumb";
  }
  return false;
}
#endif

bool cmCTest::ProgressOutputSupportedByConsole()
{
#if defined(_WIN32)
  // On Windows we need a console buffer.
  void* console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  return GetConsoleScreenBufferInfo(console, &csbi);
#else
  // On UNIX we need a non-dumb tty.
  return ConsoleIsNotDumb();
#endif
}

bool cmCTest::ColoredOutputSupportedByConsole()
{
  std::string clicolor_force;
  if (cmSystemTools::GetEnv("CLICOLOR_FORCE", clicolor_force) &&
      !clicolor_force.empty() && clicolor_force != "0") {
    return true;
  }
  std::string clicolor;
  if (cmSystemTools::GetEnv("CLICOLOR", clicolor) && clicolor == "0") {
    return false;
  }
#if defined(_WIN32)
  // Not supported on Windows
  return false;
#else
  // On UNIX we need a non-dumb tty.
  return ConsoleIsNotDumb();
#endif
}

bool cmCTest::AddVariableDefinition(std::string const& arg)
{
  std::string name;
  std::string value;
  cmStateEnums::CacheEntryType type = cmStateEnums::UNINITIALIZED;

  if (cmake::ParseCacheEntry(arg, name, value, type)) {
    this->Impl->Definitions[name] = value;
    return true;
  }

  return false;
}

bool cmCTest::SetArgsFromPreset(std::string const& presetName,
                                bool listPresets)
{
  auto const workingDirectory = cmSystemTools::GetLogicalWorkingDirectory();

  cmCMakePresetsGraph settingsFile;
  auto result = settingsFile.ReadProjectPresets(workingDirectory);
  if (result != true) {
    cmSystemTools::Error(cmStrCat("Could not read presets from ",
                                  workingDirectory, ":",
                                  settingsFile.parseState.GetErrorMessage()));
    return false;
  }

  if (listPresets) {
    settingsFile.PrintTestPresetList();
    return true;
  }

  auto presetPair = settingsFile.TestPresets.find(presetName);
  if (presetPair == settingsFile.TestPresets.end()) {
    cmSystemTools::Error(cmStrCat("No such test preset in ", workingDirectory,
                                  ": \"", presetName, '"'));
    settingsFile.PrintTestPresetList();
    return false;
  }

  if (presetPair->second.Unexpanded.Hidden) {
    cmSystemTools::Error(cmStrCat("Cannot use hidden test preset in ",
                                  workingDirectory, ": \"", presetName, '"'));
    settingsFile.PrintTestPresetList();
    return false;
  }

  auto const& expandedPreset = presetPair->second.Expanded;
  if (!expandedPreset) {
    cmSystemTools::Error(cmStrCat("Could not evaluate test preset \"",
                                  presetName, "\": Invalid macro expansion"));
    settingsFile.PrintTestPresetList();
    return false;
  }

  if (!expandedPreset->ConditionResult) {
    cmSystemTools::Error(cmStrCat("Cannot use disabled test preset in ",
                                  workingDirectory, ": \"", presetName, '"'));
    settingsFile.PrintTestPresetList();
    return false;
  }

  auto configurePresetPair =
    settingsFile.ConfigurePresets.find(expandedPreset->ConfigurePreset);
  if (configurePresetPair == settingsFile.ConfigurePresets.end()) {
    cmSystemTools::Error(cmStrCat("No such configure preset in ",
                                  workingDirectory, ": \"",
                                  expandedPreset->ConfigurePreset, '"'));
    settingsFile.PrintConfigurePresetList();
    return false;
  }

  if (configurePresetPair->second.Unexpanded.Hidden) {
    cmSystemTools::Error(cmStrCat("Cannot use hidden configure preset in ",
                                  workingDirectory, ": \"",
                                  expandedPreset->ConfigurePreset, '"'));
    settingsFile.PrintConfigurePresetList();
    return false;
  }

  auto const& expandedConfigurePreset = configurePresetPair->second.Expanded;
  if (!expandedConfigurePreset) {
    cmSystemTools::Error(cmStrCat("Could not evaluate configure preset \"",
                                  expandedPreset->ConfigurePreset,
                                  "\": Invalid macro expansion"));
    return false;
  }

  auto presetEnvironment = expandedPreset->Environment;
  for (auto const& var : presetEnvironment) {
    if (var.second) {
      cmSystemTools::PutEnv(cmStrCat(var.first, '=', *var.second));
    }
  }

  if (!expandedPreset->Configuration.empty()) {
    this->SetConfigType(expandedPreset->Configuration);
  }

  // Set build directory to value specified by the configure preset.
  this->AddCTestConfigurationOverwrite(
    cmStrCat("BuildDirectory=", expandedConfigurePreset->BinaryDir));
  for (auto const& kvp : expandedPreset->OverwriteConfigurationFile) {
    this->AddCTestConfigurationOverwrite(kvp);
  }

  if (expandedPreset->Output) {
    this->Impl->TestProgressOutput =
      expandedPreset->Output->ShortProgress.value_or(false);

    if (expandedPreset->Output->Verbosity) {
      auto const& verbosity = *expandedPreset->Output->Verbosity;
      switch (verbosity) {
        case cmCMakePresetsGraph::TestPreset::OutputOptions::VerbosityEnum::
          Extra:
          this->Impl->ExtraVerbose = true;
          CM_FALLTHROUGH;
        case cmCMakePresetsGraph::TestPreset::OutputOptions::VerbosityEnum::
          Verbose:
          this->Impl->Verbose = true;
          break;
        case cmCMakePresetsGraph::TestPreset::OutputOptions::VerbosityEnum::
          Default:
        default:
          // leave default settings
          break;
      }
    }

    this->Impl->Debug = expandedPreset->Output->Debug.value_or(false);
    this->Impl->OutputTestOutputOnTestFailure =
      expandedPreset->Output->OutputOnFailure.value_or(false);
    this->Impl->Quiet = expandedPreset->Output->Quiet.value_or(false);

    if (!expandedPreset->Output->OutputLogFile.empty()) {
      this->SetOutputLogFileName(expandedPreset->Output->OutputLogFile);
    }
    if (!expandedPreset->Output->OutputJUnitFile.empty()) {
      this->SetOutputJUnitFileName(expandedPreset->Output->OutputJUnitFile);
    }

    this->Impl->LabelSummary =
      expandedPreset->Output->LabelSummary.value_or(true);
    this->Impl->SubprojectSummary =
      expandedPreset->Output->SubprojectSummary.value_or(true);

    if (expandedPreset->Output->MaxPassedTestOutputSize) {
      this->Impl->TestOptions.OutputSizePassed =
        *expandedPreset->Output->MaxPassedTestOutputSize;
    }

    if (expandedPreset->Output->MaxFailedTestOutputSize) {
      this->Impl->TestOptions.OutputSizeFailed =
        *expandedPreset->Output->MaxFailedTestOutputSize;
    }

    if (expandedPreset->Output->TestOutputTruncation) {
      this->Impl->TestOptions.OutputTruncation =
        *expandedPreset->Output->TestOutputTruncation;
    }

    if (expandedPreset->Output->MaxTestNameWidth) {
      this->Impl->MaxTestNameWidth = *expandedPreset->Output->MaxTestNameWidth;
    }
  }

  if (expandedPreset->Filter) {
    if (expandedPreset->Filter->Include) {
      this->Impl->TestOptions.IncludeRegularExpression =
        expandedPreset->Filter->Include->Name;
      if (!expandedPreset->Filter->Include->Label.empty()) {
        this->Impl->TestOptions.LabelRegularExpression.push_back(
          expandedPreset->Filter->Include->Label);
      }

      if (expandedPreset->Filter->Include->Index) {
        if (expandedPreset->Filter->Include->Index->IndexFile.empty()) {
          auto const& start = expandedPreset->Filter->Include->Index->Start;
          auto const& end = expandedPreset->Filter->Include->Index->End;
          auto const& stride = expandedPreset->Filter->Include->Index->Stride;
          std::string indexOptions;
          indexOptions += (start ? std::to_string(*start) : "") + ",";
          indexOptions += (end ? std::to_string(*end) : "") + ",";
          indexOptions += (stride ? std::to_string(*stride) : "") + ",";
          indexOptions +=
            cmJoin(expandedPreset->Filter->Include->Index->SpecificTests, ",");

          this->Impl->TestOptions.TestsToRunInformation = indexOptions;
        } else {
          this->Impl->TestOptions.TestsToRunInformation =
            expandedPreset->Filter->Include->Index->IndexFile;
        }
      }

      this->Impl->TestOptions.UseUnion =
        expandedPreset->Filter->Include->UseUnion.value_or(false);
    }

    if (expandedPreset->Filter->Exclude) {
      this->Impl->TestOptions.ExcludeRegularExpression =
        expandedPreset->Filter->Exclude->Name;
      if (!expandedPreset->Filter->Exclude->Label.empty()) {
        this->Impl->TestOptions.ExcludeLabelRegularExpression.push_back(
          expandedPreset->Filter->Exclude->Label);
      }

      if (expandedPreset->Filter->Exclude->Fixtures) {
        this->Impl->TestOptions.ExcludeFixtureRegularExpression =
          expandedPreset->Filter->Exclude->Fixtures->Any;
        this->Impl->TestOptions.ExcludeFixtureSetupRegularExpression =
          expandedPreset->Filter->Exclude->Fixtures->Setup;
        this->Impl->TestOptions.ExcludeFixtureCleanupRegularExpression =
          expandedPreset->Filter->Exclude->Fixtures->Cleanup;
      }
    }
  }

  if (expandedPreset->Execution) {
    this->Impl->StopOnFailure =
      expandedPreset->Execution->StopOnFailure.value_or(false);
    this->Impl->Failover =
      expandedPreset->Execution->EnableFailover.value_or(false);

    if (expandedPreset->Execution->Jobs) {
      auto jobs = *expandedPreset->Execution->Jobs;
      this->SetParallelLevel(jobs);
      this->Impl->ParallelLevelSetInCli = true;
    }

    this->Impl->TestOptions.ResourceSpecFile =
      expandedPreset->Execution->ResourceSpecFile;

    if (expandedPreset->Execution->TestLoad) {
      auto testLoad = *expandedPreset->Execution->TestLoad;
      this->SetTestLoad(testLoad);
    }

    if (expandedPreset->Execution->ShowOnly) {
      this->Impl->ShowOnly = true;

      switch (*expandedPreset->Execution->ShowOnly) {
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::ShowOnlyEnum::
          JsonV1:
          this->Impl->Quiet = true;
          this->Impl->OutputAsJson = true;
          this->Impl->OutputAsJsonVersion = 1;
          break;
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::ShowOnlyEnum::
          Human:
          // intentional fallthrough (human is the default)
        default:
          break;
      }
    }

    if (expandedPreset->Execution->Repeat) {
      this->Impl->RepeatCount = expandedPreset->Execution->Repeat->Count;
      switch (expandedPreset->Execution->Repeat->Mode) {
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::RepeatOptions::
          ModeEnum::UntilFail:
          this->Impl->RepeatMode = cmCTest::Repeat::UntilFail;
          break;
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::RepeatOptions::
          ModeEnum::UntilPass:
          this->Impl->RepeatMode = cmCTest::Repeat::UntilPass;
          break;
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::RepeatOptions::
          ModeEnum::AfterTimeout:
          this->Impl->RepeatMode = cmCTest::Repeat::AfterTimeout;
          break;
        default:
          // should never default since mode is required
          return false;
      }
    }

    if (expandedPreset->Execution->InteractiveDebugging) {
      this->Impl->InteractiveDebugMode =
        *expandedPreset->Execution->InteractiveDebugging;
    }

    if (expandedPreset->Execution->ScheduleRandom.value_or(false)) {
      this->Impl->ScheduleType = "Random";
    }

    if (expandedPreset->Execution->Timeout) {
      this->Impl->GlobalTimeout =
        cmDuration(*expandedPreset->Execution->Timeout);
    }

    if (expandedPreset->Execution->NoTestsAction) {
      switch (*expandedPreset->Execution->NoTestsAction) {
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::
          NoTestsActionEnum::Error:
          this->Impl->NoTestsMode = cmCTest::NoTests::Error;
          break;
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::
          NoTestsActionEnum::Ignore:
          this->Impl->NoTestsMode = cmCTest::NoTests::Ignore;
          break;
        case cmCMakePresetsGraph::TestPreset::ExecutionOptions::
          NoTestsActionEnum::Default:
          break;
        default:
          // should never default
          return false;
      }
    }
  }

  return true;
}

// the main entry point of ctest, called from main
int cmCTest::Run(std::vector<std::string> const& args)
{
  char const* ctestExec = "ctest";
  bool cmakeAndTest = false;
  bool processSteps = false;
  bool SRArgumentSpecified = false;
  std::vector<std::pair<std::string, bool>> runScripts;

  // copy the command line
  cm::append(this->Impl->InitialCommandLineArguments, args);

  // check if a test preset was specified

  bool listPresets =
    find(args.begin(), args.end(), "--list-presets") != args.end();
  auto it =
    std::find_if(args.begin(), args.end(), [](std::string const& arg) -> bool {
      return arg == "--preset" || cmHasLiteralPrefix(arg, "--preset=");
    });
  if (listPresets || it != args.end()) {
    std::string errormsg;
    bool success;

    if (listPresets) {
      // If listing presets we don't need a presetName
      success = this->SetArgsFromPreset("", listPresets);
    } else {
      if (cmHasLiteralPrefix(*it, "--preset=")) {
        auto const& presetName = it->substr(9);
        success = this->SetArgsFromPreset(presetName, listPresets);
      } else if (++it != args.end()) {
        auto const& presetName = *it;
        success = this->SetArgsFromPreset(presetName, listPresets);
      } else {
        cmSystemTools::Error("'--preset' requires an argument");
        success = false;
      }
    }

    if (listPresets) {
      return success ? 0 : 1;
    }

    if (!success) {
      return 1;
    }
  }

  auto const dashD = [this, &processSteps](std::string const& targ) -> bool {
    // AddTestsForDashboard parses the dashboard type and converts it
    // into the separate stages
    if (this->AddTestsForDashboardType(targ)) {
      processSteps = true;
      return true;
    }
    if (this->AddVariableDefinition(targ)) {
      return true;
    }
    this->ErrorMessageUnknownDashDValue(targ);
    return false;
  };
  auto const dashT = [this, &processSteps,
                      ctestExec](std::string const& action) -> bool {
    if (!this->SetTest(action, false)) {
      cmCTestLog(this, ERROR_MESSAGE,
                 "CTest -T called with incorrect option: " << action << '\n');
      /* clang-format off */
      cmCTestLog(this, ERROR_MESSAGE,
                 "Available options are:\n"
                 "  " << ctestExec << " -T all\n"
                 "  " << ctestExec << " -T start\n"
                 "  " << ctestExec << " -T update\n"
                 "  " << ctestExec << " -T configure\n"
                 "  " << ctestExec << " -T build\n"
                 "  " << ctestExec << " -T test\n"
                 "  " << ctestExec << " -T coverage\n"
                 "  " << ctestExec << " -T memcheck\n"
                 "  " << ctestExec << " -T notes\n"
                 "  " << ctestExec << " -T submit\n");
      /* clang-format on */
      return false;
    }
    processSteps = true;
    return true;
  };
  auto const dashM = [this, &processSteps,
                      ctestExec](std::string const& model) -> bool {
    if (cmSystemTools::LowerCase(model) == "nightly"_s) {
      this->SetTestModel(cmCTest::NIGHTLY);
    } else if (cmSystemTools::LowerCase(model) == "continuous"_s) {
      this->SetTestModel(cmCTest::CONTINUOUS);
    } else if (cmSystemTools::LowerCase(model) == "experimental"_s) {
      this->SetTestModel(cmCTest::EXPERIMENTAL);
    } else {
      cmCTestLog(this, ERROR_MESSAGE,
                 "CTest -M called with incorrect option: " << model << '\n');
      /* clang-format off */
           cmCTestLog(this, ERROR_MESSAGE,
                      "Available options are:\n"
                      "  " << ctestExec << " -M Continuous\n"
                      "  " << ctestExec << " -M Experimental\n"
                      "  " << ctestExec << " -M Nightly\n");
      /* clang-format on */
      return false;
    }
    processSteps = true;
    return true;
  };
  auto const dashSP =
    [&runScripts, &SRArgumentSpecified](std::string const& script) -> bool {
    // -SR is an internal argument, -SP should be ignored when it is passed
    if (!SRArgumentSpecified) {
      runScripts.emplace_back(cmSystemTools::ToNormalizedPathOnDisk(script),
                              false);
    }
    return true;
  };
  auto const dashSR =
    [&runScripts, &SRArgumentSpecified](std::string const& script) -> bool {
    // -SR should be processed only once
    if (!SRArgumentSpecified) {
      SRArgumentSpecified = true;
      runScripts.emplace_back(cmSystemTools::ToNormalizedPathOnDisk(script),
                              true);
    }
    return true;
  };
  auto const dash_S =
    [&runScripts, &SRArgumentSpecified](std::string const& script) -> bool {
    // -SR is an internal argument, -S should be ignored when it is passed
    if (!SRArgumentSpecified) {
      runScripts.emplace_back(cmSystemTools::ToNormalizedPathOnDisk(script),
                              true);
    }
    return true;
  };
  auto const dashJ = [this](cm::string_view arg,
                            std::string const& j) -> bool {
    cm::optional<size_t> parallelLevel;
    // No value or an empty value tells ctest to choose a default.
    if (!j.empty()) {
      // A non-empty value must be a non-negative integer.
      unsigned long plevel = 0;
      if (!cmStrToULong(j, &plevel)) {
        cmSystemTools::Error(
          cmStrCat('\'', arg, "' given invalid value '", j, '\''));
        return false;
      }
      parallelLevel = plevel;
    }
    this->SetParallelLevel(parallelLevel);
    this->Impl->ParallelLevelSetInCli = true;
    return true;
  };
  auto const dashC = [this](std::string const& config) -> bool {
    this->SetConfigType(config);
    return true;
  };
  auto const dashGroup = [this](std::string const& group) -> bool {
    this->Impl->SpecificGroup = group;
    return true;
  };
  auto const dashQ = [this](std::string const&) -> bool {
    this->Impl->Quiet = true;
    return true;
  };
  auto const dashV = [this](std::string const&) -> bool {
    this->Impl->Verbose = true;
    return true;
  };
  auto const dashVV = [this](std::string const&) -> bool {
    this->Impl->ExtraVerbose = true;
    this->Impl->Verbose = true;
    return true;
  };
  auto const dashO = [this](std::string const& log) -> bool {
    this->SetOutputLogFileName(log);
    return true;
  };
  auto const dashW = [this](std::string const& width) -> bool {
    this->Impl->MaxTestNameWidth = atoi(width.c_str());
    return true;
  };
  auto const dashA = [this, &processSteps](std::string const& notes) -> bool {
    processSteps = true;
    this->SetTest("Notes");
    this->SetNotesFiles(notes);
    return true;
  };
  auto const dashI = [this](std::string const& tests) -> bool {
    this->Impl->TestOptions.TestsToRunInformation = tests;
    return true;
  };
  auto const dashU = [this](std::string const&) -> bool {
    this->Impl->TestOptions.UseUnion = true;
    return true;
  };
  auto const dashR = [this](std::string const& expr) -> bool {
    this->Impl->TestOptions.IncludeRegularExpression = expr;
    return true;
  };
  auto const dashE = [this](std::string const& expr) -> bool {
    this->Impl->TestOptions.ExcludeRegularExpression = expr;
    return true;
  };
  auto const dashL = [this](std::string const& expr) -> bool {
    this->Impl->TestOptions.LabelRegularExpression.push_back(expr);
    return true;
  };
  auto const dashLE = [this](std::string const& expr) -> bool {
    this->Impl->TestOptions.ExcludeLabelRegularExpression.push_back(expr);
    return true;
  };
  auto const dashFA = [this](std::string const& expr) -> bool {
    this->Impl->TestOptions.ExcludeFixtureRegularExpression = expr;
    return true;
  };
  auto const dashFS = [this](std::string const& expr) -> bool {
    this->Impl->TestOptions.ExcludeFixtureSetupRegularExpression = expr;
    return true;
  };
  auto const dashFC = [this](std::string const& expr) -> bool {
    this->Impl->TestOptions.ExcludeFixtureCleanupRegularExpression = expr;
    return true;
  };

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value)>;

  auto const arguments = std::vector<CommandArgument>{
    CommandArgument{ "--dashboard", CommandArgument::Values::One, dashD },
    CommandArgument{ "-D",
                     "-D must be followed by dashboard mode or VAR=VALUE.",
                     CommandArgument::Values::One, dashD },
    CommandArgument{
      "-D", "-D must be followed by dashboard mode or VAR=VALUE.",
      CommandArgument::Values::One, CommandArgument::RequiresSeparator::No,
      [this](std::string const& def) -> bool {
        // Unsuccessful parsing of VAR=VALUE has historically
        // been ignored.
        this->AddVariableDefinition(def);
        return true;
      } },
    CommandArgument{ "-T", CommandArgument::Values::One, dashT },
    CommandArgument{ "--test-action", CommandArgument::Values::One, dashT },
    CommandArgument{ "-M", CommandArgument::Values::One, dashM },
    CommandArgument{ "--test-model", CommandArgument::Values::One, dashM },
    CommandArgument{ "--extra-submit", CommandArgument::Values::One,
                     [this, &processSteps](std::string const& extra) -> bool {
                       processSteps = true;
                       this->SetTest("Submit");
                       return this->SubmitExtraFiles(extra);
                     } },
    CommandArgument{
      "--build-and-test", "--build-and-test must have source and binary dir",
      CommandArgument::Values::Two,
      [this, &cmakeAndTest](std::string const& dirs) -> bool {
        cmakeAndTest = true;
        cmList dirList{ dirs };
        if (dirList.size() != 2) {
          return false;
        }
        this->Impl->BuildAndTest.SourceDir =
          cmSystemTools::ToNormalizedPathOnDisk(dirList[0]);
        this->Impl->BuildAndTest.BinaryDir =
          cmSystemTools::ToNormalizedPathOnDisk(dirList[1]);
        cmSystemTools::MakeDirectory(this->Impl->BuildAndTest.BinaryDir);
        return true;
      } },
    CommandArgument{ "--build-target", CommandArgument::Values::One,
                     [this](std::string const& t) -> bool {
                       this->Impl->BuildAndTest.BuildTargets.emplace_back(t);
                       return true;
                     } },
    CommandArgument{ "--build-noclean", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->BuildAndTest.BuildNoClean = true;
                       return true;
                     } },
    CommandArgument{ "--build-nocmake", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->BuildAndTest.BuildNoCMake = true;
                       return true;
                     } },
    CommandArgument{ "--build-two-config", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->BuildAndTest.BuildTwoConfig = true;
                       return true;
                     } },
    CommandArgument{ "--build-run-dir", CommandArgument::Values::One,
                     [this](std::string const& dir) -> bool {
                       this->Impl->BuildAndTest.BuildRunDir = dir;
                       return true;
                     } },
    CommandArgument{ "--build-exe-dir", CommandArgument::Values::One,
                     [this](std::string const& dir) -> bool {
                       this->Impl->BuildAndTest.ExecutableDirectory = dir;
                       return true;
                     } },
    CommandArgument{ "--test-timeout", CommandArgument::Values::One,
                     [this](std::string const& t) -> bool {
                       this->Impl->BuildAndTest.Timeout =
                         cmDuration(atof(t.c_str()));
                       return true;
                     } },
    CommandArgument{ "--build-generator", CommandArgument::Values::One,
                     [this](std::string const& g) -> bool {
                       this->Impl->BuildAndTest.BuildGenerator = g;
                       return true;
                     } },
    CommandArgument{ "--build-generator-platform",
                     CommandArgument::Values::One,
                     [this](std::string const& p) -> bool {
                       this->Impl->BuildAndTest.BuildGeneratorPlatform = p;
                       return true;
                     } },
    CommandArgument{ "--build-generator-toolset", CommandArgument::Values::One,
                     [this](std::string const& t) -> bool {
                       this->Impl->BuildAndTest.BuildGeneratorToolset = t;
                       return true;
                     } },
    CommandArgument{ "--build-project", CommandArgument::Values::One,
                     [this](std::string const& p) -> bool {
                       this->Impl->BuildAndTest.BuildProject = p;
                       return true;
                     } },
    CommandArgument{ "--build-makeprogram", CommandArgument::Values::One,
                     [this](std::string const& p) -> bool {
                       this->Impl->BuildAndTest.BuildMakeProgram = p;
                       return true;
                     } },
    CommandArgument{ "--build-config-sample", CommandArgument::Values::One,
                     [this](std::string const& s) -> bool {
                       this->Impl->BuildAndTest.ConfigSample = s;
                       return true;
                     } },
    CommandArgument{ "-SP", CommandArgument::Values::One, dashSP },
    CommandArgument{ "--script-new-process", CommandArgument::Values::One,
                     dashSP },
    CommandArgument{ "-SR", CommandArgument::Values::One, dashSR },
    CommandArgument{ "--script-run", CommandArgument::Values::One, dashSR },
    CommandArgument{ "-S", CommandArgument::Values::One, dash_S },
    CommandArgument{ "--script", CommandArgument::Values::One, dash_S },
    CommandArgument{ "-F", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->Failover = true;
                       return true;
                     } },
    CommandArgument{
      "-j", CommandArgument::Values::ZeroOrOne,
      [&dashJ](std::string const& j) -> bool { return dashJ("-j"_s, j); } },
    CommandArgument{ "--parallel", CommandArgument::Values::ZeroOrOne,
                     [&dashJ](std::string const& j) -> bool {
                       return dashJ("--parallel"_s, j);
                     } },
    CommandArgument{ "-j", CommandArgument::Values::One,
                     CommandArgument::RequiresSeparator::No,
                     [this](std::string const& j) -> bool {
                       // The value must be a non-negative integer.
                       unsigned long plevel = 0;
                       if (!cmStrToULong(j, &plevel)) {
                         cmSystemTools::Error(
                           cmStrCat("'-j' given invalid value '", j, '\''));
                         return false;
                       }
                       this->SetParallelLevel(plevel);
                       this->Impl->ParallelLevelSetInCli = true;
                       return true;
                     } },
    CommandArgument{
      "--repeat-until-fail", "'--repeat-until-fail' requires an argument",
      CommandArgument::Values::One,
      [this](std::string const& r) -> bool {
        if (this->Impl->RepeatMode != cmCTest::Repeat::Never) {
          cmSystemTools::Error("At most one '--repeat' option may be used.");
          return false;
        }
        long repeat = 1;
        if (!cmStrToLong(r, &repeat)) {
          cmSystemTools::Error(cmStrCat(
            "'--repeat-until-fail' given non-integer value '", r, '\''));
          return false;
        }
        this->Impl->RepeatCount = static_cast<int>(repeat);
        if (repeat > 1) {
          this->Impl->RepeatMode = cmCTest::Repeat::UntilFail;
        }
        return true;
      } },
    CommandArgument{
      "--repeat", CommandArgument::Values::One,
      [this](std::string const& r) -> bool {
        if (this->Impl->RepeatMode != cmCTest::Repeat::Never) {
          cmSystemTools::Error("At most one '--repeat' option may be used.");
          return false;
        }
        cmsys::RegularExpression repeatRegex(
          "^(until-fail|until-pass|after-timeout):([0-9]+)$");
        if (repeatRegex.find(r)) {
          std::string const& count = repeatRegex.match(2);
          unsigned long n = 1;
          cmStrToULong(count, &n); // regex guarantees success
          this->Impl->RepeatCount = static_cast<int>(n);
          if (this->Impl->RepeatCount > 1) {
            std::string const& mode = repeatRegex.match(1);
            if (mode == "until-fail") {
              this->Impl->RepeatMode = cmCTest::Repeat::UntilFail;
            } else if (mode == "until-pass") {
              this->Impl->RepeatMode = cmCTest::Repeat::UntilPass;
            } else if (mode == "after-timeout") {
              this->Impl->RepeatMode = cmCTest::Repeat::AfterTimeout;
            }
          }
        } else {
          cmSystemTools::Error(
            cmStrCat("'--repeat' given invalid value '", r, '\''));
          return false;
        }
        return true;
      } },
    CommandArgument{ "--test-load", CommandArgument::Values::One,
                     [this](std::string const& l) -> bool {
                       unsigned long load;
                       if (cmStrToULong(l, &load)) {
                         this->SetTestLoad(load);
                       } else {
                         cmCTestLog(
                           this, WARNING,
                           "Invalid value for 'Test Load' : " << l << '\n');
                       }
                       return true;
                     } },
    CommandArgument{ "--no-compress-output", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->CompressTestOutput = false;
                       return true;
                     } },
    CommandArgument{ "--print-labels", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->PrintLabels = true;
                       return true;
                     } },
    CommandArgument{ "--http1.0", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->UseHTTP10 = true;
                       return true;
                     } },
    CommandArgument{ "--timeout", CommandArgument::Values::One,
                     [this](std::string const& t) -> bool {
                       auto timeout = cmDuration(atof(t.c_str()));
                       this->Impl->GlobalTimeout = timeout;
                       return true;
                     } },
    CommandArgument{ "--stop-time", CommandArgument::Values::One,
                     [this](std::string const& t) -> bool {
                       this->SetStopTime(t);
                       return true;
                     } },
    CommandArgument{ "--stop-on-failure", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->StopOnFailure = true;
                       return true;
                     } },
    CommandArgument{ "-C", CommandArgument::Values::One, dashC },
    CommandArgument{ "--build-config", CommandArgument::Values::One, dashC },
    CommandArgument{ "--debug", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->Debug = true;
                       return true;
                     } },
    CommandArgument{ "--group", CommandArgument::Values::One, dashGroup },
    // This is an undocumented / deprecated option.
    // "Track" has been renamed to "Group".
    CommandArgument{ "--track", CommandArgument::Values::One, dashGroup },
    CommandArgument{ "--show-line-numbers", CommandArgument::Values::Zero,
                     [](std::string const&) -> bool {
                       // Silently ignore this never-documented and now-removed
                       // option.
                       return true;
                     } },
    CommandArgument{ "--no-label-summary", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->LabelSummary = false;
                       return true;
                     } },
    CommandArgument{ "--no-subproject-summary", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->SubprojectSummary = false;
                       return true;
                     } },
    CommandArgument{ "--progress", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->TestProgressOutput = true;
                       return true;
                     } },
    CommandArgument{ "-Q", CommandArgument::Values::Zero, dashQ },
    CommandArgument{ "--quiet", CommandArgument::Values::Zero, dashQ },
    CommandArgument{ "-V", CommandArgument::Values::Zero, dashV },
    CommandArgument{ "--verbose", CommandArgument::Values::Zero, dashV },
    CommandArgument{ "-VV", CommandArgument::Values::Zero, dashVV },
    CommandArgument{ "--extra-verbose", CommandArgument::Values::Zero,
                     dashVV },
    CommandArgument{ "--output-on-failure", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->OutputTestOutputOnTestFailure = true;
                       return true;
                     } },
    CommandArgument{ "--test-output-size-passed", CommandArgument::Values::One,
                     [this](std::string const& sz) -> bool {
                       long outputSize;
                       if (cmStrToLong(sz, &outputSize)) {
                         this->Impl->TestOptions.OutputSizePassed =
                           static_cast<int>(outputSize);
                       } else {
                         cmCTestLog(
                           this, WARNING,
                           "Invalid value for '--test-output-size-passed': "
                             << sz << "\n");
                       }
                       return true;
                     } },
    CommandArgument{ "--test-output-size-failed", CommandArgument::Values::One,
                     [this](std::string const& sz) -> bool {
                       long outputSize;
                       if (cmStrToLong(sz, &outputSize)) {
                         this->Impl->TestOptions.OutputSizeFailed =
                           static_cast<int>(outputSize);
                       } else {
                         cmCTestLog(
                           this, WARNING,
                           "Invalid value for '--test-output-size-failed': "
                             << sz << "\n");
                       }
                       return true;
                     } },
    CommandArgument{
      "--test-output-truncation", CommandArgument::Values::One,
      [this](std::string const& mode) -> bool {
        if (!SetTruncationMode(this->Impl->TestOptions.OutputTruncation,
                               mode)) {
          cmSystemTools::Error(
            cmStrCat("Invalid value for '--test-output-truncation': ", mode));
          return false;
        }
        return true;
      } },
    CommandArgument{ "--show-only", CommandArgument::Values::ZeroOrOne,
                     [this](std::string const& format) -> bool {
                       this->Impl->ShowOnly = true;
                       // Check if a specific format is requested.
                       // Defaults to human readable text.
                       if (format == "json-v1") {
                         // Force quiet mode so the only output
                         // is the json object model.
                         this->Impl->Quiet = true;
                         this->Impl->OutputAsJson = true;
                         this->Impl->OutputAsJsonVersion = 1;
                       } else if (format == "human") {
                       } else if (!format.empty()) {
                         cmSystemTools::Error(
                           cmStrCat("'--show-only=' given unknown value '",
                                    format, '\''));
                         return false;
                       }
                       return true;
                     } },
    CommandArgument{ "-N", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->ShowOnly = true;
                       return true;
                     } },
    CommandArgument{ "-O", CommandArgument::Values::One, dashO },
    CommandArgument{ "--output-log", CommandArgument::Values::One, dashO },
    CommandArgument{ "--tomorrow-tag", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->TomorrowTag = true;
                       return true;
                     } },
    CommandArgument{ "--force-new-ctest-process",
                     CommandArgument::Values::Zero,
                     [](std::string const&) -> bool {
                       // Silently ignore now-removed option.
                       return true;
                     } },
    CommandArgument{ "-W", CommandArgument::Values::One, dashW },
    CommandArgument{ "--max-width", CommandArgument::Values::One, dashW },
    CommandArgument{ "--interactive-debug-mode", CommandArgument::Values::One,
                     [this](std::string const& idm) -> bool {
                       this->Impl->InteractiveDebugMode = cmIsOn(idm);
                       return true;
                     } },
    CommandArgument{ "--http-header", CommandArgument::Values::One,
                     [this](std::string const& h) -> bool {
                       this->Impl->CommandLineHttpHeaders.push_back(h);
                       return true;
                     } },
    CommandArgument{ "--submit-index", CommandArgument::Values::One,
                     [this](std::string const& index) -> bool {
                       this->Impl->SubmitIndex = atoi(index.c_str());
                       if (this->Impl->SubmitIndex < 0) {
                         this->Impl->SubmitIndex = 0;
                       }
                       return true;
                     } },
    CommandArgument{ "--overwrite", CommandArgument::Values::One,
                     [this](std::string const& opt) -> bool {
                       this->AddCTestConfigurationOverwrite(opt);
                       return true;
                     } },
    CommandArgument{ "-A", CommandArgument::Values::One, dashA },
    CommandArgument{ "--add-notes", CommandArgument::Values::One, dashA },
    CommandArgument{ "--test-dir", "'--test-dir' requires an argument",
                     CommandArgument::Values::One,
                     [this](std::string const& dir) -> bool {
                       this->Impl->TestDir = dir;
                       return true;
                     } },
    CommandArgument{ "--output-junit", CommandArgument::Values::One,
                     [this](std::string const& file) -> bool {
                       this->SetOutputJUnitFileName(file);
                       return true;
                     } },
    CommandArgument{ "--no-tests", CommandArgument::Values::One,
                     [this](std::string const& action) -> bool {
                       if (action == "error"_s) {
                         this->Impl->NoTestsMode = cmCTest::NoTests::Error;
                       } else if (action == "ignore"_s) {
                         this->Impl->NoTestsMode = cmCTest::NoTests::Ignore;
                       } else {
                         cmSystemTools::Error(
                           cmStrCat("'--no-tests=' given unknown value '",
                                    action, '\''));
                         return false;
                       }
                       this->Impl->NoTestsModeSetInCli = true;
                       return true;
                     } },
    CommandArgument{ "-I", CommandArgument::Values::One, dashI },
    CommandArgument{ "--tests-information", CommandArgument::Values::One,
                     dashI },
    CommandArgument{ "-U", CommandArgument::Values::One, dashU },
    CommandArgument{ "--union", CommandArgument::Values::One, dashU },
    CommandArgument{ "-R", CommandArgument::Values::One, dashR },
    CommandArgument{ "--tests-regex", CommandArgument::Values::One, dashR },
    CommandArgument{ "-E", CommandArgument::Values::One, dashE },
    CommandArgument{ "--exclude-regex", CommandArgument::Values::One, dashE },
    CommandArgument{ "-L", CommandArgument::Values::One, dashL },
    CommandArgument{ "--label-regex", CommandArgument::Values::One, dashL },
    CommandArgument{ "-LE", CommandArgument::Values::One, dashLE },
    CommandArgument{ "--label-exclude", CommandArgument::Values::One, dashLE },
    CommandArgument{ "-FA", CommandArgument::Values::One, dashFA },
    CommandArgument{ "--fixture-exclude-any", CommandArgument::Values::One,
                     dashFA },
    CommandArgument{ "-FS", CommandArgument::Values::One, dashFS },
    CommandArgument{ "--fixture-exclude-setup", CommandArgument::Values::One,
                     dashFS },
    CommandArgument{ "-FC", CommandArgument::Values::One, dashFC },
    CommandArgument{ "--fixture-exclude-cleanup", CommandArgument::Values::One,
                     dashFC },
    CommandArgument{ "--resource-spec-file", CommandArgument::Values::One,
                     [this](std::string const& file) -> bool {
                       this->Impl->TestOptions.ResourceSpecFile = file;
                       return true;
                     } },
    CommandArgument{ "--tests-from-file", CommandArgument::Values::One,
                     [this](std::string const& file) -> bool {
                       this->Impl->TestOptions.TestListFile = file;
                       return true;
                     } },
    CommandArgument{ "--exclude-from-file", CommandArgument::Values::One,
                     [this](std::string const& file) -> bool {
                       this->Impl->TestOptions.ExcludeTestListFile = file;
                       return true;
                     } },
    CommandArgument{ "--schedule-random", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->TestOptions.ScheduleRandom = true;
                       return true;
                     } },
    CommandArgument{ "--rerun-failed", CommandArgument::Values::Zero,
                     [this](std::string const&) -> bool {
                       this->Impl->TestOptions.RerunFailed = true;
                       return true;
                     } },
  };

  // process the command line arguments
  for (size_t i = 1; i < args.size(); ++i) {
    std::string const& arg = args[i];
    bool matched = false;
    for (auto const& m : arguments) {
      if (m.matches(arg)) {
        matched = true;
        if (!m.parse(arg, i, args)) {
          return 1;
        }
        break;
      }
    }
    if (!matched && arg == "--build-options"_s) {
      matched = true;
      while (i + 1 < args.size() && args[i + 1] != "--build-target"_s &&
             args[i + 1] != "--test-command"_s) {
        ++i;
        this->Impl->BuildAndTest.BuildOptions.emplace_back(args[i]);
      }
    }
    if (!matched && arg == "--test-command"_s && i + 1 < args.size()) {
      matched = true;
      ++i;
      this->Impl->BuildAndTest.TestCommand = args[i];
      while (i + 1 < args.size()) {
        ++i;
        this->Impl->BuildAndTest.TestCommandArgs.emplace_back(args[i]);
      }
    }
    if (!matched && cmHasLiteralPrefix(arg, "-") &&
        !cmHasLiteralPrefix(arg, "--preset")) {
      cmSystemTools::Error(cmStrCat("Unknown argument: ", arg));
      cmSystemTools::Error("Run 'ctest --help' for all supported options.");
      return 1;
    }
  }

  // handle CTEST_PARALLEL_LEVEL environment variable
  if (!this->Impl->ParallelLevelSetInCli) {
    if (cm::optional<std::string> parallelEnv =
          cmSystemTools::GetEnvVar("CTEST_PARALLEL_LEVEL")) {
      if (parallelEnv->empty() ||
          parallelEnv->find_first_not_of(" \t") == std::string::npos) {
        // An empty value tells ctest to choose a default.
        this->SetParallelLevel(cm::nullopt);
      } else {
        // A non-empty value must be a non-negative integer.
        // Otherwise, ignore it.
        unsigned long plevel = 0;
        if (cmStrToULong(*parallelEnv, &plevel)) {
          this->SetParallelLevel(plevel);
        }
      }
    }
  }

  // handle CTEST_NO_TESTS_ACTION environment variable
  if (!this->Impl->NoTestsModeSetInCli) {
    std::string action;
    if (cmSystemTools::GetEnv("CTEST_NO_TESTS_ACTION", action) &&
        !action.empty()) {
      if (action == "error"_s) {
        this->Impl->NoTestsMode = cmCTest::NoTests::Error;
      } else if (action == "ignore"_s) {
        this->Impl->NoTestsMode = cmCTest::NoTests::Ignore;
      } else {
        cmCTestLog(this, ERROR_MESSAGE,
                   "Unknown value for CTEST_NO_TESTS_ACTION: '" << action
                                                                << '\'');
        return 1;
      }
    }
  }

  // TestProgressOutput only supported if console supports it and not logging
  // to a file
  this->Impl->TestProgressOutput = this->Impl->TestProgressOutput &&
    !this->Impl->OutputLogFile && this->ProgressOutputSupportedByConsole();
#ifdef _WIN32
  if (this->Impl->TestProgressOutput) {
    // Disable output line buffering so we can print content without
    // a newline.
    std::setvbuf(stdout, nullptr, _IONBF, 0);
  }
#endif

  // now what should cmake do? if --build-and-test was specified then
  // we run the build and test handler and return
  if (cmakeAndTest) {
    return this->RunCMakeAndTest();
  }

  // -S, -SP, and/or -SP was specified
  if (!runScripts.empty()) {
    return this->RunScripts(runScripts);
  }

  // Establish the working directory.
  std::string const currDir = cmSystemTools::GetLogicalWorkingDirectory();
  std::string workDir = currDir;
  if (!this->Impl->TestDir.empty()) {
    workDir = cmSystemTools::ToNormalizedPathOnDisk(this->Impl->TestDir);
  }
  cmWorkingDirectory changeDir(workDir);
  if (changeDir.Failed()) {
    cmCTestLog(this, ERROR_MESSAGE, changeDir.GetError() << std::endl);
    return 1;
  }
  this->Impl->BinaryDir = workDir;

  // -D, -T, and/or -M was specified
  if (processSteps) {
    return this->ProcessSteps();
  }

  return this->ExecuteTests(args);
}

int cmCTest::RunScripts(
  std::vector<std::pair<std::string, bool>> const& scripts)
{
  if (this->Impl->ExtraVerbose) {
    cmCTestLog(this, OUTPUT, "* Extra verbosity turned on" << std::endl);
  }

  auto ch = cm::make_unique<cmCTestScriptHandler>(this);
  for (auto const& script : scripts) {
    ch->AddConfigurationScript(script.first, script.second);
  }

  int res = ch->ProcessHandler();
  if (res != 0) {
    cmCTestLog(this, DEBUG,
               "running script failing returning: " << res << std::endl);
  }

  return res;
}

int cmCTest::ExecuteTests(std::vector<std::string> const& args)
{
  this->Impl->ExtraVerbose = this->Impl->Verbose;
  this->Impl->Verbose = true;

  cmCTestLog(this, DEBUG, "Here: " << __LINE__ << std::endl);
  if (!this->Impl->InteractiveDebugMode) {
    this->BlockTestErrorDiagnostics();
  } else {
    cmSystemTools::PutEnv("CTEST_INTERACTIVE_DEBUG_MODE=1");
  }

  this->UpdateCTestConfiguration();

  cmCTestLog(this, DEBUG, "Here: " << __LINE__ << std::endl);

  cmCTestTestHandler handler(this);

  {
    cmake cm(cmake::RoleScript, cmState::CTest);
    cm.SetHomeDirectory("");
    cm.SetHomeOutputDirectory("");
    cm.GetCurrentSnapshot().SetDefaultDefinitions();
    cmGlobalGenerator gg(&cm);
    cmMakefile mf(&gg, cm.GetCurrentSnapshot());
    this->ReadCustomConfigurationFileTree(this->Impl->BinaryDir, &mf);
    handler.PopulateCustomVectors(&mf);
  }

  handler.SetVerbose(this->Impl->Verbose);

  cmInstrumentation instrumentation(this->GetBinaryDir());
  auto processHandler = [&handler]() -> int {
    return handler.ProcessHandler();
  };
  int ret = instrumentation.InstrumentCommand("ctest", args, processHandler);
  instrumentation.CollectTimingData(cmInstrumentationQuery::Hook::PostTest);
  if (ret < 0) {
    cmCTestLog(this, ERROR_MESSAGE, "Errors while running CTest\n");
    if (!this->Impl->OutputTestOutputOnTestFailure) {
      std::string const lastTestLog =
        this->GetBinaryDir() + "/Testing/Temporary/LastTest.log";
      cmCTestLog(this, ERROR_MESSAGE,
                 "Output from these tests are in: " << lastTestLog << '\n');
      cmCTestLog(this, ERROR_MESSAGE,
                 "Use \"--rerun-failed --output-on-failure\" to re-run the "
                 "failed cases verbosely.\n");
    }
    return cmCTest::TEST_ERRORS;
  }

  return 0;
}

int cmCTest::RunCMakeAndTest()
{
  int retv = this->Impl->BuildAndTest.Run();
#ifndef CMAKE_BOOTSTRAP
  cmDynamicLoader::FlushCache();
#endif
  return retv;
}

void cmCTest::SetNotesFiles(std::string const& notes)
{
  this->Impl->NotesFiles = notes;
}

bool cmCTest::GetStopOnFailure() const
{
  return this->Impl->StopOnFailure;
}

void cmCTest::SetStopOnFailure(bool stop)
{
  this->Impl->StopOnFailure = stop;
}

std::chrono::system_clock::time_point cmCTest::GetStopTime() const
{
  return this->Impl->StopTime;
}

void cmCTest::SetStopTime(std::string const& time_str)
{

  struct tm* lctime;
  time_t current_time = time(nullptr);
  lctime = gmtime(&current_time);
  int gm_hour = lctime->tm_hour;
  time_t gm_time = mktime(lctime);
  lctime = localtime(&current_time);
  int local_hour = lctime->tm_hour;

  int tzone_offset = local_hour - gm_hour;
  if (gm_time > current_time && gm_hour < local_hour) {
    // this means gm_time is on the next day
    tzone_offset -= 24;
  } else if (gm_time < current_time && gm_hour > local_hour) {
    // this means gm_time is on the previous day
    tzone_offset += 24;
  }

  tzone_offset *= 100;
  char buf[1024];
  snprintf(buf, sizeof(buf), "%d%02d%02d %s %+05i", lctime->tm_year + 1900,
           lctime->tm_mon + 1, lctime->tm_mday, time_str.c_str(),
           tzone_offset);

  time_t stop_time = curl_getdate(buf, &current_time);
  if (stop_time == -1) {
    this->Impl->StopTime = std::chrono::system_clock::time_point();
    return;
  }
  this->Impl->StopTime = std::chrono::system_clock::from_time_t(stop_time);

  if (stop_time < current_time) {
    this->Impl->StopTime += std::chrono::hours(24);
  }
}

std::string cmCTest::GetScheduleType() const
{
  return this->Impl->ScheduleType;
}

void cmCTest::SetScheduleType(std::string const& type)
{
  this->Impl->ScheduleType = type;
}

void cmCTest::ReadCustomConfigurationFileTree(std::string const& dir,
                                              cmMakefile* mf)
{
  cmCTestLog(this, DEBUG,
             "* Read custom CTest configuration directory: " << dir
                                                             << std::endl);

  auto const fname = [this, &dir]() -> std::string {
    for (char const* ext : { ".cmake", ".ctest" }) {
      std::string path = cmStrCat(dir, "/CTestCustom", ext);
      cmCTestLog(this, DEBUG, "* Check for file: " << path << std::endl);
      if (cmSystemTools::FileExists(path)) {
        return path;
      }
    }
    return "";
  }();

  if (!fname.empty()) {
    cmCTestLog(this, DEBUG,
               "* Read custom CTest configuration file: " << fname
                                                          << std::endl);
    bool erroroc = cmSystemTools::GetErrorOccurredFlag();
    cmSystemTools::ResetErrorOccurredFlag();

    if (!mf->ReadListFile(fname) || cmSystemTools::GetErrorOccurredFlag()) {
      cmCTestLog(this, ERROR_MESSAGE,
                 "Problem reading custom configuration: " << fname
                                                          << std::endl);
    }
    if (erroroc) {
      cmSystemTools::SetErrorOccurred();
    }
  }
}

void cmCTest::PopulateCustomVector(cmMakefile* mf, std::string const& def,
                                   std::vector<std::string>& vec)
{
  cmValue dval = mf->GetDefinition(def);
  if (!dval) {
    return;
  }
  cmCTestLog(this, DEBUG, "PopulateCustomVector: " << def << std::endl);

  cmList::assign(vec, *dval);

  for (std::string const& it : vec) {
    cmCTestLog(this, DEBUG, "  -- " << it << std::endl);
  }
}

void cmCTest::PopulateCustomInteger(cmMakefile* mf, std::string const& def,
                                    int& val)
{
  cmValue dval = mf->GetDefinition(def);
  if (!dval) {
    return;
  }
  val = atoi(dval->c_str());
}

std::string cmCTest::GetShortPathToFile(std::string const& cfname)
{
  std::string const& sourceDir =
    this->GetCTestConfiguration("SourceDirectory");
  std::string const& buildDir = this->GetCTestConfiguration("BuildDirectory");
  std::string fname = cmSystemTools::CollapseFullPath(cfname);

  // Find relative paths to both directories
  std::string srcRelpath = cmSystemTools::RelativePath(sourceDir, fname);
  std::string bldRelpath = cmSystemTools::RelativePath(buildDir, fname);

  // If any contains "." it is not parent directory
  bool inSrc = srcRelpath.find("..") == std::string::npos;
  bool inBld = bldRelpath.find("..") == std::string::npos;
  // TODO: Handle files with .. in their name

  std::string* res = nullptr;

  if (inSrc && inBld) {
    // If both have relative path with no dots, pick the shorter one
    if (srcRelpath.size() < bldRelpath.size()) {
      res = &srcRelpath;
    } else {
      res = &bldRelpath;
    }
  } else if (inSrc) {
    res = &srcRelpath;
  } else if (inBld) {
    res = &bldRelpath;
  }

  std::string path;

  if (!res) {
    path = fname;
  } else {
    cmSystemTools::ConvertToUnixSlashes(*res);

    path = "./" + *res;
    if (path.back() == '/') {
      path.resize(path.size() - 1);
    }
  }

  cmsys::SystemTools::ReplaceString(path, ":", "_");
  cmsys::SystemTools::ReplaceString(path, " ", "_");
  return path;
}

std::string cmCTest::GetCTestConfiguration(std::string const& name)
{
  if (this->Impl->CTestConfigurationOverwrites.find(name) !=
      this->Impl->CTestConfigurationOverwrites.end()) {
    return this->Impl->CTestConfigurationOverwrites[name];
  }
  return this->Impl->CTestConfiguration[name];
}

void cmCTest::EmptyCTestConfiguration()
{
  this->Impl->CTestConfiguration.clear();
}

void cmCTest::SetCTestConfiguration(char const* name, std::string const& value,
                                    bool suppress)
{
  cmCTestOptionalLog(this, HANDLER_VERBOSE_OUTPUT,
                     "SetCTestConfiguration:" << name << ":" << value << "\n",
                     suppress);

  if (!name) {
    return;
  }
  if (value.empty()) {
    this->Impl->CTestConfiguration.erase(name);
    return;
  }
  this->Impl->CTestConfiguration[name] = value;
}

std::string cmCTest::GetSubmitURL()
{
  std::string url = this->GetCTestConfiguration("SubmitURL");
  if (url.empty()) {
    std::string method = this->GetCTestConfiguration("DropMethod");
    std::string user = this->GetCTestConfiguration("DropSiteUser");
    std::string password = this->GetCTestConfiguration("DropSitePassword");
    std::string site = this->GetCTestConfiguration("DropSite");
    std::string location = this->GetCTestConfiguration("DropLocation");

    url = cmStrCat(method.empty() ? "http" : method, "://"_s);
    if (!user.empty()) {
      url += user;
      if (!password.empty()) {
        url += ':';
        url += password;
      }
      url += '@';
    }
    url += site;
    url += location;
  }
  return url;
}

std::string cmCTest::GetCurrentTag()
{
  return this->Impl->CurrentTag;
}

std::string cmCTest::GetBinaryDir()
{
  return this->Impl->BinaryDir;
}

std::string const& cmCTest::GetConfigType()
{
  return this->Impl->ConfigType;
}

cmDuration cmCTest::GetTimeOut() const
{
  return this->Impl->TimeOut;
}

void cmCTest::SetTimeOut(cmDuration t)
{
  this->Impl->TimeOut = t;
}

cmDuration cmCTest::GetGlobalTimeout() const
{
  return this->Impl->GlobalTimeout;
}

bool cmCTest::GetShowOnly()
{
  return this->Impl->ShowOnly;
}

bool cmCTest::GetOutputAsJson()
{
  return this->Impl->OutputAsJson;
}

int cmCTest::GetOutputAsJsonVersion()
{
  return this->Impl->OutputAsJsonVersion;
}

bool cmCTest::ShouldUseHTTP10() const
{
  return this->Impl->UseHTTP10;
}

bool cmCTest::ShouldPrintLabels() const
{
  return this->Impl->PrintLabels;
}

int cmCTest::GetMaxTestNameWidth() const
{
  return this->Impl->MaxTestNameWidth;
}

void cmCTest::SetMaxTestNameWidth(int w)
{
  this->Impl->MaxTestNameWidth = w;
}

void cmCTest::SetProduceXML(bool v)
{
  this->Impl->ProduceXML = v;
}

bool cmCTest::GetProduceXML()
{
  return this->Impl->ProduceXML;
}

std::vector<std::string>& cmCTest::GetInitialCommandLineArguments()
{
  return this->Impl->InitialCommandLineArguments;
}

char const* cmCTest::GetSpecificGroup()
{
  if (this->Impl->SpecificGroup.empty()) {
    return nullptr;
  }
  return this->Impl->SpecificGroup.c_str();
}

void cmCTest::SetSpecificGroup(char const* group)
{
  if (!group) {
    this->Impl->SpecificGroup.clear();
    return;
  }
  this->Impl->SpecificGroup = group;
}

void cmCTest::SetFailover(bool failover)
{
  this->Impl->Failover = failover;
}

bool cmCTest::GetFailover() const
{
  return this->Impl->Failover;
}

bool cmCTest::GetTestProgressOutput() const
{
  return this->Impl->TestProgressOutput && !GetExtraVerbose();
}

bool cmCTest::GetVerbose() const
{
  return this->Impl->Verbose;
}

bool cmCTest::GetExtraVerbose() const
{
  return this->Impl->ExtraVerbose;
}

bool cmCTest::GetInteractiveDebugMode() const
{
  return this->Impl->InteractiveDebugMode;
}

bool cmCTest::GetLabelSummary() const
{
  return this->Impl->LabelSummary;
}

bool cmCTest::GetSubprojectSummary() const
{
  return this->Impl->SubprojectSummary;
}

bool cmCTest::GetOutputTestOutputOnTestFailure() const
{
  return this->Impl->OutputTestOutputOnTestFailure;
}

std::map<std::string, std::string> const& cmCTest::GetDefinitions() const
{
  return this->Impl->Definitions;
}

int cmCTest::GetRepeatCount() const
{
  return this->Impl->RepeatCount;
}

cmCTest::Repeat cmCTest::GetRepeatMode() const
{
  return this->Impl->RepeatMode;
}

cmCTest::NoTests cmCTest::GetNoTestsMode() const
{
  return this->Impl->NoTestsMode;
}

void cmCTest::SetBuildID(std::string const& id)
{
  this->Impl->BuildID = id;
}

std::string cmCTest::GetBuildID() const
{
  return this->Impl->BuildID;
}

cmCTestTestOptions const& cmCTest::GetTestOptions() const
{
  return this->Impl->TestOptions;
}

std::vector<std::string> cmCTest::GetCommandLineHttpHeaders() const
{
  return this->Impl->CommandLineHttpHeaders;
}

void cmCTest::AddSubmitFile(Part part, std::string const& name)
{
  this->Impl->Parts[part].SubmitFiles.emplace_back(name);
}

std::vector<std::string> const& cmCTest::GetSubmitFiles(Part part) const
{
  return this->Impl->Parts[part].SubmitFiles;
}

void cmCTest::ClearSubmitFiles(Part part)
{
  this->Impl->Parts[part].SubmitFiles.clear();
}

void cmCTest::AddCTestConfigurationOverwrite(std::string const& overStr)
{
  size_t epos = overStr.find('=');
  if (epos == std::string::npos) {
    cmCTestLog(this, ERROR_MESSAGE,
               "CTest configuration overwrite specified in the wrong format.\n"
               "Valid format is: --overwrite key=value\n"
               "The specified was: --overwrite "
                 << overStr << '\n');
    return;
  }
  std::string key = overStr.substr(0, epos);
  std::string value = overStr.substr(epos + 1);
  this->Impl->CTestConfigurationOverwrites[key] = value;
}

void cmCTest::SetConfigType(std::string const& ct)
{
  this->Impl->ConfigType = ct;
  cmSystemTools::ReplaceString(this->Impl->ConfigType, ".\\", "");
  std::string confTypeEnv = "CMAKE_CONFIG_TYPE=" + this->Impl->ConfigType;
  cmSystemTools::PutEnv(confTypeEnv);
}

bool cmCTest::SetCTestConfigurationFromCMakeVariable(
  cmMakefile* mf, char const* dconfig, std::string const& cmake_var,
  bool suppress)
{
  cmValue ctvar = mf->GetDefinition(cmake_var);
  if (!ctvar) {
    return false;
  }
  cmCTestOptionalLog(this, HANDLER_VERBOSE_OUTPUT,
                     "SetCTestConfigurationFromCMakeVariable:"
                       << dconfig << ":" << cmake_var << std::endl,
                     suppress);
  this->SetCTestConfiguration(dconfig, *ctvar, suppress);
  return true;
}

void cmCTest::SetCMakeVariables(cmMakefile& mf)
{
  auto set = [&](char const* cmake_var, char const* ctest_opt) {
    std::string val = this->GetCTestConfiguration(ctest_opt);
    if (!val.empty()) {
      cmCTestOptionalLog(
        this, HANDLER_VERBOSE_OUTPUT,
        "SetCMakeVariable:" << cmake_var << ":" << val << std::endl, false);
      mf.AddDefinition(cmake_var, val);
    }
  };

  set("CTEST_SITE", "Site");
  set("CTEST_BUILD_NAME", "BuildName");
  set("CTEST_NIGHTLY_START_TIME", "NightlyStartTime");
  set("CTEST_SOURCE_DIRECTORY", "SourceDirectory");
  set("CTEST_BINARY_DIRECTORY", "BuildDirectory");

  // CTest Update Step
  set("CTEST_UPDATE_COMMAND", "UpdateCommand");
  set("CTEST_UPDATE_OPTIONS", "UpdateOptions");
  set("CTEST_UPDATE_TYPE", "UpdateType");
  set("CTEST_CVS_COMMAND", "CVSCommand");
  set("CTEST_CVS_UPDATE_OPTIONS", "CVSUpdateOptions");
  set("CTEST_SVN_COMMAND", "SVNCommand");
  set("CTEST_SVN_UPDATE_OPTIONS", "SVNUpdateOptions");
  set("CTEST_SVN_OPTIONS", "SVNOptions");
  set("CTEST_BZR_COMMAND", "BZRCommand");
  set("CTEST_BZR_UPDATE_OPTIONS", "BZRUpdateOptions");
  set("CTEST_GIT_COMMAND", "GITCommand");
  set("CTEST_GIT_UPDATE_OPTIONS", "GITUpdateOptions");
  set("CTEST_GIT_INIT_SUBMODULES", "GITInitSubmodules");
  set("CTEST_GIT_UPDATE_CUSTOM", "GITUpdateCustom");
  set("CTEST_UPDATE_VERSION_ONLY", "UpdateVersionOnly");
  set("CTEST_UPDATE_VERSION_OVERRIDE", "UpdateVersionOverride");
  set("CTEST_HG_COMMAND", "HGCommand");
  set("CTEST_HG_UPDATE_OPTIONS", "HGUpdateOptions");
  set("CTEST_P4_COMMAND", "P4Command");
  set("CTEST_P4_UPDATE_CUSTOM", "P4UpdateCustom");
  set("CTEST_P4_UPDATE_OPTIONS", "P4UpdateOptions");
  set("CTEST_P4_CLIENT", "P4Client");
  set("CTEST_P4_OPTIONS", "P4Options");

  // CTest Configure Step
  set("CTEST_CONFIGURE_COMMAND", "ConfigureCommand");
  set("CTEST_LABELS_FOR_SUBPROJECTS", "LabelsForSubprojects");

  // CTest Build Step
  set("CTEST_BUILD_COMMAND", "MakeCommand");
  set("CTEST_USE_LAUNCHERS", "UseLaunchers");

  // CTest Coverage Step
  set("CTEST_COVERAGE_COMMAND", "CoverageCommand");
  set("CTEST_COVERAGE_EXTRA_FLAGS", "CoverageExtraFlags");

  // CTest MemCheck Step
  set("CTEST_MEMORYCHECK_TYPE", "MemoryCheckType");
  set("CTEST_MEMORYCHECK_SANITIZER_OPTIONS", "MemoryCheckSanitizerOptions");
  set("CTEST_MEMORYCHECK_COMMAND", "MemoryCheckCommand");
  set("CTEST_MEMORYCHECK_COMMAND_OPTIONS", "MemoryCheckCommandOptions");
  set("CTEST_MEMORYCHECK_SUPPRESSIONS_FILE", "MemoryCheckSuppressionFile");

  // CTest Submit Step
  set("CTEST_SUBMIT_URL", "SubmitURL");
  set("CTEST_DROP_METHOD", "DropMethod");
  set("CTEST_DROP_SITE_USER", "DropSiteUser");
  set("CTEST_DROP_SITE_PASSWORD", "DropSitePassword");
  set("CTEST_DROP_SITE", "DropSite");
  set("CTEST_DROP_LOCATION", "DropLocation");
  set("CTEST_TLS_VERIFY", "TLSVerify");
  set("CTEST_TLS_VERSION", "TLSVersion");
  set("CTEST_CURL_OPTIONS", "CurlOptions");
  set("CTEST_SUBMIT_INACTIVITY_TIMEOUT", "SubmitInactivityTimeout");
}

bool cmCTest::RunCommand(std::vector<std::string> const& args,
                         std::string* stdOut, std::string* stdErr, int* retVal,
                         char const* dir, cmDuration timeout,
                         Encoding encoding)
{
  std::vector<char const*> argv;
  argv.reserve(args.size() + 1);
  for (std::string const& a : args) {
    argv.push_back(a.c_str());
  }
  argv.push_back(nullptr);

  stdOut->clear();
  stdErr->clear();

  cmUVProcessChainBuilder builder;
  builder.AddCommand(args)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR);
  if (dir) {
    builder.SetWorkingDirectory(dir);
  }
  auto chain = builder.Start();

  cm::uv_timer_ptr timer;
  bool timedOut = false;
  if (timeout.count()) {
    timer.init(chain.GetLoop(), &timedOut);
    timer.start(
      [](uv_timer_t* t) {
        auto* timedOutPtr = static_cast<bool*>(t->data);
        *timedOutPtr = true;
      },
      static_cast<uint64_t>(timeout.count() * 1000.0), 0);
  }

  std::vector<char> tempOutput;
  bool outFinished = false;
  cm::uv_pipe_ptr outStream;
  std::vector<char> tempError;
  bool errFinished = false;
  cm::uv_pipe_ptr errStream;
  cmProcessOutput processOutput(encoding);
  auto startRead = [this, &chain, &processOutput](
                     cm::uv_pipe_ptr& pipe, int stream,
                     std::vector<char>& temp,
                     bool& finished) -> std::unique_ptr<cmUVStreamReadHandle> {
    pipe.init(chain.GetLoop(), 0);
    uv_pipe_open(pipe, stream);
    return cmUVStreamRead(
      pipe,
      [this, &temp, &processOutput](std::vector<char> data) {
        cm::append(temp, data);
        if (this->Impl->ExtraVerbose) {
          std::string strdata;
          processOutput.DecodeText(data.data(), data.size(), strdata);
          cmSystemTools::Stdout(strdata);
        }
      },
      [&finished]() { finished = true; });
  };
  auto outputHandle =
    startRead(outStream, chain.OutputStream(), tempOutput, outFinished);
  auto errorHandle =
    startRead(errStream, chain.ErrorStream(), tempError, errFinished);
  while (!timedOut && !(outFinished && errFinished)) {
    uv_run(&chain.GetLoop(), UV_RUN_ONCE);
  }
  if (this->Impl->ExtraVerbose) {
    std::string strdata;
    processOutput.DecodeText(std::string(), strdata);
    if (!strdata.empty()) {
      cmSystemTools::Stdout(strdata);
    }
  }

  while (!timedOut && !chain.Finished()) {
    uv_run(&chain.GetLoop(), UV_RUN_ONCE);
  }
  if (!tempOutput.empty()) {
    processOutput.DecodeText(tempOutput, tempOutput);
    stdOut->append(tempOutput.data(), tempOutput.size());
  }
  if (!tempError.empty()) {
    processOutput.DecodeText(tempError, tempError);
    stdErr->append(tempError.data(), tempError.size());
  }

  bool result = true;
  if (timedOut) {
    char const* error_str = "Process terminated due to timeout\n";
    cmCTestLog(this, ERROR_MESSAGE, error_str << std::endl);
    stdErr->append(error_str, strlen(error_str));
    result = false;
  } else {
    auto const& status = chain.GetStatus(0);
    auto exception = status.GetException();
    switch (exception.first) {
      case cmUVProcessChain::ExceptionCode::None:
        if (retVal) {
          *retVal = static_cast<int>(status.ExitStatus);
        } else {
          if (status.ExitStatus != 0) {
            result = false;
          }
        }
        break;
      default: {
        cmCTestLog(this, ERROR_MESSAGE, exception.second << std::endl);
        stdErr->append(exception.second);
        result = false;
      } break;
    }
  }

  return result;
}

void cmCTest::SetOutputLogFileName(std::string const& name)
{
  if (!name.empty()) {
    this->Impl->OutputLogFile = cm::make_unique<cmGeneratedFileStream>(name);
  } else {
    this->Impl->OutputLogFile.reset();
  }
}

void cmCTest::SetOutputJUnitFileName(std::string const& name)
{
  this->Impl->TestOptions.JUnitXMLFileName = name;
  // Turn test output compression off.
  // This makes it easier to include test output in the resulting
  // JUnit XML report.
  this->Impl->CompressTestOutput = false;
}

static char const* cmCTestStringLogType[] = { "DEBUG",
                                              "OUTPUT",
                                              "HANDLER_OUTPUT",
                                              "HANDLER_PROGRESS_OUTPUT",
                                              "HANDLER_TEST_PROGRESS_OUTPUT",
                                              "HANDLER_VERBOSE_OUTPUT",
                                              "WARNING",
                                              "ERROR_MESSAGE" };

void cmCTest::Log(LogType logType, std::string msg, bool suppress)
{
  if (msg.empty()) {
    return;
  }
  if (suppress && logType != cmCTest::ERROR_MESSAGE) {
    return;
  }
  if (logType == cmCTest::HANDLER_PROGRESS_OUTPUT &&
      (this->Impl->Debug || this->Impl->ExtraVerbose)) {
    return;
  }
  if (this->Impl->OutputLogFile) {
    bool display = true;
    if (logType == cmCTest::DEBUG && !this->Impl->Debug) {
      display = false;
    }
    if (logType == cmCTest::HANDLER_VERBOSE_OUTPUT && !this->Impl->Debug &&
        !this->Impl->ExtraVerbose) {
      display = false;
    }
    if (display) {
      if (this->Impl->OutputLogFileLastTag &&
          logType != *this->Impl->OutputLogFileLastTag) {
        *this->Impl->OutputLogFile << "[" << cmCTestStringLogType[logType]
                                   << "] " << std::endl;
      }
      *this->Impl->OutputLogFile << msg << std::flush;
      if (this->Impl->OutputLogFileLastTag &&
          logType != *this->Impl->OutputLogFileLastTag) {
        *this->Impl->OutputLogFile << std::endl;
        this->Impl->OutputLogFileLastTag = logType;
      }
    }
  }
  if (!this->Impl->Quiet) {
    if (logType == HANDLER_TEST_PROGRESS_OUTPUT) {
      if (this->Impl->TestProgressOutput) {
        if (this->Impl->FlushTestProgressLine) {
          printf("\r");
          this->Impl->FlushTestProgressLine = false;
          std::cout.flush();
        }

        if (msg.find('\n') != std::string::npos) {
          this->Impl->FlushTestProgressLine = true;
          msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
        }

        std::cout << msg;
#ifndef _WIN32
        printf("\x1B[K"); // move caret to end
#endif
        std::cout.flush();
        return;
      }
      logType = HANDLER_OUTPUT;
    }

    switch (logType) {
      case DEBUG:
        if (this->Impl->Debug) {
          std::cout << msg << std::flush;
        }
        break;
      case OUTPUT:
      case HANDLER_OUTPUT:
        if (this->Impl->Debug || this->Impl->Verbose) {
          std::cout << msg << std::flush;
        }
        break;
      case HANDLER_VERBOSE_OUTPUT:
        if (this->Impl->Debug || this->Impl->ExtraVerbose) {
          std::cout << msg << std::flush;
        }
        break;
      case WARNING:
        std::cerr << msg << std::flush;
        break;
      case ERROR_MESSAGE:
        std::cerr << msg << std::flush;
        cmSystemTools::SetErrorOccurred();
        break;
      default:
        std::cout << msg << std::flush;
    }
  }
}

std::string cmCTest::GetColorCode(Color color) const
{
  if (this->Impl->OutputColorCode) {
    return cmStrCat("\033[0;", static_cast<int>(color), 'm');
  }

  return {};
}

void cmCTest::SetTimeLimit(cmValue val)
{
  this->Impl->TimeLimit =
    val ? cmDuration(atof(val->c_str())) : cmCTest::MaxDuration();
}

cmDuration cmCTest::GetElapsedTime() const
{
  return std::chrono::duration_cast<cmDuration>(
    std::chrono::steady_clock::now() - this->Impl->StartTime);
}

cmDuration cmCTest::GetRemainingTimeAllowed() const
{
  return this->Impl->TimeLimit - this->GetElapsedTime();
}

cmDuration cmCTest::MaxDuration()
{
  return cmDuration(1.0e7);
}

bool cmCTest::CompressString(std::string& str)
{
  int ret;
  z_stream strm;

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit(&strm, -1); // default compression level
  if (ret != Z_OK) {
    return false;
  }

  unsigned char* in =
    reinterpret_cast<unsigned char*>(const_cast<char*>(str.c_str()));
  // zlib makes the guarantee that this is the maximum output size
  int outSize =
    static_cast<int>(static_cast<double>(str.size()) * 1.001 + 13.0);
  std::vector<unsigned char> out(outSize);

  strm.avail_in = static_cast<uInt>(str.size());
  strm.next_in = in;
  strm.avail_out = outSize;
  strm.next_out = out.data();
  ret = deflate(&strm, Z_FINISH);

  if (ret != Z_STREAM_END) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Error during gzip compression." << std::endl);
    return false;
  }

  (void)deflateEnd(&strm);

  // Now base64 encode the resulting binary string
  std::vector<unsigned char> base64EncodedBuffer((outSize * 3) / 2);

  size_t rlen = cmsysBase64_Encode(out.data(), strm.total_out,
                                   base64EncodedBuffer.data(), 1);

  str.assign(reinterpret_cast<char*>(base64EncodedBuffer.data()), rlen);

  return true;
}

bool cmCTest::StartResultingXML(Part part, char const* name, int submitIndex,
                                cmGeneratedFileStream& xofs)
{
  if (!name) {
    cmCTestLog(
      this, ERROR_MESSAGE,
      "Cannot create resulting XML file without providing the name\n");
    return false;
  }
  if (submitIndex == 0) {
    submitIndex = this->Impl->SubmitIndex;
  }
  std::ostringstream ostr;
  ostr << name;
  if (submitIndex > 0) {
    ostr << "_" << submitIndex;
  }
  ostr << ".xml";
  if (this->Impl->CurrentTag.empty()) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Current Tag empty, this may mean NightlyStartTime / "
               "CTEST_NIGHTLY_START_TIME was not set correctly. Or "
               "maybe you forgot to call ctest_start() before calling "
               "ctest_configure().\n");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  if (!this->OpenOutputFile(this->Impl->CurrentTag, ostr.str(), xofs, true)) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Cannot create resulting XML file: " << ostr.str() << '\n');
    return false;
  }
  this->AddSubmitFile(part, ostr.str());
  return true;
}

bool cmCTest::StartLogFile(char const* name, int submitIndex,
                           cmGeneratedFileStream& xofs)
{
  if (!name) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Cannot create log file without providing the name\n");
    return false;
  }
  if (submitIndex == 0) {
    submitIndex = this->Impl->SubmitIndex;
  }
  std::ostringstream ostr;
  ostr << "Last" << name;
  if (submitIndex > 0) {
    ostr << "_" << submitIndex;
  }
  if (!this->Impl->CurrentTag.empty()) {
    ostr << "_" << this->Impl->CurrentTag;
  }
  ostr << ".log";
  if (!this->OpenOutputFile("Temporary", ostr.str(), xofs)) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Cannot create log file: " << ostr.str() << '\n');
    return false;
  }
  return true;
}

cmInstrumentation& cmCTest::GetInstrumentation()
{
  if (!this->Impl->Instrumentation) {
    this->Impl->Instrumentation =
      cm::make_unique<cmInstrumentation>(this->GetBinaryDir());
  }
  return *this->Impl->Instrumentation;
}

bool cmCTest::GetUseVerboseInstrumentation() const
{
  return this->Impl->UseVerboseInstrumentation;
}

void cmCTest::ConvertInstrumentationSnippetsToXML(cmXMLWriter& xml,
                                                  std::string const& subdir)
{
  std::string data_dir =
    cmStrCat(this->GetInstrumentation().GetCDashDir(), '/', subdir);

  cmsys::Directory d;
  if (!d.Load(data_dir) || d.GetNumberOfFiles() == 0) {
    return;
  }

  xml.StartElement("Commands");

  for (unsigned int i = 0; i < d.GetNumberOfFiles(); i++) {
    std::string fpath = d.GetFilePath(i);
    std::string fname = d.GetFile(i);
    if (fname.rfind('.', 0) == 0) {
      continue;
    }
    this->ConvertInstrumentationJSONFileToXML(fpath, xml);
  }

  xml.EndElement(); // Commands
}

bool cmCTest::ConvertInstrumentationJSONFileToXML(std::string const& fpath,
                                                  cmXMLWriter& xml)
{
  Json::Value root;
  this->Impl->parseState = cmJSONState(fpath, &root);
  if (!this->Impl->parseState.errors.empty()) {
    cmCTestLog(this, ERROR_MESSAGE,
               this->Impl->parseState.GetErrorMessage(true) << std::endl);
    return false;
  }

  if (root.type() != Json::objectValue) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Expected object, found " << root.type() << " for "
                                         << root.asString() << std::endl);
    return false;
  }

  std::vector<std::string> required_members = {
    "command",
    "role",
    "dynamicSystemInformation",
  };
  for (std::string const& required_member : required_members) {
    if (!root.isMember(required_member)) {
      cmCTestLog(this, ERROR_MESSAGE,
                 fpath << " is missing the '" << required_member << "' key"
                       << std::endl);
      return false;
    }
  }

  // Do not record command-level data for Test.xml files because
  // it is redundant with information actually captured by CTest.
  bool generating_test_xml = root["role"] == "test";
  if (!generating_test_xml) {
    std::string element_name = root["role"].asString();
    element_name[0] = static_cast<char>(std::toupper(element_name[0]));
    xml.StartElement(element_name);
    std::vector<std::string> keys = root.getMemberNames();
    for (auto const& key : keys) {
      auto key_type = root[key].type();
      if (key_type == Json::objectValue || key_type == Json::arrayValue) {
        continue;
      }
      if (key == "role" || key == "target" || key == "targetType" ||
          key == "targetLabels") {
        continue;
      }
      // Truncate the full command line if verbose instrumentation
      // was not requested.
      if (key == "command" && !this->GetUseVerboseInstrumentation()) {
        std::string command_str = root[key].asString();
        std::string truncated = command_str.substr(0, command_str.find(' '));
        if (command_str != truncated) {
          truncated = cmStrCat(truncated, " (truncated)");
        }
        xml.Attribute(key.c_str(), truncated);
        continue;
      }
      xml.Attribute(key.c_str(), root[key].asString());
    }
  }

  // Record dynamicSystemInformation section as XML.
  auto dynamic_information = root["dynamicSystemInformation"];
  std::vector<std::string> keys = dynamic_information.getMemberNames();
  for (auto const& key : keys) {
    std::string measurement_name = key;
    measurement_name[0] = static_cast<char>(std::toupper(measurement_name[0]));

    xml.StartElement("NamedMeasurement");
    xml.Attribute("type", "numeric/double");
    xml.Attribute("name", measurement_name);
    xml.Element("Value", dynamic_information[key].asString());
    xml.EndElement(); // NamedMeasurement
  }

  if (!generating_test_xml) {
    xml.EndElement(); // role
  }

  cmSystemTools::RemoveFile(fpath);
  return true;
}
