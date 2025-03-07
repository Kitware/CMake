/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <ctime>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmDuration.h"
#include "cmProcessOutput.h"

class cmake;
class cmGeneratedFileStream;
class cmInstrumentation;
class cmMakefile;
class cmValue;
class cmXMLWriter;
struct cmCTestTestOptions;

/** \class cmCTest
 * \brief Represents a ctest invocation.
 *
 * This class represents a ctest invocation. It is the top level class when
 * running ctest.
 *
 */
class cmCTest
{
public:
  using Encoding = cmProcessOutput::Encoding;
  /** Enumerate parts of the testing and submission process.  */
  enum Part
  {
    PartStart,
    PartUpdate,
    PartConfigure,
    PartBuild,
    PartTest,
    PartCoverage,
    PartMemCheck,
    PartSubmit,
    PartNotes,
    PartExtraFiles,
    PartUpload,
    PartDone,
    PartCount // Update names in constructor when adding a part
  };

  /** Get a testing part id from its string name.  Returns PartCount
      if the string does not name a valid part.  */
  Part GetPartFromName(std::string const& name);

  /** Process Command line arguments */
  int Run(std::vector<std::string> const& args);

  /** Initialize a dashboard run in the given build tree. */
  void Initialize(std::string const& binary_dir);

  bool CreateNewTag(bool quiet);
  bool ReadExistingTag(bool quiet);

  /**
   * Process the dashboard client steps.
   */
  int ProcessSteps();

  /**
   * A utility function that returns the nightly time
   */
  struct tm* GetNightlyTime(std::string const& str, bool tomorrowtag);

  /**
   * Is the tomorrow tag set?
   */
  bool GetTomorrowTag() const;

  /**
   * Try to run tests of the project
   */
  int TestDirectory(bool memcheck);

  /** what is the configuration type, e.g. Debug, Release etc. */
  std::string const& GetConfigType();
  cmDuration GetTimeOut() const;
  void SetTimeOut(cmDuration t);

  cmDuration GetGlobalTimeout() const;

  /** how many test to run at the same time */
  cm::optional<size_t> GetParallelLevel() const;
  void SetParallelLevel(cm::optional<size_t> level);

  unsigned long GetTestLoad() const;
  void SetTestLoad(unsigned long);

  /**
   * Check if CTest file exists
   */
  bool CTestFileExists(std::string const& filename);
  bool AddIfExists(Part part, std::string const& file);

  /**
   * Set the cmake test
   */
  bool SetTest(std::string const&, bool report = true);

  /**
   * Set the cmake test mode (experimental, nightly, continuous).
   */
  void SetTestModel(int mode);
  int GetTestModel() const;

  std::string GetTestModelString() const;
  std::string GetTestGroupString() const;
  static int GetTestModelFromString(std::string const& str);
  static std::string CleanString(std::string const& str,
                                 std::string::size_type spos = 0);
  std::string GetCTestConfiguration(std::string const& name);
  void SetCTestConfiguration(char const* name, std::string const& value,
                             bool suppress = false);
  void EmptyCTestConfiguration();

  std::string GetSubmitURL();

  /**
   * constructor and destructor
   */
  cmCTest();
  ~cmCTest();

  cmCTest(cmCTest const&) = delete;
  cmCTest& operator=(cmCTest const&) = delete;

  /** Set the notes files to be created. */
  void SetNotesFiles(std::string const& notes);

  void PopulateCustomVector(cmMakefile* mf, std::string const& definition,
                            std::vector<std::string>& vec);
  void PopulateCustomInteger(cmMakefile* mf, std::string const& def, int& val);

  /** Get the current time as string */
  std::string CurrentTime();

  /** tar/gzip and then base 64 encode a file */
  std::string Base64GzipEncodeFile(std::string const& file);
  /** base64 encode a file */
  std::string Base64EncodeFile(std::string const& file);

  void SetTimeLimit(cmValue val);
  cmDuration GetElapsedTime() const;

  /**
   * Return the time remaining that the script is allowed to run in
   * seconds if the user has set the variable CTEST_TIME_LIMIT. If that has
   * not been set it returns a very large duration.
   */
  cmDuration GetRemainingTimeAllowed() const;

  static cmDuration MaxDuration();

  /**
   * Open file in the output directory and set the stream
   */
  bool OpenOutputFile(std::string const& path, std::string const& name,
                      cmGeneratedFileStream& stream, bool compress = false);

  /** Should we only show what we would do? */
  bool GetShowOnly();

  bool GetOutputAsJson();

  int GetOutputAsJsonVersion();

  bool ShouldUseHTTP10() const;

  bool ShouldPrintLabels() const;

  bool ShouldCompressTestOutput();
  bool CompressString(std::string& str);

