/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTest.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
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
#include <cm3p/zlib.h>

#include "cmsys/Base64.h"
#include "cmsys/Directory.hxx"
#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/Process.h"
#include "cmsys/RegularExpression.hxx"
#include "cmsys/SystemInformation.hxx"
#if defined(_WIN32)
#  include <windows.h> // IWYU pragma: keep
#else
#  include <unistd.h> // IWYU pragma: keep
#endif

#include "cmCMakePresetsGraph.h"
#include "cmCTestBuildAndTestHandler.h"
#include "cmCTestBuildHandler.h"
#include "cmCTestConfigureHandler.h"
#include "cmCTestCoverageHandler.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestMemCheckHandler.h"
#include "cmCTestScriptHandler.h"
#include "cmCTestStartCommand.h"
#include "cmCTestSubmitHandler.h"
#include "cmCTestTestHandler.h"
#include "cmCTestUpdateHandler.h"
#include "cmCTestUploadHandler.h"
#include "cmDynamicLoader.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmProcessOutput.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmVersionConfig.h"
#include "cmXMLWriter.h"
#include "cmake.h"

#if defined(__BEOS__) || defined(__HAIKU__)
#  include <be/kernel/OS.h> /* disable_debugger() API. */
#endif

struct cmCTest::Private
{
  /** Representation of one part.  */
  struct PartInfo
  {
    void SetName(const std::string& name) { this->Name = name; }
    const std::string& GetName() const { return this->Name; }

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

  bool FlushTestProgressLine = false;

  bool ForceNewCTestProcess = false;

  bool RunConfigurationScript = false;

  // these are helper classes
  cmCTestBuildHandler BuildHandler;
  cmCTestBuildAndTestHandler BuildAndTestHandler;
  cmCTestCoverageHandler CoverageHandler;
  cmCTestScriptHandler ScriptHandler;
  cmCTestTestHandler TestHandler;
  cmCTestUpdateHandler UpdateHandler;
  cmCTestConfigureHandler ConfigureHandler;
  cmCTestMemCheckHandler MemCheckHandler;
  cmCTestSubmitHandler SubmitHandler;
  cmCTestUploadHandler UploadHandler;

  std::vector<cmCTestGenericHandler*> GetTestingHandlers()
  {
    return { &this->BuildHandler,     &this->BuildAndTestHandler,
             &this->CoverageHandler,  &this->ScriptHandler,
             &this->TestHandler,      &this->UpdateHandler,
             &this->ConfigureHandler, &this->MemCheckHandler,
             &this->SubmitHandler,    &this->UploadHandler };
  }

  std::map<std::string, cmCTestGenericHandler*> GetNamedTestingHandlers()
  {
    return { { "build", &this->BuildHandler },
             { "buildtest", &this->BuildAndTestHandler },
             { "coverage", &this->CoverageHandler },
             { "script", &this->ScriptHandler },
             { "test", &this->TestHandler },
             { "update", &this->UpdateHandler },
             { "configure", &this->ConfigureHandler },
             { "memcheck", &this->MemCheckHandler },
             { "submit", &this->SubmitHandler },
             { "upload", &this->UploadHandler } };
  }

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

  int MaxTestNameWidth = 30;

  int ParallelLevel = 1;
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

  // By default we write output to the process output streams.
  std::ostream* StreamOut = &std::cout;
  std::ostream* StreamErr = &std::cerr;

  bool SuppressUpdatingCTestConfiguration = false;

  bool Debug = false;
  bool ShowLineNumbers = false;
  bool Quiet = false;

  std::string BuildID;

  std::vector<std::string> InitialCommandLineArguments;

  int SubmitIndex = 0;

  std::unique_ptr<cmGeneratedFileStream> OutputLogFile;
  int OutputLogFileLastTag = -1;

  bool OutputTestOutputOnTestFailure = false;
  bool OutputColorCode = cmCTest::ColoredOutputSupportedByConsole();

  std::map<std::string, std::string> Definitions;

