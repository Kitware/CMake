/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <ctime>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmDuration.h"
#include "cmProcessOutput.h"

class cmCTestBuildHandler;
class cmCTestBuildAndTestHandler;
class cmCTestCoverageHandler;
class cmCTestScriptHandler;
class cmCTestTestHandler;
class cmCTestUpdateHandler;
class cmCTestConfigureHandler;
class cmCTestMemCheckHandler;
class cmCTestSubmitHandler;
class cmCTestUploadHandler;
class cmCTestStartCommand;
class cmGeneratedFileStream;
class cmMakefile;
class cmXMLWriter;

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
  Part GetPartFromName(const std::string& name);

  /** Process Command line arguments */
  int Run(std::vector<std::string>&, std::string* output = nullptr);

  /**
   * Initialize and finalize testing
   */
  bool InitializeFromCommand(cmCTestStartCommand* command);
  void Finalize();

  /**
   * Process the dashboard client steps.
   *
   * Steps are enabled using SetTest()
   *
   * The execution of the steps (or #Part) should look like this:
   *
   * /code
   * ctest foo;
   * foo.Initialize();
   * // Set some things on foo
   * foo.ProcessSteps();
   * foo.Finalize();
   * /endcode
   *
   * \sa Initialize(), Finalize(), Part, PartInfo, SetTest()
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
  bool CTestFileExists(const std::string& filename);
  bool AddIfExists(Part part, const std::string& file);

  /**
   * Set the cmake test
   */
  bool SetTest(const std::string&, bool report = true);

  /**
   * Set the cmake test mode (experimental, nightly, continuous).
   */
  void SetTestModel(int mode);
  int GetTestModel() const;

  std::string GetTestModelString();
  static int GetTestModelFromString(const std::string& str);
  static std::string CleanString(const std::string& str,
                                 std::string::size_type spos = 0);
  std::string GetCTestConfiguration(const std::string& name);
  void SetCTestConfiguration(const char* name, const std::string& value,
                             bool suppress = false);
  void EmptyCTestConfiguration();

  std::string GetSubmitURL();

  /**
   * constructor and destructor
   */
  cmCTest();
  ~cmCTest();

  cmCTest(const cmCTest&) = delete;
  cmCTest& operator=(const cmCTest&) = delete;

  /** Set the notes files to be created. */
  void SetNotesFiles(const std::string& notes);

  void PopulateCustomVector(cmMakefile* mf, const std::string& definition,
                            std::vector<std::string>& vec);
  void PopulateCustomInteger(cmMakefile* mf, const std::string& def, int& val);

  /** Get the current time as string */
  std::string CurrentTime();

  /** tar/gzip and then base 64 encode a file */
  std::string Base64GzipEncodeFile(std::string const& file);
  /** base64 encode a file */
  std::string Base64EncodeFile(std::string const& file);

  /**
   * Return the time remaining that the script is allowed to run in
   * seconds if the user has set the variable CTEST_TIME_LIMIT. If that has
   * not been set it returns a very large duration.
   */
  cmDuration GetRemainingTimeAllowed();

  static cmDuration MaxDuration();

  /**
   * Open file in the output directory and set the stream
   */
  bool OpenOutputFile(const std::string& path, const std::string& name,
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
                  const char* dir = nullptr,
                  cmDuration timeout = cmDuration::zero(),
                  Encoding encoding = cmProcessOutput::Auto);

  /**
   * Clean/make safe for xml the given value such that it may be used as
   * one of the key fields by CDash when computing the buildid.
   */
  static std::string SafeBuildIdField(const std::string& value);

  /** Start CTest XML output file */
  void StartXML(cmXMLWriter& xml, bool append);

  /** End CTest XML output file */
  void EndXML(cmXMLWriter& xml);

  /**
   * Run command specialized for make and configure. Returns process status
   * and retVal is return value or exception.
   */
  bool RunMakeCommand(const std::string& command, std::string& output,
                      int* retVal, const char* dir, cmDuration timeout,
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
  std::string GetShortPathToFile(const std::string& fname);

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
   * Run command specialized for tests. Returns process status and retVal is
   * return value or exception. If environment is non-null, it is used to set
   * environment variables prior to running the test. After running the test,
   * environment variables are restored to their previous values.
   */
  bool RunTest(const std::vector<std::string>& args, std::string* output,
               int* retVal, std::ostream* logfile, cmDuration testTimeOut,
               std::vector<std::string>* environment,
               Encoding encoding = cmProcessOutput::Auto);

  /**
   * Get the handler object
   */
  cmCTestBuildHandler* GetBuildHandler();
  cmCTestBuildAndTestHandler* GetBuildAndTestHandler();
  cmCTestCoverageHandler* GetCoverageHandler();
  cmCTestScriptHandler* GetScriptHandler();
  cmCTestTestHandler* GetTestHandler();
  cmCTestUpdateHandler* GetUpdateHandler();
  cmCTestConfigureHandler* GetConfigureHandler();
  cmCTestMemCheckHandler* GetMemCheckHandler();
  cmCTestSubmitHandler* GetSubmitHandler();
  cmCTestUploadHandler* GetUploadHandler();

  /**
   * Set the CTest variable from CMake variable
   */
  bool SetCTestConfigurationFromCMakeVariable(cmMakefile* mf,
                                              const char* dconfig,
                                              const std::string& cmake_var,
                                              bool suppress = false);

  /** Decode a URL to the original string.  */
  static std::string DecodeURL(const std::string&);

  /**
   * Should ctect configuration be updated. When using new style ctest
   * script, this should be true.
   */
  void SetSuppressUpdatingCTestConfiguration(bool val);

  /**
   * Add overwrite to ctest configuration.
   *
   * The format is key=value
   */
  void AddCTestConfigurationOverwrite(const std::string& encstr);

  /** Create XML file that contains all the notes specified */
  int GenerateNotesFile(std::vector<std::string> const& files);

  /** Create XML file to indicate that build is complete */
  int GenerateDoneFile();

  /** Submit extra files to the server */
  bool SubmitExtraFiles(const std::string& files);
  bool SubmitExtraFiles(std::vector<std::string> const& files);

  /** Set the output log file name */
  void SetOutputLogFileName(const std::string& name);

  /** Set the output JUnit file name */
  void SetOutputJUnitFileName(const std::string& name);

  /** Set the visual studio or Xcode config type */
  void SetConfigType(const std::string& ct);

  /** Various log types */
  enum
  {
    DEBUG = 0,
    OUTPUT,
    HANDLER_OUTPUT,
    HANDLER_PROGRESS_OUTPUT,
    HANDLER_TEST_PROGRESS_OUTPUT,
    HANDLER_VERBOSE_OUTPUT,
    WARNING,
    ERROR_MESSAGE,
    OTHER
  };

  /** Add log to the output */
  void Log(int logType, const char* file, int line, const char* msg,
           bool suppress = false);

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
  void SetBuildID(const std::string& id);
  std::string GetBuildID() const;

  /** Add file to be submitted */
  void AddSubmitFile(Part part, const std::string& name);
  std::vector<std::string> const& GetSubmitFiles(Part part) const;
  void ClearSubmitFiles(Part part);

  /**
   * Read the custom configuration files and apply them to the current ctest
   */
  int ReadCustomConfigurationFileTree(const std::string& dir, cmMakefile* mf);

  std::vector<std::string>& GetInitialCommandLineArguments();

  /** Set the group to submit to */
  void SetSpecificGroup(const char* group);
  const char* GetSpecificGroup();

  void SetFailover(bool failover);
  bool GetFailover() const;

  bool GetTestProgressOutput() const;

  bool GetVerbose() const;
  bool GetExtraVerbose() const;

  /** Direct process output to given streams.  */
  void SetStreams(std::ostream* out, std::ostream* err);

  void AddSiteProperties(cmXMLWriter& xml);

  bool GetLabelSummary() const;
  bool GetSubprojectSummary() const;

  std::string GetCostDataFile();

  bool GetOutputTestOutputOnTestFailure() const;

  const std::map<std::string, std::string>& GetDefinitions() const;

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

  void SetRunCurrentScript(bool value);

private:
  void SetPersistentOptionIfNotEmpty(const std::string& value,
                                     const std::string& optionName);
  void AddPersistentMultiOptionIfNotEmpty(const std::string& value,
                                          const std::string& optionName);

  int GenerateNotesFile(const std::string& files);

  void BlockTestErrorDiagnostics();

  /**
   * Initialize a dashboard run in the given build tree.  The "command"
   * argument is non-NULL when running from a command-driven (ctest_start)
   * dashboard script, and NULL when running from the CTest command
   * line.  Note that a declarative dashboard script does not actually
   * call this method because it sets CTEST_COMMAND to drive a build
   * through the ctest command line.
   */
  int Initialize(const std::string& binary_dir, cmCTestStartCommand* command);

  /** parse the option after -D and convert it into the appropriate steps */
  bool AddTestsForDashboardType(std::string& targ);

  /** read as "emit an error message for an unknown -D value" */
  void ErrorMessageUnknownDashDValue(std::string& val);

  /** add a variable definition from a command line -D value */
  bool AddVariableDefinition(const std::string& arg);

  /** set command line arguments read from a test preset */
  bool SetArgsFromPreset(const std::string& presetName, bool listPresets);

  /** parse and process most common command line arguments */
  bool HandleCommandLineArguments(size_t& i, std::vector<std::string>& args,
                                  std::string& errormsg);

#if !defined(_WIN32)
  /** returns true iff the console supports progress output */
  static bool ConsoleIsNotDumb();
#endif

  /** returns true iff the console supports progress output */
  static bool ProgressOutputSupportedByConsole();

  /** returns true iff the console supports colored output */
  static bool ColoredOutputSupportedByConsole();

  /** handle the -S -SP and -SR arguments */
  void HandleScriptArguments(size_t& i, std::vector<std::string>& args,
                             bool& SRArgumentSpecified);

  /** Reread the configuration file */
  bool UpdateCTestConfiguration();

  /** Create note from files. */
  int GenerateCTestNotesOutput(cmXMLWriter& xml,
                               std::vector<std::string> const& files);

  /** Check if the argument is the one specified */
  static bool CheckArgument(const std::string& arg, cm::string_view varg1,
                            const char* varg2 = nullptr);

  /** Output errors from a test */
  void OutputTestErrors(std::vector<char> const& process_output);

  /** Handle the --test-action command line argument */
  bool HandleTestActionArgument(const char* ctestExec, size_t& i,
                                const std::vector<std::string>& args);

  /** Handle the --test-model command line argument */
  bool HandleTestModelArgument(const char* ctestExec, size_t& i,
                               const std::vector<std::string>& args);

  int RunCMakeAndTest(std::string* output);
  int ExecuteTests();

  /** return true iff change directory was successful */
  bool TryToChangeDirectory(std::string const& dir);

  struct Private;
  std::unique_ptr<Private> Impl;
};

class cmCTestLogWrite
{
public:
  cmCTestLogWrite(const char* data, size_t length)
    : Data(data)
    , Length(length)
  {
  }

  const char* Data;
  size_t Length;
};

inline std::ostream& operator<<(std::ostream& os, const cmCTestLogWrite& c)
{
  if (!c.Length) {
    return os;
  }
  os.write(c.Data, c.Length);
  os.flush();
  return os;
}

#define cmCTestLog(ctSelf, logType, msg)                                      \
  do {                                                                        \
    std::ostringstream cmCTestLog_msg;                                        \
    cmCTestLog_msg << msg;                                                    \
    (ctSelf)->Log(cmCTest::logType, __FILE__, __LINE__,                       \
                  cmCTestLog_msg.str().c_str());                              \
  } while (false)

#define cmCTestOptionalLog(ctSelf, logType, msg, suppress)                    \
  do {                                                                        \
    std::ostringstream cmCTestLog_msg;                                        \
    cmCTestLog_msg << msg;                                                    \
    (ctSelf)->Log(cmCTest::logType, __FILE__, __LINE__,                       \
                  cmCTestLog_msg.str().c_str(), suppress);                    \
  } while (false)