  bool GetStopOnFailure() const;
  void SetStopOnFailure(bool stop);

  std::chrono::system_clock::time_point GetStopTime() const;
  void SetStopTime(std::string const& time);

  /** Used for parallel ctest job scheduling */
  std::string GetScheduleType() const;
  void SetScheduleType(std::string const& type);

  /** The max output width */
  int GetMaxTestNameWidth() const;
  void SetMaxTestNameWidth(int w);

  /**
   * Run a single executable command and put the stdout and stderr
   * in output.
   *
   * If verbose is false, no user-viewable output from the program
   * being run will be generated.
   *
   * If timeout is specified, the command will be terminated after
   * timeout expires. Timeout is specified in seconds.
   *
   * Argument retVal should be a pointer to the location where the
   * exit code will be stored. If the retVal is not specified and
   * the program exits with a code other than 0, then the this
   * function will return false.
   */
  bool RunCommand(std::vector<std::string> const& args, std::string* stdOut,
                  std::string* stdErr, int* retVal = nullptr,
                  char const* dir = nullptr,
                  cmDuration timeout = cmDuration::zero(),
                  Encoding encoding = cmProcessOutput::Auto);

  /**
   * Clean/make safe for xml the given value such that it may be used as
   * one of the key fields by CDash when computing the buildid.
   */
  static std::string SafeBuildIdField(std::string const& value);

  /** Start CTest XML output file */
  void StartXML(cmXMLWriter& xml, cmake* cm, bool append);

  /** End CTest XML output file */
  void EndXML(cmXMLWriter& xml);

  /**
   * Run command specialized for make and configure. Returns process status
   * and retVal is return value or exception.
   */
  bool RunMakeCommand(std::string const& command, std::string& output,
                      int* retVal, char const* dir, cmDuration timeout,
                      std::ostream& ofs,
                      Encoding encoding = cmProcessOutput::Auto);

  /** Return the current tag */
  std::string GetCurrentTag();

  /** Get the path to the build tree */
  std::string GetBinaryDir();

  /**
   * Get the short path to the file.
   *
   * This means if the file is in binary or
   * source directory, it will become /.../relative/path/to/file
   */
  std::string GetShortPathToFile(std::string const& fname);

  enum
  {
    UNKNOWN = -1,
    EXPERIMENTAL = 0,
    NIGHTLY = 1,
    CONTINUOUS = 2,
  };

  /** provide some more detailed info on the return code for ctest */
  enum
  {
    UPDATE_ERRORS = 0x01,
    CONFIGURE_ERRORS = 0x02,
    BUILD_ERRORS = 0x04,
    TEST_ERRORS = 0x08,
    MEMORY_ERRORS = 0x10,
    COVERAGE_ERRORS = 0x20,
    SUBMIT_ERRORS = 0x40
  };

  /** Are we producing XML */
  bool GetProduceXML();
  void SetProduceXML(bool v);

  /**
   * Set the CTest variable from CMake variable
   */
  bool SetCTestConfigurationFromCMakeVariable(cmMakefile* mf,
                                              char const* dconfig,
                                              std::string const& cmake_var,
                                              bool suppress = false);

  /**
   * Set CMake variables from CTest Options
   */
  void SetCMakeVariables(cmMakefile& mf);

  /** Decode a URL to the original string.  */
  static std::string DecodeURL(std::string const&);

  /**
   * Add overwrite to ctest configuration.
   *
   * The format is key=value
   */
  void AddCTestConfigurationOverwrite(std::string const& encstr);

  /** Create XML file that contains all the notes specified */
  int GenerateNotesFile(cmake* cm, std::vector<std::string> const& files);

  /** Create XML file to indicate that build is complete */
  int GenerateDoneFile();

  /** Submit extra files to the server */
  bool SubmitExtraFiles(std::string const& files);
  bool SubmitExtraFiles(std::vector<std::string> const& files);

  /** Set the output log file name */
  void SetOutputLogFileName(std::string const& name);

  /** Set the output JUnit file name */
  void SetOutputJUnitFileName(std::string const& name);

  /** Set the visual studio or Xcode config type */
  void SetConfigType(std::string const& ct);

  /** Various log types */
  enum LogType
  {
    DEBUG,
    OUTPUT,
    HANDLER_OUTPUT,
    HANDLER_PROGRESS_OUTPUT,
    HANDLER_TEST_PROGRESS_OUTPUT,
    HANDLER_VERBOSE_OUTPUT,
    WARNING,
    ERROR_MESSAGE,
  };

  /** Add log to the output */
  void Log(LogType logType, std::string msg, bool suppress = false);

  /** Color values */
  enum class Color
  {
    CLEAR_COLOR = 0,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34
  };