  cmCTest::NoTests NoTestsMode = cmCTest::NoTests::Legacy;
};

struct tm* cmCTest::GetNightlyTime(std::string const& str, bool tomorrowtag)
{
  struct tm* lctime;
  time_t tctime = time(nullptr);
  lctime = gmtime(&tctime);
  char buf[1024];
  // add todays year day and month to the time in str because
  // curl_getdate no longer assumes the day is today
  snprintf(buf, sizeof(buf), "%d%02d%02d %s", lctime->tm_year + 1900,
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

  const int dayLength = 24 * 60 * 60;
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

std::string cmCTest::CleanString(const std::string& str,
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

std::string cmCTest::DecodeURL(const std::string& in)
{
  std::string out;
  for (const char* c = in.c_str(); *c; ++c) {
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
  : Impl(new Private)
{
  std::string envValue;
  if (cmSystemTools::GetEnv("CTEST_OUTPUT_ON_FAILURE", envValue)) {
    this->Impl->OutputTestOutputOnTestFailure = !cmIsOff(envValue);
  }
  envValue.clear();
  if (cmSystemTools::GetEnv("CTEST_PROGRESS_OUTPUT", envValue)) {
    this->Impl->TestProgressOutput = !cmIsOff(envValue);
  }

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

  for (auto& handler : this->Impl->GetTestingHandlers()) {
    handler->SetCTestInstance(this);
  }

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();
}

cmCTest::~cmCTest() = default;

int cmCTest::GetParallelLevel() const
{
  return this->Impl->ParallelLevel;
}

void cmCTest::SetParallelLevel(int level)
{
  this->Impl->ParallelLevel = level < 1 ? 1 : level;
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

cmCTest::Part cmCTest::GetPartFromName(const std::string& name)
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

int cmCTest::Initialize(const char* binary_dir, cmCTestStartCommand* command)
{
  bool quiet = false;
  if (command && command->ShouldBeQuiet()) {
    quiet = true;
  }

  cmCTestOptionalLog(this, DEBUG, "Here: " << __LINE__ << std::endl, quiet);
  if (!this->Impl->InteractiveDebugMode) {
    this->BlockTestErrorDiagnostics();
  } else {
    cmSystemTools::PutEnv("CTEST_INTERACTIVE_DEBUG_MODE=1");
  }

  this->Impl->BinaryDir = binary_dir;
  cmSystemTools::ConvertToUnixSlashes(this->Impl->BinaryDir);

  this->UpdateCTestConfiguration();

  cmCTestOptionalLog(this, DEBUG, "Here: " << __LINE__ << std::endl, quiet);
  if (this->Impl->ProduceXML) {
    cmCTestOptionalLog(this, DEBUG, "Here: " << __LINE__ << std::endl, quiet);
    cmCTestOptionalLog(this, OUTPUT,
                       "   Site: "
                         << this->GetCTestConfiguration("Site") << std::endl
                         << "   Build name: "
                         << cmCTest::SafeBuildIdField(
                              this->GetCTestConfiguration("BuildName"))
                         << std::endl,
                       quiet);
    cmCTestOptionalLog(this, DEBUG, "Produce XML is on" << std::endl, quiet);
    if (this->Impl->TestModel == cmCTest::NIGHTLY &&
        this->GetCTestConfiguration("NightlyStartTime").empty()) {
      cmCTestOptionalLog(
        this, WARNING,
        "WARNING: No nightly start time found please set in CTestConfig.cmake"
        " or DartConfig.cmake"
          << std::endl,
        quiet);
      cmCTestOptionalLog(this, DEBUG, "Here: " << __LINE__ << std::endl,
                         quiet);
      return 0;
    }
  }

  cmake cm(cmake::RoleScript, cmState::CTest);
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator gg(&cm);
  cmMakefile mf(&gg, cm.GetCurrentSnapshot());
  if (!this->ReadCustomConfigurationFileTree(this->Impl->BinaryDir, &mf)) {
    cmCTestOptionalLog(
      this, DEBUG, "Cannot find custom configuration file tree" << std::endl,
      quiet);
    return 0;
  }

  if (this->Impl->ProduceXML) {
    // Verify "Testing" directory exists:
    //
    std::string testingDir = this->Impl->BinaryDir + "/Testing";
    if (cmSystemTools::FileExists(testingDir)) {
      if (!cmSystemTools::FileIsDirectory(testingDir)) {
        cmCTestLog(this, ERROR_MESSAGE,
                   "File " << testingDir
                           << " is in the place of the testing directory"
                           << std::endl);
        return 0;
      }
    } else {
      if (!cmSystemTools::MakeDirectory(testingDir)) {
        cmCTestLog(this, ERROR_MESSAGE,
                   "Cannot create directory " << testingDir << std::endl);
        return 0;
      }
    }

    // Create new "TAG" file or read existing one:
    //
    bool createNewTag = true;
    if (command) {
      createNewTag = command->ShouldCreateNewTag();
    }

    std::string tagfile = testingDir + "/TAG";
    cmsys::ifstream tfin(tagfile.c_str());
    std::string tag;

    if (createNewTag) {
      time_t tctime = time(nullptr);
      if (this->Impl->TomorrowTag) {
        tctime += (24 * 60 * 60);
      }
      struct tm* lctime = gmtime(&tctime);
      if (tfin && cmSystemTools::GetLineFromStream(tfin, tag)) {
        int year = 0;
        int mon = 0;
        int day = 0;
        int hour = 0;
        int min = 0;
        sscanf(tag.c_str(), "%04d%02d%02d-%02d%02d", &year, &mon, &day, &hour,
               &min);
        if (year != lctime->tm_year + 1900 || mon != lctime->tm_mon + 1 ||
            day != lctime->tm_mday) {
          tag.clear();
        }
        std::string group;
        if (cmSystemTools::GetLineFromStream(tfin, group) &&
            !this->Impl->Parts[PartStart] && !command) {
          this->Impl->SpecificGroup = group;
        }
        std::string model;
        if (cmSystemTools::GetLineFromStream(tfin, model) &&
            !this->Impl->Parts[PartStart] && !command) {
          this->Impl->TestModel = GetTestModelFromString(model);
        }
        tfin.close();
      }
      if (tag.empty() || (nullptr != command) ||
          this->Impl->Parts[PartStart]) {
        cmCTestOptionalLog(
          this, DEBUG,
          "TestModel: " << this->GetTestModelString() << std::endl, quiet);
        cmCTestOptionalLog(this, DEBUG,
                           "TestModel: " << this->Impl->TestModel << std::endl,
                           quiet);
        if (this->Impl->TestModel == cmCTest::NIGHTLY) {
          lctime = this->GetNightlyTime(
            this->GetCTestConfiguration("NightlyStartTime"),
            this->Impl->TomorrowTag);
        }
        char datestring[100];
        snprintf(datestring, sizeof(datestring), "%04d%02d%02d-%02d%02d",
                 lctime->tm_year + 1900, lctime->tm_mon + 1, lctime->tm_mday,
                 lctime->tm_hour, lctime->tm_min);
        tag = datestring;
        cmsys::ofstream ofs(tagfile.c_str());
        if (ofs) {
          ofs << tag << std::endl;
          ofs << this->GetTestModelString() << std::endl;
          switch (this->Impl->TestModel) {
            case cmCTest::EXPERIMENTAL:
              ofs << "Experimental" << std::endl;
              break;
            case cmCTest::NIGHTLY:
              ofs << "Nightly" << std::endl;
              break;
            case cmCTest::CONTINUOUS:
              ofs << "Continuous" << std::endl;
              break;
          }
        }
        ofs.close();
        if (nullptr == command) {
          cmCTestOptionalLog(this, OUTPUT,
                             "Create new tag: " << tag << " - "
                                                << this->GetTestModelString()
                                                << std::endl,
                             quiet);
        }
      }
    } else {
      std::string group;
      std::string modelStr;
      int model = cmCTest::UNKNOWN;

      if (tfin) {
        cmSystemTools::GetLineFromStream(tfin, tag);
        cmSystemTools::GetLineFromStream(tfin, group);
        if (cmSystemTools::GetLineFromStream(tfin, modelStr)) {
          model = GetTestModelFromString(modelStr);
        }
        tfin.close();
      }

      if (tag.empty()) {
        cmCTestLog(this, ERROR_MESSAGE,
                   "Cannot read existing TAG file in " << testingDir
                                                       << std::endl);
        return 0;
      }

      if (this->Impl->TestModel == cmCTest::UNKNOWN) {
        if (model == cmCTest::UNKNOWN) {
          cmCTestLog(this, ERROR_MESSAGE,
                     "TAG file does not contain model and "
                     "no model specified in start command"
                       << std::endl);
          return 0;
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
                                                << this->GetTestModelString()
                                                << std::endl,
                         quiet);
    }

    this->Impl->CurrentTag = tag;
  }

  return 1;
}

bool cmCTest::InitializeFromCommand(cmCTestStartCommand* command)
{
  std::string src_dir = this->GetCTestConfiguration("SourceDirectory");
  std::string bld_dir = this->GetCTestConfiguration("BuildDirectory");
  this->Impl->BuildID = "";
  for (Part p = PartStart; p != PartCount; p = static_cast<Part>(p + 1)) {
    this->Impl->Parts[p].SubmitFiles.clear();
  }

  cmMakefile* mf = command->GetMakefile();
  std::string fname;

  std::string src_dir_fname = cmStrCat(src_dir, "/CTestConfig.cmake");
  cmSystemTools::ConvertToUnixSlashes(src_dir_fname);

  std::string bld_dir_fname = cmStrCat(bld_dir, "/CTestConfig.cmake");
  cmSystemTools::ConvertToUnixSlashes(bld_dir_fname);

  if (cmSystemTools::FileExists(bld_dir_fname)) {
    fname = bld_dir_fname;
  } else if (cmSystemTools::FileExists(src_dir_fname)) {
    fname = src_dir_fname;
  }

  if (!fname.empty()) {
    cmCTestOptionalLog(this, OUTPUT,
                       "   Reading ctest configuration file: " << fname
                                                               << std::endl,
                       command->ShouldBeQuiet());
    bool readit = mf->ReadDependentFile(fname);
    if (!readit) {
      std::string m = cmStrCat("Could not find include file: ", fname);
      command->SetError(m);
      return false;
    }
  }

  this->SetCTestConfigurationFromCMakeVariable(mf, "NightlyStartTime",
                                               "CTEST_NIGHTLY_START_TIME",
                                               command->ShouldBeQuiet());
  this->SetCTestConfigurationFromCMakeVariable(mf, "Site", "CTEST_SITE",
                                               command->ShouldBeQuiet());
  this->SetCTestConfigurationFromCMakeVariable(
    mf, "BuildName", "CTEST_BUILD_NAME", command->ShouldBeQuiet());

  if (!this->Initialize(bld_dir.c_str(), command)) {
    return false;
  }
  cmCTestOptionalLog(this, OUTPUT,
                     "   Use " << this->GetTestModelString() << " tag: "
                               << this->GetCurrentTag() << std::endl,
                     command->ShouldBeQuiet());
  return true;
}

bool cmCTest::UpdateCTestConfiguration()
{
  if (this->Impl->SuppressUpdatingCTestConfiguration) {
    return true;
  }
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
    cmSystemTools::ChangeDirectory(this->Impl->BinaryDir);
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

bool cmCTest::SetTest(const std::string& ttype, bool report)
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

void cmCTest::Finalize()
{
}

bool cmCTest::OpenOutputFile(const std::string& path, const std::string& name,
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
  stream.SetTempExt("tmp");
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

bool cmCTest::AddIfExists(Part part, const std::string& file)
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

bool cmCTest::CTestFileExists(const std::string& filename)
{
  std::string testingDir = this->Impl->BinaryDir + "/Testing/" +
    this->Impl->CurrentTag + "/" + filename;
  return cmSystemTools::FileExists(testingDir);
}

cmCTestBuildHandler* cmCTest::GetBuildHandler()
{
  return &this->Impl->BuildHandler;
}

cmCTestBuildAndTestHandler* cmCTest::GetBuildAndTestHandler()
{
  return &this->Impl->BuildAndTestHandler;
}

cmCTestCoverageHandler* cmCTest::GetCoverageHandler()
{
  return &this->Impl->CoverageHandler;
}

cmCTestScriptHandler* cmCTest::GetScriptHandler()
{
  return &this->Impl->ScriptHandler;
}

cmCTestTestHandler* cmCTest::GetTestHandler()
{
  return &this->Impl->TestHandler;
}

cmCTestUpdateHandler* cmCTest::GetUpdateHandler()
{
  return &this->Impl->UpdateHandler;
}

cmCTestConfigureHandler* cmCTest::GetConfigureHandler()
{
  return &this->Impl->ConfigureHandler;
}

cmCTestMemCheckHandler* cmCTest::GetMemCheckHandler()
{
  return &this->Impl->MemCheckHandler;
}

cmCTestSubmitHandler* cmCTest::GetSubmitHandler()
{
  return &this->Impl->SubmitHandler;
}

cmCTestUploadHandler* cmCTest::GetUploadHandler()
{
  return &this->Impl->UploadHandler;
}

int cmCTest::ProcessSteps()
{
  int res = 0;
  bool notest = true;
  int update_count = 0;

  for (Part p = PartStart; notest && p != PartCount;
       p = static_cast<Part>(p + 1)) {
    notest = !this->Impl->Parts[p];
  }
  if (this->Impl->Parts[PartUpdate] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    cmCTestUpdateHandler* uphandler = this->GetUpdateHandler();
    uphandler->SetPersistentOption(
      "SourceDirectory",
      this->GetCTestConfiguration("SourceDirectory").c_str());
    update_count = uphandler->ProcessHandler();
    if (update_count < 0) {
      res |= cmCTest::UPDATE_ERRORS;
    }
  }
  if (this->Impl->TestModel == cmCTest::CONTINUOUS && !update_count) {
    return 0;
  }
  if (this->Impl->Parts[PartConfigure] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    if (this->GetConfigureHandler()->ProcessHandler() < 0) {
      res |= cmCTest::CONFIGURE_ERRORS;
    }
  }
  if (this->Impl->Parts[PartBuild] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    if (this->GetBuildHandler()->ProcessHandler() < 0) {
      res |= cmCTest::BUILD_ERRORS;
    }
  }
  if ((this->Impl->Parts[PartTest] || notest) &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    if (this->GetTestHandler()->ProcessHandler() < 0) {
      res |= cmCTest::TEST_ERRORS;
    }
  }
  if (this->Impl->Parts[PartCoverage] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    if (this->GetCoverageHandler()->ProcessHandler() < 0) {
      res |= cmCTest::COVERAGE_ERRORS;
    }
  }
  if (this->Impl->Parts[PartMemCheck] &&
      (this->GetRemainingTimeAllowed() > std::chrono::minutes(2))) {
    this->UpdateCTestConfiguration();
    if (this->GetMemCheckHandler()->ProcessHandler() < 0) {
      res |= cmCTest::MEMORY_ERRORS;
    }
  }
  if (!notest) {
    std::string notes_dir = this->Impl->BinaryDir + "/Testing/Notes";
    if (cmSystemTools::FileIsDirectory(notes_dir)) {
      cmsys::Directory d;
      d.Load(notes_dir);
      unsigned long kk;
      for (kk = 0; kk < d.GetNumberOfFiles(); kk++) {
        const char* file = d.GetFile(kk);
        std::string fullname = notes_dir + "/" + file;
        if (cmSystemTools::FileExists(fullname) &&
            !cmSystemTools::FileIsDirectory(fullname)) {
          if (!this->Impl->NotesFiles.empty()) {
            this->Impl->NotesFiles += ";";
          }
          this->Impl->NotesFiles += fullname;
          this->Impl->Parts[PartNotes].Enable();
        }
      }
    }
  }
  if (this->Impl->Parts[PartNotes]) {
    this->UpdateCTestConfiguration();
    if (!this->Impl->NotesFiles.empty()) {
      this->GenerateNotesFile(this->Impl->NotesFiles);
    }
  }
  if (this->Impl->Parts[PartSubmit]) {
    this->UpdateCTestConfiguration();
    if (this->GetSubmitHandler()->ProcessHandler() < 0) {
      res |= cmCTest::SUBMIT_ERRORS;
    }
  }
  if (res != 0) {
    cmCTestLog(this, ERROR_MESSAGE, "Errors while running CTest" << std::endl);
    if (!this->Impl->OutputTestOutputOnTestFailure) {
      const std::string lastTestLog =
        this->GetBinaryDir() + "/Testing/Temporary/LastTest.log";
      cmCTestLog(this, ERROR_MESSAGE,
                 "Output from these tests are in: " << lastTestLog
                                                    << std::endl);
      cmCTestLog(this, ERROR_MESSAGE,
                 "Use \"--rerun-failed --output-on-failure\" to re-run the "
                 "failed cases verbosely."
                   << std::endl);
    }
  }
  return res;
}

std::string cmCTest::GetTestModelString()
{
  if (!this->Impl->SpecificGroup.empty()) {
    return this->Impl->SpecificGroup;
  }
  switch (this->Impl->TestModel) {
    case cmCTest::NIGHTLY:
      return "Nightly";
    case cmCTest::CONTINUOUS:
      return "Continuous";
  }
  return "Experimental";
}

int cmCTest::GetTestModelFromString(const std::string& str)
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

//######################################################################
//######################################################################
//######################################################################
//######################################################################

int cmCTest::RunMakeCommand(const std::string& command, std::string& output,
                            int* retVal, const char* dir, cmDuration timeout,
                            std::ostream& ofs, Encoding encoding)
{
  // First generate the command and arguments
  std::vector<std::string> args = cmSystemTools::ParseArguments(command);

  if (args.empty()) {
    return false;
  }

  std::vector<const char*> argv;
  argv.reserve(args.size() + 1);
  for (std::string const& a : args) {
    argv.push_back(a.c_str());
  }
  argv.push_back(nullptr);

  output.clear();
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Run command:");
  for (char const* arg : argv) {
    if (!arg) {
      break;
    }
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, " \"" << arg << "\"");
  }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, std::endl);

  // Now create process object
  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, argv.data());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  cmsysProcess_SetTimeout(cp, timeout.count());
  cmsysProcess_Execute(cp);

  // Initialize tick's
  std::string::size_type tick = 0;
  std::string::size_type tick_len = 1024;
  std::string::size_type tick_line_len = 50;

  char* data;
  int length;
  cmProcessOutput processOutput(encoding);
  std::string strdata;
  cmCTestLog(this, HANDLER_PROGRESS_OUTPUT,
             "   Each . represents " << tick_len
                                     << " bytes of output\n"
                                        "    "
                                     << std::flush);
  while (cmsysProcess_WaitForData(cp, &data, &length, nullptr)) {
    processOutput.DecodeText(data, length, strdata);
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
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
               cmCTestLogWrite(strdata.c_str(), strdata.size()));
    if (ofs) {
      ofs << cmCTestLogWrite(strdata.c_str(), strdata.size());
    }
  }
  processOutput.DecodeText(std::string(), strdata);
  if (!strdata.empty()) {
    output.append(strdata);
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
               cmCTestLogWrite(strdata.c_str(), strdata.size()));
    if (ofs) {
      ofs << cmCTestLogWrite(strdata.c_str(), strdata.size());
    }
  }
  cmCTestLog(this, HANDLER_PROGRESS_OUTPUT,
             " Size of output: " << int(double(output.size()) / 1024.0) << "K"
                                 << std::endl);

  cmsysProcess_WaitForExit(cp, nullptr);

  int result = cmsysProcess_GetState(cp);

  if (result == cmsysProcess_State_Exited) {
    *retVal = cmsysProcess_GetExitValue(cp);
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
               "Command exited with the value: " << *retVal << std::endl);
  } else if (result == cmsysProcess_State_Exception) {
    *retVal = cmsysProcess_GetExitException(cp);
    cmCTestLog(this, WARNING,
               "There was an exception: " << *retVal << std::endl);
  } else if (result == cmsysProcess_State_Expired) {
    cmCTestLog(this, WARNING, "There was a timeout" << std::endl);
  } else if (result == cmsysProcess_State_Error) {
    output += "\n*** ERROR executing: ";
    output += cmsysProcess_GetErrorString(cp);
    output += "\n***The build process failed.";
    cmCTestLog(this, ERROR_MESSAGE,
               "There was an error: " << cmsysProcess_GetErrorString(cp)
                                      << std::endl);
  }

  cmsysProcess_Delete(cp);

  return result;
}

//######################################################################
//######################################################################
//######################################################################
//######################################################################

int cmCTest::RunTest(std::vector<const char*> argv, std::string* output,
                     int* retVal, std::ostream* log, cmDuration testTimeOut,
                     std::vector<std::string>* environment, Encoding encoding)
{
  bool modifyEnv = (environment && !environment->empty());

  // determine how much time we have
  cmDuration timeout = this->GetRemainingTimeAllowed();
  if (timeout != cmCTest::MaxDuration()) {
    timeout -= std::chrono::minutes(2);
  }
  if (this->Impl->TimeOut > cmDuration::zero() &&
      this->Impl->TimeOut < timeout) {
    timeout = this->Impl->TimeOut;
  }
  if (testTimeOut > cmDuration::zero() &&
      testTimeOut < this->GetRemainingTimeAllowed()) {
    timeout = testTimeOut;
  }

  // always have at least 1 second if we got to here
  if (timeout <= cmDuration::zero()) {
    timeout = std::chrono::seconds(1);
  }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
             "Test timeout computed to be: "
               << (timeout == cmCTest::MaxDuration()
                     ? std::string("infinite")
                     : std::to_string(cmDurationTo<unsigned int>(timeout)))
               << "\n");
  if (cmSystemTools::SameFile(argv[0], cmSystemTools::GetCTestCommand()) &&
      !this->Impl->ForceNewCTestProcess) {
    cmCTest inst;
    inst.Impl->ConfigType = this->Impl->ConfigType;
    inst.Impl->TimeOut = timeout;

    // Capture output of the child ctest.
    std::ostringstream oss;
    inst.SetStreams(&oss, &oss);

    std::vector<std::string> args;
    for (char const* i : argv) {
      if (i) {
        // make sure we pass the timeout in for any build and test
        // invocations. Since --build-generator is required this is a
        // good place to check for it, and to add the arguments in
        if (strcmp(i, "--build-generator") == 0 &&
            timeout != cmCTest::MaxDuration() &&
            timeout > cmDuration::zero()) {
          args.emplace_back("--test-timeout");
          args.push_back(std::to_string(cmDurationTo<unsigned int>(timeout)));
        }
        args.emplace_back(i);
      }
    }
    if (log) {
      *log << "* Run internal CTest" << std::endl;
    }

    std::unique_ptr<cmSystemTools::SaveRestoreEnvironment> saveEnv;
    if (modifyEnv) {
      saveEnv = cm::make_unique<cmSystemTools::SaveRestoreEnvironment>();
      cmSystemTools::AppendEnv(*environment);
    }

    *retVal = inst.Run(args, output);
    if (output) {
      *output += oss.str();
    }
    if (log && output) {
      *log << *output;
    }
    if (output) {
      cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
                 "Internal cmCTest object used to run test." << std::endl
                                                             << *output
                                                             << std::endl);
    }

    return cmsysProcess_State_Exited;
  }
  std::vector<char> tempOutput;
  if (output) {
    output->clear();
  }

  std::unique_ptr<cmSystemTools::SaveRestoreEnvironment> saveEnv;
  if (modifyEnv) {
    saveEnv = cm::make_unique<cmSystemTools::SaveRestoreEnvironment>();
    cmSystemTools::AppendEnv(*environment);
  }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, argv.data());
  cmCTestLog(this, DEBUG, "Command is: " << argv[0] << std::endl);
  if (cmSystemTools::GetRunCommandHideConsole()) {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  }

  cmsysProcess_SetTimeout(cp, timeout.count());
  cmsysProcess_Execute(cp);

  char* data;
  int length;
  cmProcessOutput processOutput(encoding);
  std::string strdata;
  while (cmsysProcess_WaitForData(cp, &data, &length, nullptr)) {
    processOutput.DecodeText(data, length, strdata);
    if (output) {
      cm::append(tempOutput, data, data + length);
    }
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
               cmCTestLogWrite(strdata.c_str(), strdata.size()));
    if (log) {
      log->write(strdata.c_str(), strdata.size());
    }
  }
  processOutput.DecodeText(std::string(), strdata);
  if (!strdata.empty()) {
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
               cmCTestLogWrite(strdata.c_str(), strdata.size()));
    if (log) {
      log->write(strdata.c_str(), strdata.size());
    }
  }

  cmsysProcess_WaitForExit(cp, nullptr);
  processOutput.DecodeText(tempOutput, tempOutput);
  if (output && tempOutput.begin() != tempOutput.end()) {
    output->append(tempOutput.data(), tempOutput.size());
  }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
             "-- Process completed" << std::endl);

  int result = cmsysProcess_GetState(cp);

  if (result == cmsysProcess_State_Exited) {
    *retVal = cmsysProcess_GetExitValue(cp);
    if (*retVal != 0 && this->Impl->OutputTestOutputOnTestFailure) {
      this->OutputTestErrors(tempOutput);
    }
  } else if (result == cmsysProcess_State_Exception) {
    if (this->Impl->OutputTestOutputOnTestFailure) {
      this->OutputTestErrors(tempOutput);
    }
    *retVal = cmsysProcess_GetExitException(cp);
    std::string outerr = cmStrCat("\n*** Exception executing: ",
                                  cmsysProcess_GetExceptionString(cp));
    if (output) {
      *output += outerr;
    }
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, outerr << std::endl);
  } else if (result == cmsysProcess_State_Error) {
    std::string outerr =
      cmStrCat("\n*** ERROR executing: ", cmsysProcess_GetErrorString(cp));
    if (output) {
      *output += outerr;
    }
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, outerr << std::endl);
  }
  cmsysProcess_Delete(cp);

  return result;
}