  /** Get color code characters for a specific color */
  std::string GetColorCode(Color color) const;

  /** The Build ID is assigned by CDash */
  void SetBuildID(std::string const& id);
  std::string GetBuildID() const;

  /** Add file to be submitted */
  void AddSubmitFile(Part part, std::string const& name);
  std::vector<std::string> const& GetSubmitFiles(Part part) const;
  void ClearSubmitFiles(Part part);

  /**
   * Read the custom configuration files and apply them to the current ctest
   */
  void ReadCustomConfigurationFileTree(std::string const& dir, cmMakefile* mf);

  std::vector<std::string>& GetInitialCommandLineArguments();

  /** Set the group to submit to */
  void SetSpecificGroup(char const* group);
  char const* GetSpecificGroup();

  void SetFailover(bool failover);
  bool GetFailover() const;

  bool GetTestProgressOutput() const;

  bool GetVerbose() const;
  bool GetExtraVerbose() const;

  bool StartResultingXML(Part part, char const* name, int submitIndex,
                         cmGeneratedFileStream& xofs);
  bool StartLogFile(char const* name, int submitIndex,
                    cmGeneratedFileStream& xofs);

  void ConvertInstrumentationSnippetsToXML(cmXMLWriter& xml,
                                           std::string const& subdir);
  bool ConvertInstrumentationJSONFileToXML(std::string const& fpath,
                                           cmXMLWriter& xml);

  void AddSiteProperties(cmXMLWriter& xml, cmake* cm);

  bool GetInteractiveDebugMode() const;

  bool GetLabelSummary() const;
  bool GetSubprojectSummary() const;

  std::string GetCostDataFile();

  bool GetOutputTestOutputOnTestFailure() const;

  std::map<std::string, std::string> const& GetDefinitions() const;

  /** Return the number of times a test should be run */
  int GetRepeatCount() const;

  enum class Repeat
  {
    Never,
    UntilFail,
    UntilPass,
    AfterTimeout,
  };
  Repeat GetRepeatMode() const;

  enum class NoTests
  {
    Legacy,
    Error,
    Ignore
  };
  NoTests GetNoTestsMode() const;

  void GenerateSubprojectsOutput(cmXMLWriter& xml);
  std::vector<std::string> GetLabelsForSubprojects();

  /** Reread the configuration file */
  bool UpdateCTestConfiguration();

  cmCTestTestOptions const& GetTestOptions() const;
  std::vector<std::string> GetCommandLineHttpHeaders() const;

  cmInstrumentation& GetInstrumentation();
  bool GetUseVerboseInstrumentation() const;

private:
  int GenerateNotesFile(cmake* cm, std::string const& files);

  void BlockTestErrorDiagnostics();

  /** parse the option after -D and convert it into the appropriate steps */
  bool AddTestsForDashboardType(std::string const& targ);

  /** read as "emit an error message for an unknown -D value" */
  void ErrorMessageUnknownDashDValue(std::string const& val);

  /** add a variable definition from a command line -D value */
  bool AddVariableDefinition(std::string const& arg);

  /** set command line arguments read from a test preset */
  bool SetArgsFromPreset(std::string const& presetName, bool listPresets);

#if !defined(_WIN32)
  /** returns true iff the console supports progress output */
  static bool ConsoleIsNotDumb();
#endif

  /** returns true iff the console supports progress output */
  static bool ProgressOutputSupportedByConsole();

  /** returns true iff the console supports colored output */
  static bool ColoredOutputSupportedByConsole();

  /** Create note from files. */
  int GenerateCTestNotesOutput(cmXMLWriter& xml, cmake* cm,
                               std::vector<std::string> const& files);

  /** Check if the argument is the one specified */
  static bool CheckArgument(std::string const& arg, cm::string_view varg1,
                            char const* varg2 = nullptr);

  int RunCMakeAndTest();
  int RunScripts(std::vector<std::pair<std::string, bool>> const& scripts);
  int ExecuteTests(std::vector<std::string> const& args);

  struct Private;
  std::unique_ptr<Private> Impl;
};

#define cmCTestLog(ctSelf, logType, msg)                                      \
  do {                                                                        \
    std::ostringstream cmCTestLog_msg;                                        \
    cmCTestLog_msg << msg;                                                    \
    (ctSelf)->Log(cmCTest::logType, cmCTestLog_msg.str());                    \
  } while (false)

#define cmCTestOptionalLog(ctSelf, logType, msg, suppress)                    \
  do {                                                                        \
    std::ostringstream cmCTestLog_msg;                                        \
    cmCTestLog_msg << msg;                                                    \
    (ctSelf)->Log(cmCTest::logType, cmCTestLog_msg.str(), suppress);          \
  } while (false)