std::string cmCTest::SafeBuildIdField(const std::string& value)
{
  std::string safevalue(value);

  if (!safevalue.empty()) {
    // Disallow non-filename and non-space whitespace characters.
    // If they occur, replace them with ""
    //
    const char* disallowed = "\\:*?\"<>|\n\r\t\f\v";

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

void cmCTest::StartXML(cmXMLWriter& xml, bool append)
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
                                                this->GetTestModelString());
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

  this->AddSiteProperties(xml);
}

void cmCTest::AddSiteProperties(cmXMLWriter& xml)
{
  cmCTestScriptHandler* ch = this->GetScriptHandler();
  cmake* cm = ch->GetCMake();
  // if no CMake then this is the old style script and props like
  // this will not work anyway.
  if (!cm) {
    return;
  }
  // This code should go when cdash is changed to use labels only
  cmValue subproject = cm->GetState()->GetGlobalProperty("SubProject");
  if (subproject) {
    xml.StartElement("Subproject");
    xml.Attribute("name", *subproject);
    cmValue labels =
      ch->GetCMake()->GetState()->GetGlobalProperty("SubProjectLabels");
    if (labels) {
      xml.StartElement("Labels");
      std::vector<std::string> args = cmExpandedList(*labels);
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
  std::vector<std::string> subprojects = cmExpandedList(labelsForSubprojects);

  // sort the array
  std::sort(subprojects.begin(), subprojects.end());
  // remove duplicates
  auto new_end = std::unique(subprojects.begin(), subprojects.end());
  subprojects.erase(new_end, subprojects.end());

  return subprojects;
}

void cmCTest::EndXML(cmXMLWriter& xml)
{
  xml.EndElement(); // Site
  xml.EndDocument();
}

int cmCTest::GenerateCTestNotesOutput(cmXMLWriter& xml,
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
                this->Impl->CurrentTag + "-" + this->GetTestModelString());
  xml.Attribute("Name", this->GetCTestConfiguration("Site"));
  xml.Attribute("Generator",
                std::string("ctest-") + cmVersion::GetCMakeVersion());
  this->AddSiteProperties(xml);
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

int cmCTest::GenerateNotesFile(std::vector<std::string> const& files)
{
  cmGeneratedFileStream ofs;
  if (!this->OpenOutputFile(this->Impl->CurrentTag, "Notes.xml", ofs)) {
    cmCTestLog(this, ERROR_MESSAGE, "Cannot open notes file" << std::endl);
    return 1;
  }
  cmXMLWriter xml(ofs);
  this->GenerateCTestNotesOutput(xml, files);
  return 0;
}

int cmCTest::GenerateNotesFile(const std::string& cfiles)
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

  return this->GenerateNotesFile(files);
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

bool cmCTest::TryToChangeDirectory(std::string const& dir)
{
  cmCTestLog(this, OUTPUT,
             "Internal ctest changing into directory: " << dir << std::endl);
  cmsys::Status status = cmSystemTools::ChangeDirectory(dir);
  if (!status) {
    auto msg = "Failed to change working directory to \"" + dir +
      "\" : " + status.GetString() + "\n";
    cmCTestLog(this, ERROR_MESSAGE, msg);
    return false;
  }
  return true;
}

std::string cmCTest::Base64GzipEncodeFile(std::string const& file)
{
  const std::string currDir = cmSystemTools::GetCurrentWorkingDirectory();
  std::string parentDir = cmSystemTools::GetParentDirectory(file);

  // Temporarily change to the file's directory so the tar gets created
  // with a flat directory structure.
  if (currDir != parentDir) {
    if (!this->TryToChangeDirectory(parentDir)) {
      return "";
    }
  }

  std::string tarFile = file + "_temp.tar.gz";
  std::vector<std::string> files;
  files.push_back(file);

  if (!cmSystemTools::CreateTar(tarFile, files, cmSystemTools::TarCompressGZip,
                                false)) {
    cmCTestLog(this, ERROR_MESSAGE,
               "Error creating tar while "
               "encoding file: "
                 << file << std::endl);
    return "";
  }
  std::string base64 = this->Base64EncodeFile(tarFile);
  cmSystemTools::RemoveFile(tarFile);

  // Change back to the directory we started in.
  if (currDir != parentDir) {
    cmSystemTools::ChangeDirectory(currDir);
  }

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
                                            << std::endl;);
      return false;
    }
    this->AddSubmitFile(PartExtraFiles, file);
  }
  return true;
}

bool cmCTest::SubmitExtraFiles(const std::string& cfiles)
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
bool cmCTest::AddTestsForDashboardType(std::string& targ)
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

void cmCTest::ErrorMessageUnknownDashDValue(std::string& val)
{
  cmCTestLog(this, ERROR_MESSAGE,
             "CTest -D called with incorrect option: " << val << std::endl);

  cmCTestLog(
    this, ERROR_MESSAGE,
    "Available options are:"
      << std::endl
      << "  ctest -D Continuous" << std::endl
      << "  ctest -D Continuous(Start|Update|Configure|Build)" << std::endl
      << "  ctest -D Continuous(Test|Coverage|MemCheck|Submit)" << std::endl
      << "  ctest -D Experimental" << std::endl
      << "  ctest -D Experimental(Start|Update|Configure|Build)" << std::endl
      << "  ctest -D Experimental(Test|Coverage|MemCheck|Submit)" << std::endl
      << "  ctest -D Nightly" << std::endl
      << "  ctest -D Nightly(Start|Update|Configure|Build)" << std::endl
      << "  ctest -D Nightly(Test|Coverage|MemCheck|Submit)" << std::endl
      << "  ctest -D NightlyMemoryCheck" << std::endl);
}

bool cmCTest::CheckArgument(const std::string& arg, cm::string_view varg1,
                            const char* varg2)
{
  return (arg == varg1) || (varg2 && arg == varg2);
}

// Processes one command line argument (and its arguments if any)
// for many simple options and then returns
bool cmCTest::HandleCommandLineArguments(size_t& i,
                                         std::vector<std::string>& args,
                                         std::string& errormsg)
{
  std::string arg = args[i];
  if (this->CheckArgument(arg, "-F"_s)) {
    this->Impl->Failover = true;
  } else if (this->CheckArgument(arg, "-j"_s, "--parallel") &&
             i < args.size() - 1) {
    i++;
    int plevel = atoi(args[i].c_str());
    this->SetParallelLevel(plevel);
    this->Impl->ParallelLevelSetInCli = true;
  } else if (cmHasPrefix(arg, "-j")) {
    int plevel = atoi(arg.substr(2).c_str());
    this->SetParallelLevel(plevel);
    this->Impl->ParallelLevelSetInCli = true;
  }

  else if (this->CheckArgument(arg, "--repeat-until-fail"_s)) {
    if (i >= args.size() - 1) {
      errormsg = "'--repeat-until-fail' requires an argument";
      return false;
    }
    if (this->Impl->RepeatMode != cmCTest::Repeat::Never) {
      errormsg = "At most one '--repeat' option may be used.";
      return false;
    }
    i++;
    long repeat = 1;
    if (!cmStrToLong(args[i], &repeat)) {
      errormsg = cmStrCat("'--repeat-until-fail' given non-integer value '",
                          args[i], "'");
      return false;
    }
    this->Impl->RepeatCount = static_cast<int>(repeat);
    if (repeat > 1) {
      this->Impl->RepeatMode = cmCTest::Repeat::UntilFail;
    }
  }

  else if (this->CheckArgument(arg, "--repeat"_s)) {
    if (i >= args.size() - 1) {
      errormsg = "'--repeat' requires an argument";
      return false;
    }
    if (this->Impl->RepeatMode != cmCTest::Repeat::Never) {
      errormsg = "At most one '--repeat' option may be used.";
      return false;
    }
    i++;
    cmsys::RegularExpression repeatRegex(
      "^(until-fail|until-pass|after-timeout):([0-9]+)$");
    if (repeatRegex.find(args[i])) {
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
      errormsg = cmStrCat("'--repeat' given invalid value '", args[i], "'");
      return false;
    }
  }

  else if (this->CheckArgument(arg, "--test-load"_s) && i < args.size() - 1) {
    i++;
    unsigned long load;
    if (cmStrToULong(args[i], &load)) {
      this->SetTestLoad(load);
    } else {
      cmCTestLog(this, WARNING,
                 "Invalid value for 'Test Load' : " << args[i] << std::endl);
    }
  }

  else if (this->CheckArgument(arg, "--no-compress-output"_s)) {
    this->Impl->CompressTestOutput = false;
  }

  else if (this->CheckArgument(arg, "--print-labels"_s)) {
    this->Impl->PrintLabels = true;
  }

  else if (this->CheckArgument(arg, "--http1.0"_s)) {
    this->Impl->UseHTTP10 = true;
  }

  else if (this->CheckArgument(arg, "--timeout"_s) && i < args.size() - 1) {
    i++;
    auto timeout = cmDuration(atof(args[i].c_str()));
    this->Impl->GlobalTimeout = timeout;
  }

  else if (this->CheckArgument(arg, "--stop-time"_s) && i < args.size() - 1) {
    i++;
    this->SetStopTime(args[i]);
  }

  else if (this->CheckArgument(arg, "--stop-on-failure"_s)) {
    this->Impl->StopOnFailure = true;
  }

  else if (this->CheckArgument(arg, "-C"_s, "--build-config") &&
           i < args.size() - 1) {
    i++;
    this->SetConfigType(args[i]);
  }

  else if (this->CheckArgument(arg, "--debug"_s)) {
    this->Impl->Debug = true;
    this->Impl->ShowLineNumbers = true;
  } else if ((this->CheckArgument(arg, "--group"_s) ||
              // This is an undocumented / deprecated option.
              // "Track" has been renamed to "Group".
              this->CheckArgument(arg, "--track"_s)) &&
             i < args.size() - 1) {
    i++;
    this->Impl->SpecificGroup = args[i];
  } else if (this->CheckArgument(arg, "--show-line-numbers"_s)) {
    this->Impl->ShowLineNumbers = true;
  } else if (this->CheckArgument(arg, "--no-label-summary"_s)) {
    this->Impl->LabelSummary = false;
  } else if (this->CheckArgument(arg, "--no-subproject-summary"_s)) {
    this->Impl->SubprojectSummary = false;
  } else if (this->CheckArgument(arg, "-Q"_s, "--quiet")) {
    this->Impl->Quiet = true;
  } else if (this->CheckArgument(arg, "--progress"_s)) {
    this->Impl->TestProgressOutput = true;
  } else if (this->CheckArgument(arg, "-V"_s, "--verbose")) {
    this->Impl->Verbose = true;
  } else if (this->CheckArgument(arg, "-VV"_s, "--extra-verbose")) {
    this->Impl->ExtraVerbose = true;
    this->Impl->Verbose = true;
  } else if (this->CheckArgument(arg, "--output-on-failure"_s)) {
    this->Impl->OutputTestOutputOnTestFailure = true;
  } else if (this->CheckArgument(arg, "--test-output-size-passed"_s) &&
             i < args.size() - 1) {
    i++;
    long outputSize;
    if (cmStrToLong(args[i], &outputSize)) {
      this->Impl->TestHandler.SetTestOutputSizePassed(
        static_cast<int>(outputSize));
    } else {
      cmCTestLog(this, WARNING,
                 "Invalid value for '--test-output-size-passed': " << args[i]
                                                                   << "\n");
    }
  } else if (this->CheckArgument(arg, "--test-output-size-failed"_s) &&
             i < args.size() - 1) {
    i++;
    long outputSize;
    if (cmStrToLong(args[i], &outputSize)) {
      this->Impl->TestHandler.SetTestOutputSizeFailed(
        static_cast<int>(outputSize));
    } else {
      cmCTestLog(this, WARNING,
                 "Invalid value for '--test-output-size-failed': " << args[i]
                                                                   << "\n");
    }
  } else if (this->CheckArgument(arg, "--test-output-truncation"_s) &&
             i < args.size() - 1) {
    i++;
    if (!this->Impl->TestHandler.SetTestOutputTruncation(args[i])) {
      errormsg = "Invalid value for '--test-output-truncation': " + args[i];
      return false;
    }
  } else if (this->CheckArgument(arg, "-N"_s, "--show-only")) {
    this->Impl->ShowOnly = true;
  } else if (cmHasLiteralPrefix(arg, "--show-only=")) {
    this->Impl->ShowOnly = true;

    // Check if a specific format is requested. Defaults to human readable
    // text.
    std::string argWithFormat = "--show-only=";
    std::string format = arg.substr(argWithFormat.length());
    if (format == "json-v1") {
      // Force quiet mode so the only output is the json object model.
      this->Impl->Quiet = true;
      this->Impl->OutputAsJson = true;
      this->Impl->OutputAsJsonVersion = 1;
    } else if (format != "human") {
      errormsg = "'--show-only=' given unknown value '" + format + "'";
      return false;
    }
  }

  else if (this->CheckArgument(arg, "-O"_s, "--output-log") &&
           i < args.size() - 1) {
    i++;
    this->SetOutputLogFileName(args[i]);
  }

  else if (this->CheckArgument(arg, "--tomorrow-tag"_s)) {
    this->Impl->TomorrowTag = true;
  } else if (this->CheckArgument(arg, "--force-new-ctest-process"_s)) {
    this->Impl->ForceNewCTestProcess = true;
  } else if (this->CheckArgument(arg, "-W"_s, "--max-width") &&
             i < args.size() - 1) {
    i++;
    this->Impl->MaxTestNameWidth = atoi(args[i].c_str());
  } else if (this->CheckArgument(arg, "--interactive-debug-mode"_s) &&
             i < args.size() - 1) {
    i++;
    this->Impl->InteractiveDebugMode = cmIsOn(args[i]);
  } else if (this->CheckArgument(arg, "--submit-index"_s) &&
             i < args.size() - 1) {
    i++;
    this->Impl->SubmitIndex = atoi(args[i].c_str());
    if (this->Impl->SubmitIndex < 0) {
      this->Impl->SubmitIndex = 0;
    }
  }

  else if (this->CheckArgument(arg, "--overwrite"_s) && i < args.size() - 1) {
    i++;
    this->AddCTestConfigurationOverwrite(args[i]);
  } else if (this->CheckArgument(arg, "-A"_s, "--add-notes") &&
             i < args.size() - 1) {
    this->Impl->ProduceXML = true;
    this->SetTest("Notes");
    i++;
    this->SetNotesFiles(args[i]);
    return true;
  } else if (this->CheckArgument(arg, "--test-dir"_s)) {
    if (i >= args.size() - 1) {
      errormsg = "'--test-dir' requires an argument";
      return false;
    }
    i++;
    this->Impl->TestDir = std::string(args[i]);
  } else if (this->CheckArgument(arg, "--output-junit"_s)) {
    if (i >= args.size() - 1) {
      errormsg = "'--output-junit' requires an argument";
      return false;
    }
    i++;
    this->Impl->TestHandler.SetJUnitXMLFileName(std::string(args[i]));
    // Turn test output compression off.
    // This makes it easier to include test output in the resulting
    // JUnit XML report.
    this->Impl->CompressTestOutput = false;
  }

  cm::string_view noTestsPrefix = "--no-tests=";
  if (cmHasPrefix(arg, noTestsPrefix)) {
    cm::string_view noTestsMode =
      cm::string_view(arg).substr(noTestsPrefix.length());
    if (noTestsMode == "error") {
      this->Impl->NoTestsMode = cmCTest::NoTests::Error;
    } else if (noTestsMode != "ignore") {
      errormsg =
        cmStrCat("'--no-tests=' given unknown value '", noTestsMode, '\'');
      return false;
    } else {
      this->Impl->NoTestsMode = cmCTest::NoTests::Ignore;
    }
  }

  // options that control what tests are run
  else if (this->CheckArgument(arg, "-I"_s, "--tests-information") &&
           i < args.size() - 1) {
    i++;
    this->GetTestHandler()->SetPersistentOption("TestsToRunInformation",
                                                args[i].c_str());
    this->GetMemCheckHandler()->SetPersistentOption("TestsToRunInformation",
                                                    args[i].c_str());
  } else if (this->CheckArgument(arg, "-U"_s, "--union")) {
    this->GetTestHandler()->SetPersistentOption("UseUnion", "true");
    this->GetMemCheckHandler()->SetPersistentOption("UseUnion", "true");
  } else if (this->CheckArgument(arg, "-R"_s, "--tests-regex") &&
             i < args.size() - 1) {
    i++;
    this->GetTestHandler()->SetPersistentOption("IncludeRegularExpression",
                                                args[i].c_str());
    this->GetMemCheckHandler()->SetPersistentOption("IncludeRegularExpression",
                                                    args[i].c_str());
  } else if (this->CheckArgument(arg, "-L"_s, "--label-regex") &&
             i < args.size() - 1) {
    i++;
    this->GetTestHandler()->AddPersistentMultiOption("LabelRegularExpression",
                                                     args[i]);
    this->GetMemCheckHandler()->AddPersistentMultiOption(
      "LabelRegularExpression", args[i]);
  } else if (this->CheckArgument(arg, "-LE"_s, "--label-exclude") &&
             i < args.size() - 1) {
    i++;
    this->GetTestHandler()->AddPersistentMultiOption(
      "ExcludeLabelRegularExpression", args[i]);
    this->GetMemCheckHandler()->AddPersistentMultiOption(
      "ExcludeLabelRegularExpression", args[i]);
  }

  else if (this->CheckArgument(arg, "-E"_s, "--exclude-regex") &&
           i < args.size() - 1) {
    i++;
    this->GetTestHandler()->SetPersistentOption("ExcludeRegularExpression",
                                                args[i].c_str());
    this->GetMemCheckHandler()->SetPersistentOption("ExcludeRegularExpression",
                                                    args[i].c_str());
  }

  else if (this->CheckArgument(arg, "-FA"_s, "--fixture-exclude-any") &&
           i < args.size() - 1) {
    i++;
    this->GetTestHandler()->SetPersistentOption(
      "ExcludeFixtureRegularExpression", args[i].c_str());
    this->GetMemCheckHandler()->SetPersistentOption(
      "ExcludeFixtureRegularExpression", args[i].c_str());
  } else if (this->CheckArgument(arg, "-FS"_s, "--fixture-exclude-setup") &&
             i < args.size() - 1) {
    i++;
    this->GetTestHandler()->SetPersistentOption(
      "ExcludeFixtureSetupRegularExpression", args[i].c_str());
    this->GetMemCheckHandler()->SetPersistentOption(
      "ExcludeFixtureSetupRegularExpression", args[i].c_str());
  } else if (this->CheckArgument(arg, "-FC"_s, "--fixture-exclude-cleanup") &&
             i < args.size() - 1) {
    i++;
    this->GetTestHandler()->SetPersistentOption(
      "ExcludeFixtureCleanupRegularExpression", args[i].c_str());
    this->GetMemCheckHandler()->SetPersistentOption(
      "ExcludeFixtureCleanupRegularExpression", args[i].c_str());
  }

  else if (this->CheckArgument(arg, "--resource-spec-file"_s) &&
           i < args.size() - 1) {
    i++;
    this->GetTestHandler()->SetPersistentOption("ResourceSpecFile",
                                                args[i].c_str());
    this->GetMemCheckHandler()->SetPersistentOption("ResourceSpecFile",
                                                    args[i].c_str());
  }

  else if (this->CheckArgument(arg, "--rerun-failed"_s)) {
    this->GetTestHandler()->SetPersistentOption("RerunFailed", "true");
    this->GetMemCheckHandler()->SetPersistentOption("RerunFailed", "true");
  }
  return true;
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
#if defined(_WIN32)
  // Not supported on Windows
  return false;
#else
  // On UNIX we need a non-dumb tty.
  std::string clicolor_force;
  if (cmSystemTools::GetEnv("CLICOLOR_FORCE", clicolor_force) &&
      !clicolor_force.empty() && clicolor_force != "0") {
    return true;
  }
  std::string clicolor;
  if (cmSystemTools::GetEnv("CLICOLOR", clicolor) && clicolor == "0") {
    return false;
  }
  return ConsoleIsNotDumb();
#endif
}

// handle the -S -SR and -SP arguments
void cmCTest::HandleScriptArguments(size_t& i, std::vector<std::string>& args,
                                    bool& SRArgumentSpecified)
{
  std::string arg = args[i];
  if (this->CheckArgument(arg, "-SP"_s, "--script-new-process") &&
      i < args.size() - 1) {
    this->Impl->RunConfigurationScript = true;
    i++;
    cmCTestScriptHandler* ch = this->GetScriptHandler();
    // -SR is an internal argument, -SP should be ignored when it is passed
    if (!SRArgumentSpecified) {
      ch->AddConfigurationScript(args[i], false);
    }
  }

  if (this->CheckArgument(arg, "-SR"_s, "--script-run") &&
      i < args.size() - 1) {
    SRArgumentSpecified = true;
    this->Impl->RunConfigurationScript = true;
    i++;
    cmCTestScriptHandler* ch = this->GetScriptHandler();
    ch->AddConfigurationScript(args[i], true);
  }

  if (this->CheckArgument(arg, "-S"_s, "--script") && i < args.size() - 1) {
    this->Impl->RunConfigurationScript = true;
    i++;
    cmCTestScriptHandler* ch = this->GetScriptHandler();
    // -SR is an internal argument, -S should be ignored when it is passed
    if (!SRArgumentSpecified) {
      ch->AddConfigurationScript(args[i], true);
    }
  }
}

bool cmCTest::AddVariableDefinition(const std::string& arg)
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

void cmCTest::SetPersistentOptionIfNotEmpty(const std::string& value,
                                            const std::string& optionName)
{
  if (!value.empty()) {
    this->GetTestHandler()->SetPersistentOption(optionName, value.c_str());
    this->GetMemCheckHandler()->SetPersistentOption(optionName, value.c_str());
  }
}

void cmCTest::AddPersistentMultiOptionIfNotEmpty(const std::string& value,
                                                 const std::string& optionName)
{
  if (!value.empty()) {
    this->GetTestHandler()->AddPersistentMultiOption(optionName, value);
    this->GetMemCheckHandler()->AddPersistentMultiOption(optionName, value);
  }
}

bool cmCTest::SetArgsFromPreset(const std::string& presetName,
                                bool listPresets)
{
  const auto workingDirectory = cmSystemTools::GetCurrentWorkingDirectory();

  cmCMakePresetsGraph settingsFile;
  auto result = settingsFile.ReadProjectPresets(workingDirectory);
  if (result != cmCMakePresetsGraph::ReadFileResult::READ_OK) {
    cmSystemTools::Error(
      cmStrCat("Could not read presets from ", workingDirectory, ": ",
               cmCMakePresetsGraph::ResultToString(result)));
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
  for (const auto& kvp : expandedPreset->OverwriteConfigurationFile) {
    this->AddCTestConfigurationOverwrite(kvp);
  }

  if (expandedPreset->Output) {
    this->Impl->TestProgressOutput =
      expandedPreset->Output->ShortProgress.value_or(false);

    if (expandedPreset->Output->Verbosity) {
      const auto& verbosity = *expandedPreset->Output->Verbosity;
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
    this->Impl->ShowLineNumbers =
      expandedPreset->Output->Debug.value_or(false);
    this->Impl->OutputTestOutputOnTestFailure =
      expandedPreset->Output->OutputOnFailure.value_or(false);
    this->Impl->Quiet = expandedPreset->Output->Quiet.value_or(false);

    if (!expandedPreset->Output->OutputLogFile.empty()) {
      this->SetOutputLogFileName(expandedPreset->Output->OutputLogFile);
    }

    this->Impl->LabelSummary =
      expandedPreset->Output->LabelSummary.value_or(true);
    this->Impl->SubprojectSummary =
      expandedPreset->Output->SubprojectSummary.value_or(true);

    if (expandedPreset->Output->MaxPassedTestOutputSize) {
      this->Impl->TestHandler.SetTestOutputSizePassed(
        *expandedPreset->Output->MaxPassedTestOutputSize);
    }

    if (expandedPreset->Output->MaxFailedTestOutputSize) {
      this->Impl->TestHandler.SetTestOutputSizeFailed(
        *expandedPreset->Output->MaxFailedTestOutputSize);
    }

    if (expandedPreset->Output->TestOutputTruncation) {
      this->Impl->TestHandler.TestOutputTruncation =
        *expandedPreset->Output->TestOutputTruncation;
    }

    if (expandedPreset->Output->MaxTestNameWidth) {
      this->Impl->MaxTestNameWidth = *expandedPreset->Output->MaxTestNameWidth;
    }
  }

  if (expandedPreset->Filter) {
    if (expandedPreset->Filter->Include) {
      this->SetPersistentOptionIfNotEmpty(
        expandedPreset->Filter->Include->Name, "IncludeRegularExpression");
      this->AddPersistentMultiOptionIfNotEmpty(
        expandedPreset->Filter->Include->Label, "LabelRegularExpression");

      if (expandedPreset->Filter->Include->Index) {
        if (expandedPreset->Filter->Include->Index->IndexFile.empty()) {
          const auto& start = expandedPreset->Filter->Include->Index->Start;
          const auto& end = expandedPreset->Filter->Include->Index->End;
          const auto& stride = expandedPreset->Filter->Include->Index->Stride;
          std::string indexOptions;
          indexOptions += (start ? std::to_string(*start) : "") + ",";
          indexOptions += (end ? std::to_string(*end) : "") + ",";
          indexOptions += (stride ? std::to_string(*stride) : "") + ",";
          indexOptions +=
            cmJoin(expandedPreset->Filter->Include->Index->SpecificTests, ",");

          this->SetPersistentOptionIfNotEmpty(indexOptions,
                                              "TestsToRunInformation");
        } else {
          this->SetPersistentOptionIfNotEmpty(
            expandedPreset->Filter->Include->Index->IndexFile,
            "TestsToRunInformation");
        }
      }

      if (expandedPreset->Filter->Include->UseUnion.value_or(false)) {
        this->GetTestHandler()->SetPersistentOption("UseUnion", "true");
        this->GetMemCheckHandler()->SetPersistentOption("UseUnion", "true");
      }
    }

    if (expandedPreset->Filter->Exclude) {
      this->SetPersistentOptionIfNotEmpty(
        expandedPreset->Filter->Exclude->Name, "ExcludeRegularExpression");
      this->AddPersistentMultiOptionIfNotEmpty(
        expandedPreset->Filter->Exclude->Label,
        "ExcludeLabelRegularExpression");

      if (expandedPreset->Filter->Exclude->Fixtures) {
        this->SetPersistentOptionIfNotEmpty(
          expandedPreset->Filter->Exclude->Fixtures->Any,
          "ExcludeFixtureRegularExpression");
        this->SetPersistentOptionIfNotEmpty(
          expandedPreset->Filter->Exclude->Fixtures->Setup,
          "ExcludeFixtureSetupRegularExpression");
        this->SetPersistentOptionIfNotEmpty(
          expandedPreset->Filter->Exclude->Fixtures->Cleanup,
          "ExcludeFixtureCleanupRegularExpression");
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

    this->SetPersistentOptionIfNotEmpty(
      expandedPreset->Execution->ResourceSpecFile, "ResourceSpecFile");

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
int cmCTest::Run(std::vector<std::string>& args, std::string* output)
{
  const char* ctestExec = "ctest";
  bool cmakeAndTest = false;
  bool executeTests = true;
  bool SRArgumentSpecified = false;

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
        auto presetName = it->substr(9);
        success = this->SetArgsFromPreset(presetName, listPresets);
      } else if (++it != args.end()) {
        auto presetName = *it;
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

  // process the command line arguments
  for (size_t i = 1; i < args.size(); ++i) {
    // handle the simple commandline arguments
    std::string errormsg;
    if (!this->HandleCommandLineArguments(i, args, errormsg)) {
      cmSystemTools::Error(errormsg);
      return 1;
    }

    // handle the script arguments -S -SR -SP
    this->HandleScriptArguments(i, args, SRArgumentSpecified);

    // --dashboard: handle a request for a dashboard
    std::string arg = args[i];
    if (this->CheckArgument(arg, "-D"_s, "--dashboard") &&
        i < args.size() - 1) {
      this->Impl->ProduceXML = true;
      i++;
      std::string targ = args[i];
      // AddTestsForDashboard parses the dashboard type and converts it
      // into the separate stages
      if (!this->AddTestsForDashboardType(targ)) {
        if (!this->AddVariableDefinition(targ)) {
          this->ErrorMessageUnknownDashDValue(targ);
          executeTests = false;
        }
      }
    }

    // If it's not exactly -D, but it starts with -D, then try to parse out
    // a variable definition from it, same as CMake does. Unsuccessful
    // attempts are simply ignored since previous ctest versions ignore
    // this too. (As well as many other unknown command line args.)
    //
    if (arg != "-D" && cmHasLiteralPrefix(arg, "-D")) {
      std::string input = arg.substr(2);
      this->AddVariableDefinition(input);
    }

    // --test-action: calls SetTest(<stage>, /*report=*/ false) to enable
    // the corresponding stage
    if (!this->HandleTestActionArgument(ctestExec, i, args)) {
      executeTests = false;
    }

    // --test-model: what type of test model
    if (!this->HandleTestModelArgument(ctestExec, i, args)) {
      executeTests = false;
    }

    // --extra-submit
    if (this->CheckArgument(arg, "--extra-submit"_s) && i < args.size() - 1) {
      this->Impl->ProduceXML = true;
      this->SetTest("Submit");
      i++;
      if (!this->SubmitExtraFiles(args[i])) {
        return 0;
      }
    }

    // --build-and-test options
    if (this->CheckArgument(arg, "--build-and-test"_s) &&
        i < args.size() - 1) {
      cmakeAndTest = true;
    }

    // --schedule-random
    if (this->CheckArgument(arg, "--schedule-random"_s)) {
      this->Impl->ScheduleType = "Random";
    }

    // pass the argument to all the handlers as well, but it may no longer be
    // set to what it was originally so I'm not sure this is working as
    // intended
    for (auto& handler : this->Impl->GetTestingHandlers()) {
      if (!handler->ProcessCommandLineArguments(arg, i, args)) {
        cmCTestLog(this, ERROR_MESSAGE,
                   "Problem parsing command line arguments within a handler");
        return 0;
      }
    }
  } // the close of the for argument loop

  // handle CTEST_PARALLEL_LEVEL environment variable
  if (!this->Impl->ParallelLevelSetInCli) {
    std::string parallel;
    if (cmSystemTools::GetEnv("CTEST_PARALLEL_LEVEL", parallel)) {
      int plevel = atoi(parallel.c_str());
      this->SetParallelLevel(plevel);
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
    return this->RunCMakeAndTest(output);
  }

  if (executeTests) {
    return this->ExecuteTests();
  }

  return 1;
}

bool cmCTest::HandleTestActionArgument(const char* ctestExec, size_t& i,
                                       const std::vector<std::string>& args)
{
  bool success = true;
  std::string const& arg = args[i];
  if (this->CheckArgument(arg, "-T"_s, "--test-action") &&
      (i < args.size() - 1)) {
    this->Impl->ProduceXML = true;
    i++;
    if (!this->SetTest(args[i], false)) {
      success = false;
      cmCTestLog(this, ERROR_MESSAGE,
                 "CTest -T called with incorrect option: " << args[i]
                                                           << std::endl);
      cmCTestLog(this, ERROR_MESSAGE,
                 "Available options are:"
                   << std::endl
                   << "  " << ctestExec << " -T all" << std::endl
                   << "  " << ctestExec << " -T start" << std::endl
                   << "  " << ctestExec << " -T update" << std::endl
                   << "  " << ctestExec << " -T configure" << std::endl
                   << "  " << ctestExec << " -T build" << std::endl
                   << "  " << ctestExec << " -T test" << std::endl
                   << "  " << ctestExec << " -T coverage" << std::endl
                   << "  " << ctestExec << " -T memcheck" << std::endl
                   << "  " << ctestExec << " -T notes" << std::endl
                   << "  " << ctestExec << " -T submit" << std::endl);
    }
  }
  return success;
}

bool cmCTest::HandleTestModelArgument(const char* ctestExec, size_t& i,
                                      const std::vector<std::string>& args)
{
  bool success = true;
  std::string const& arg = args[i];
  if (this->CheckArgument(arg, "-M"_s, "--test-model") &&
      (i < args.size() - 1)) {
    i++;
    std::string const& str = args[i];
    if (cmSystemTools::LowerCase(str) == "nightly"_s) {
      this->SetTestModel(cmCTest::NIGHTLY);
    } else if (cmSystemTools::LowerCase(str) == "continuous"_s) {
      this->SetTestModel(cmCTest::CONTINUOUS);
    } else if (cmSystemTools::LowerCase(str) == "experimental"_s) {
      this->SetTestModel(cmCTest::EXPERIMENTAL);
    } else {
      success = false;
      cmCTestLog(this, ERROR_MESSAGE,
                 "CTest -M called with incorrect option: " << str
                                                           << std::endl);
      cmCTestLog(this, ERROR_MESSAGE,
                 "Available options are:"
                   << std::endl
                   << "  " << ctestExec << " -M Continuous" << std::endl
                   << "  " << ctestExec << " -M Experimental" << std::endl
                   << "  " << ctestExec << " -M Nightly" << std::endl);
    }
  }
  return success;
}

int cmCTest::ExecuteTests()
{
  int res;
  // call process directory
  if (this->Impl->RunConfigurationScript) {
    if (this->Impl->ExtraVerbose) {
      cmCTestLog(this, OUTPUT, "* Extra verbosity turned on" << std::endl);
    }
    for (auto& handler : this->Impl->GetTestingHandlers()) {
      handler->SetVerbose(this->Impl->ExtraVerbose);
      handler->SetSubmitIndex(this->Impl->SubmitIndex);
    }
    this->GetScriptHandler()->SetVerbose(this->Impl->Verbose);
    res = this->GetScriptHandler()->ProcessHandler();
    if (res != 0) {
      cmCTestLog(this, DEBUG,
                 "running script failing returning: " << res << std::endl);
    }

  } else {
    // What is this?  -V seems to be the same as -VV,
    // and Verbose is always on in this case
    this->Impl->ExtraVerbose = this->Impl->Verbose;
    this->Impl->Verbose = true;
    for (auto& handler : this->Impl->GetTestingHandlers()) {
      handler->SetVerbose(this->Impl->Verbose);
      handler->SetSubmitIndex(this->Impl->SubmitIndex);
    }

    const std::string currDir = cmSystemTools::GetCurrentWorkingDirectory();
    std::string workDir = currDir;
    if (!this->Impl->TestDir.empty()) {
      workDir = cmSystemTools::CollapseFullPath(this->Impl->TestDir);
    }

    if (currDir != workDir) {
      if (!this->TryToChangeDirectory(workDir)) {
        return 1;
      }
    }

    if (!this->Initialize(workDir.c_str(), nullptr)) {
      res = 12;
      cmCTestLog(this, ERROR_MESSAGE,
                 "Problem initializing the dashboard." << std::endl);
    } else {
      res = this->ProcessSteps();
    }
    this->Finalize();

    if (currDir != workDir) {
      cmSystemTools::ChangeDirectory(currDir);
    }
  }
  if (res != 0) {
    cmCTestLog(this, DEBUG,
               "Running a test(s) failed returning : " << res << std::endl);
  }
  return res;
}

int cmCTest::RunCMakeAndTest(std::string* output)
{
  this->Impl->Verbose = true;
  cmCTestBuildAndTestHandler* handler = this->GetBuildAndTestHandler();
  int retv = handler->ProcessHandler();
  *output = handler->GetOutput();
#ifndef CMAKE_BOOTSTRAP
  cmDynamicLoader::FlushCache();
#endif
  if (retv != 0) {
    cmCTestLog(this, DEBUG,
               "build and test failing returning: " << retv << std::endl);
  }
  return retv;
}

void cmCTest::SetNotesFiles(const std::string& notes)
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

int cmCTest::ReadCustomConfigurationFileTree(const std::string& dir,
                                             cmMakefile* mf)
{
  bool found = false;
  cmCTestLog(this, DEBUG,
             "* Read custom CTest configuration directory: " << dir
                                                             << std::endl);

  std::string fname = cmStrCat(dir, "/CTestCustom.cmake");
  cmCTestLog(this, DEBUG, "* Check for file: " << fname << std::endl);
  if (cmSystemTools::FileExists(fname)) {
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
    found = true;
    if (erroroc) {
      cmSystemTools::SetErrorOccurred();
    }
  }

  std::string rexpr = cmStrCat(dir, "/CTestCustom.ctest");
  cmCTestLog(this, DEBUG, "* Check for file: " << rexpr << std::endl);
  if (!found && cmSystemTools::FileExists(rexpr)) {
    cmsys::Glob gl;
    gl.RecurseOn();
    gl.FindFiles(rexpr);
    std::vector<std::string>& files = gl.GetFiles();
    for (const std::string& file : files) {
      cmCTestLog(this, DEBUG,
                 "* Read custom CTest configuration file: " << file
                                                            << std::endl);
      if (!mf->ReadListFile(file) || cmSystemTools::GetErrorOccurredFlag()) {
        cmCTestLog(this, ERROR_MESSAGE,
                   "Problem reading custom configuration: " << file
                                                            << std::endl);
      }
    }
    found = true;
  }

  if (found) {
    for (auto& handler : this->Impl->GetNamedTestingHandlers()) {
      cmCTestLog(this, DEBUG,
                 "* Read custom CTest configuration vectors for handler: "
                   << handler.first << " (" << handler.second << ")"
                   << std::endl);
      handler.second->PopulateCustomVectors(mf);
    }
  }

  return 1;
}

void cmCTest::PopulateCustomVector(cmMakefile* mf, const std::string& def,
                                   std::vector<std::string>& vec)
{
  cmValue dval = mf->GetDefinition(def);
  if (!dval) {
    return;
  }
  cmCTestLog(this, DEBUG, "PopulateCustomVector: " << def << std::endl);

  vec.clear();
  cmExpandList(*dval, vec);

  for (std::string const& it : vec) {
    cmCTestLog(this, DEBUG, "  -- " << it << std::endl);
  }
}

void cmCTest::PopulateCustomInteger(cmMakefile* mf, const std::string& def,
                                    int& val)
{
  cmValue dval = mf->GetDefinition(def);
  if (!dval) {
    return;
  }
  val = atoi(dval->c_str());
}

std::string cmCTest::GetShortPathToFile(const std::string& cfname)
{
  const std::string& sourceDir = cmSystemTools::CollapseFullPath(
    this->GetCTestConfiguration("SourceDirectory"));
  const std::string& buildDir = cmSystemTools::CollapseFullPath(
    this->GetCTestConfiguration("BuildDirectory"));
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

std::string cmCTest::GetCTestConfiguration(const std::string& name)
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

void cmCTest::SetCTestConfiguration(const char* name, const std::string& value,
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

const char* cmCTest::GetSpecificGroup()
{
  if (this->Impl->SpecificGroup.empty()) {
    return nullptr;
  }
  return this->Impl->SpecificGroup.c_str();
}

void cmCTest::SetSpecificGroup(const char* group)
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

void cmCTest::SetStreams(std::ostream* out, std::ostream* err)
{
  this->Impl->StreamOut = out;
  this->Impl->StreamErr = err;
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

const std::map<std::string, std::string>& cmCTest::GetDefinitions() const
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

void cmCTest::SetBuildID(const std::string& id)
{
  this->Impl->BuildID = id;
}

std::string cmCTest::GetBuildID() const
{
  return this->Impl->BuildID;
}

void cmCTest::AddSubmitFile(Part part, const std::string& name)
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

void cmCTest::SetSuppressUpdatingCTestConfiguration(bool val)
{
  this->Impl->SuppressUpdatingCTestConfiguration = val;
}

void cmCTest::AddCTestConfigurationOverwrite(const std::string& overStr)
{
  size_t epos = overStr.find('=');
  if (epos == std::string::npos) {
    cmCTestLog(this, ERROR_MESSAGE,
               "CTest configuration overwrite specified in the wrong format."
                 << std::endl
                 << "Valid format is: --overwrite key=value" << std::endl
                 << "The specified was: --overwrite " << overStr << std::endl);
    return;
  }
  std::string key = overStr.substr(0, epos);
  std::string value = overStr.substr(epos + 1);
  this->Impl->CTestConfigurationOverwrites[key] = value;
}

void cmCTest::SetConfigType(const std::string& ct)
{
  this->Impl->ConfigType = ct;
  cmSystemTools::ReplaceString(this->Impl->ConfigType, ".\\", "");
  std::string confTypeEnv = "CMAKE_CONFIG_TYPE=" + this->Impl->ConfigType;
  cmSystemTools::PutEnv(confTypeEnv);
}

bool cmCTest::SetCTestConfigurationFromCMakeVariable(
  cmMakefile* mf, const char* dconfig, const std::string& cmake_var,
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

bool cmCTest::RunCommand(std::vector<std::string> const& args,
                         std::string* stdOut, std::string* stdErr, int* retVal,
                         const char* dir, cmDuration timeout,
                         Encoding encoding)
{
  std::vector<const char*> argv;
  argv.reserve(args.size() + 1);
  for (std::string const& a : args) {
    argv.push_back(a.c_str());
  }
  argv.push_back(nullptr);

  stdOut->clear();
  stdErr->clear();

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, argv.data());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  if (cmSystemTools::GetRunCommandHideConsole()) {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  }
  cmsysProcess_SetTimeout(cp, timeout.count());
  cmsysProcess_Execute(cp);

  std::vector<char> tempOutput;
  std::vector<char> tempError;
  char* data;
  int length;
  cmProcessOutput processOutput(encoding);
  std::string strdata;
  int res;
  bool done = false;
  while (!done) {
    res = cmsysProcess_WaitForData(cp, &data, &length, nullptr);
    switch (res) {
      case cmsysProcess_Pipe_STDOUT:
        cm::append(tempOutput, data, data + length);
        break;
      case cmsysProcess_Pipe_STDERR:
        cm::append(tempError, data, data + length);
        break;
      default:
        done = true;
    }
    if ((res == cmsysProcess_Pipe_STDOUT || res == cmsysProcess_Pipe_STDERR) &&
        this->Impl->ExtraVerbose) {
      processOutput.DecodeText(data, length, strdata);
      cmSystemTools::Stdout(strdata);
    }
  }
  if (this->Impl->ExtraVerbose) {
    processOutput.DecodeText(std::string(), strdata);
    if (!strdata.empty()) {
      cmSystemTools::Stdout(strdata);
    }
  }

  cmsysProcess_WaitForExit(cp, nullptr);
  if (!tempOutput.empty()) {
    processOutput.DecodeText(tempOutput, tempOutput);
    stdOut->append(tempOutput.data(), tempOutput.size());
  }
  if (!tempError.empty()) {
    processOutput.DecodeText(tempError, tempError);
    stdErr->append(tempError.data(), tempError.size());
  }

  bool result = true;
  if (cmsysProcess_GetState(cp) == cmsysProcess_State_Exited) {
    if (retVal) {
      *retVal = cmsysProcess_GetExitValue(cp);
    } else {
      if (cmsysProcess_GetExitValue(cp) != 0) {
        result = false;
      }
    }
  } else if (cmsysProcess_GetState(cp) == cmsysProcess_State_Exception) {
    const char* exception_str = cmsysProcess_GetExceptionString(cp);
    cmCTestLog(this, ERROR_MESSAGE, exception_str << std::endl);
    stdErr->append(exception_str, strlen(exception_str));
    result = false;
  } else if (cmsysProcess_GetState(cp) == cmsysProcess_State_Error) {
    const char* error_str = cmsysProcess_GetErrorString(cp);
    cmCTestLog(this, ERROR_MESSAGE, error_str << std::endl);
    stdErr->append(error_str, strlen(error_str));
    result = false;
  } else if (cmsysProcess_GetState(cp) == cmsysProcess_State_Expired) {
    const char* error_str = "Process terminated due to timeout\n";
    cmCTestLog(this, ERROR_MESSAGE, error_str << std::endl);
    stdErr->append(error_str, strlen(error_str));
    result = false;
  }

  cmsysProcess_Delete(cp);
  return result;
}

void cmCTest::SetOutputLogFileName(const std::string& name)
{
  if (!name.empty()) {
    this->Impl->OutputLogFile = cm::make_unique<cmGeneratedFileStream>(name);
  } else {
    this->Impl->OutputLogFile.reset();
  }
}

static const char* cmCTestStringLogType[] = { "DEBUG",
                                              "OUTPUT",
                                              "HANDLER_OUTPUT",
                                              "HANDLER_PROGRESS_OUTPUT",
                                              "HANDLER_TEST_PROGRESS_OUTPUT",
                                              "HANDLER_VERBOSE_OUTPUT",
                                              "WARNING",
                                              "ERROR_MESSAGE",
                                              nullptr };

#define cmCTestLogOutputFileLine(stream)                                      \
  do {                                                                        \
    if (this->Impl->ShowLineNumbers) {                                        \
      (stream) << std::endl << file << ":" << line << " ";                    \
    }                                                                         \
  } while (false)

void cmCTest::Log(int logType, const char* file, int line, const char* msg,
                  bool suppress)
{
  if (!msg || !*msg) {
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
      cmCTestLogOutputFileLine(*this->Impl->OutputLogFile);
      if (logType != this->Impl->OutputLogFileLastTag) {
        *this->Impl->OutputLogFile << "[";
        if (logType >= OTHER || logType < 0) {
          *this->Impl->OutputLogFile << "OTHER";
        } else {
          *this->Impl->OutputLogFile << cmCTestStringLogType[logType];
        }
        *this->Impl->OutputLogFile << "] " << std::endl;
      }
      *this->Impl->OutputLogFile << msg << std::flush;
      if (logType != this->Impl->OutputLogFileLastTag) {
        *this->Impl->OutputLogFile << std::endl;
        this->Impl->OutputLogFileLastTag = logType;
      }
    }
  }
  if (!this->Impl->Quiet) {
    std::ostream& out = *this->Impl->StreamOut;
    std::ostream& err = *this->Impl->StreamErr;

    if (logType == HANDLER_TEST_PROGRESS_OUTPUT) {
      if (this->Impl->TestProgressOutput) {
        cmCTestLogOutputFileLine(out);
        if (this->Impl->FlushTestProgressLine) {
          printf("\r");
          this->Impl->FlushTestProgressLine = false;
          out.flush();
        }

        std::string msg_str{ msg };
        auto const lineBreakIt = msg_str.find('\n');
        if (lineBreakIt != std::string::npos) {
          this->Impl->FlushTestProgressLine = true;
          msg_str.erase(std::remove(msg_str.begin(), msg_str.end(), '\n'),
                        msg_str.end());
        }

        out << msg_str;
#ifndef _WIN32
        printf("\x1B[K"); // move caret to end
#endif
        out.flush();
        return;
      }
      logType = HANDLER_OUTPUT;
    }

    switch (logType) {
      case DEBUG:
        if (this->Impl->Debug) {
          cmCTestLogOutputFileLine(out);
          out << msg;
          out.flush();
        }
        break;
      case OUTPUT:
      case HANDLER_OUTPUT:
        if (this->Impl->Debug || this->Impl->Verbose) {
          cmCTestLogOutputFileLine(out);
          out << msg;
          out.flush();
        }
        break;
      case HANDLER_VERBOSE_OUTPUT:
        if (this->Impl->Debug || this->Impl->ExtraVerbose) {
          cmCTestLogOutputFileLine(out);
          out << msg;
          out.flush();
        }
        break;
      case WARNING:
        cmCTestLogOutputFileLine(err);
        err << msg;
        err.flush();
        break;
      case ERROR_MESSAGE:
        cmCTestLogOutputFileLine(err);
        err << msg;
        err.flush();
        cmSystemTools::SetErrorOccurred();
        break;
      default:
        cmCTestLogOutputFileLine(out);
        out << msg;
        out.flush();
    }
  }
}

std::string cmCTest::GetColorCode(Color color) const
{
  if (this->Impl->OutputColorCode) {
#if defined(_WIN32)
    // Not supported on Windows
    static_cast<void>(color);
#else
    return "\033[0;" + std::to_string(static_cast<int>(color)) + "m";
#endif
  }

  return "";
}

cmDuration cmCTest::GetRemainingTimeAllowed()
{
  return this->GetScriptHandler()->GetRemainingTimeAllowed();
}

cmDuration cmCTest::MaxDuration()
{
  return cmDuration(1.0e7);
}

void cmCTest::SetRunCurrentScript(bool value)
{
  this->GetScriptHandler()->SetRunCurrentScript(value);
}

void cmCTest::OutputTestErrors(std::vector<char> const& process_output)
{
  std::string test_outputs("\n*** Test Failed:\n");
  if (!process_output.empty()) {
    test_outputs.append(process_output.data(), process_output.size());
  }
  cmCTestLog(this, HANDLER_OUTPUT, test_outputs << std::endl);
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
