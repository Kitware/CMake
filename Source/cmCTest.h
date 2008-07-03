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

#ifndef cmCTest_h
#define cmCTest_h

#include "cmStandardIncludes.h"
#include "cmListFileCache.h"
#include <time.h>

class cmake;
class cmMakefile;
class cmCTestGenericHandler;
class cmGeneratedFileStream;
class cmCTestCommand;

#define cmCTestLog(ctSelf, logType, msg) \
  do { \
  cmOStringStream cmCTestLog_msg; \
  cmCTestLog_msg << msg; \
  (ctSelf)->Log(cmCTest::logType, __FILE__, __LINE__,\
                cmCTestLog_msg.str().c_str());\
  } while ( 0 )

#ifdef cerr
#  undef cerr
#endif
#define cerr no_cerr_use_cmCTestLog

#ifdef cout
#  undef cout
#endif
#define cout no_cout_use_cmCTestLog

class cmCTest
{
public:
  typedef std::vector<cmStdString> VectorOfStrings;
  typedef std::set<cmStdString> SetOfStrings;

  ///! Process Command line arguments
  int Run(std::vector<std::string> &, std::string* output = 0);

  /**
   * Initialize and finalize testing
   */
  int Initialize(const char* binary_dir, bool new_tag = false,
    bool verbose_tag = true);
  bool InitializeFromCommand(cmCTestCommand* command, bool first = false);
  void Finalize();

  /**
   * Process the tests. This is the main routine. The execution of the
   * tests should look like this:
   *
   * ctest foo;
   * foo.Initialize();
   * // Set some things on foo
   * foo.ProcessTests();
   * foo.Finalize();
   */
  int ProcessTests();

  /*
   * A utility function that returns the nightly time
   */
  struct tm* GetNightlyTime(std::string str,
    bool tomorrowtag);

  /*
   * Is the tomorrow tag set?
   */
  bool GetTomorrowTag() { return this->TomorrowTag; };

  /**
   * Try to run tests of the project
   */
  int TestDirectory(bool memcheck);

  ///! what is the configuraiton type, e.g. Debug, Release etc.
  std::string const& GetConfigType();
  double GetTimeOut() { return this->TimeOut; }
  void SetTimeOut(double t) { this->TimeOut = t; }
  // how many test to run at the same time
  int GetParallelLevel() { return this->ParallelLevel; }
  void SetParallelLevel(int t) { this->ParallelLevel = t; }

  bool GetParallelSubprocess() { return this->ParallelSubprocess; }
  void SetParallelSubprocess() { this->ParallelSubprocess = true; }

  void SetParallelSubprocessId(int id) { this->ParallelSubprocessId = id;}
  int GetParallelSubprocessId() { return this->ParallelSubprocessId;}
  const char* GetParallelCacheFile()
    { return this->ParallelCacheFile.c_str();}
  void SetParallelCacheFile(const char* c) { this->ParallelCacheFile = c; }

  /**
   * Check if CTest file exists
   */
  bool CTestFileExists(const std::string& filename);
  bool AddIfExists(SetOfStrings& files, const char* file);

  /**
   * Set the cmake test
   */
  bool SetTest(const char*, bool report = true);

  /**
   * Set the cmake test mode (experimental, nightly, continuous).
   */
  void SetTestModel(int mode);
  int GetTestModel() { return this->TestModel; };

  std::string GetTestModelString();
  static int GetTestModelFromString(const char* str);
  static std::string CleanString(const std::string& str);
  std::string GetCTestConfiguration(const char *name);
  void SetCTestConfiguration(const char *name, const char* value);
  void EmptyCTestConfiguration();

  /**
   * constructor and destructor
   */
  cmCTest();
  ~cmCTest();

  //! Set the notes files to be created.
  void SetNotesFiles(const char* notes);

  void PopulateCustomVector(cmMakefile* mf, const char* definition,
    VectorOfStrings& vec);
  void PopulateCustomInteger(cmMakefile* mf, const char* def,
    int& val);

  ///! Get the current time as string
  std::string CurrentTime();

  /** 
   * Return the time remaianing that the script is allowed to run in
   * seconds if the user has set the variable CTEST_TIME_LIMIT. If that has
   * not been set it returns 1e7 seconds
   */
  double GetRemainingTimeAllowed();
    
  ///! Open file in the output directory and set the stream
  bool OpenOutputFile(const std::string& path,
                      const std::string& name,
                      cmGeneratedFileStream& stream,
                      bool compress = false);

  ///! Convert string to something that is XML safe
  static std::string MakeXMLSafe(const std::string&);

  ///! Should we only show what we would do?
  bool GetShowOnly();

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
   *
   * If the command has spaces in the path the caller MUST call
   * cmSystemTools::ConvertToRunCommandPath on the command before passing
   * it into this function or it will not work.  The command must be correctly
   * escaped for this to with spaces.
   */
  bool RunCommand(const char* command,
    std::string* stdOut, std::string* stdErr,
    int* retVal = 0, const char* dir = 0, double timeout = 0.0);

  //! Start CTest XML output file
  void StartXML(std::ostream& ostr);

  //! End CTest XML output file
  void EndXML(std::ostream& ostr);

  //! Run command specialized for make and configure. Returns process status
  // and retVal is return value or exception.
  int RunMakeCommand(const char* command, std::string* output,
    int* retVal, const char* dir, int timeout,
    std::ofstream& ofs);

  /*
   * return the current tag
   */
  std::string GetCurrentTag();

  //! Get the path to the build tree
  std::string GetBinaryDir();

  //! Get the short path to the file. This means if the file is in binary or
  //source directory, it will become /.../relative/path/to/file
  std::string GetShortPathToFile(const char* fname);

  //! Get the path to CTest
  const char* GetCTestExecutable() { return this->CTestSelf.c_str(); }
  const char* GetCMakeExecutable() { return this->CMakeSelf.c_str(); }

  enum {
    EXPERIMENTAL,
    NIGHTLY,
    CONTINUOUS
  };

  // provide some more detailed info on the return code for ctest
  enum {
    UPDATE_ERRORS    = 0x01,
    CONFIGURE_ERRORS = 0x02,
    BUILD_ERRORS     = 0x04,
    TEST_ERRORS      = 0x08,
    MEMORY_ERRORS    = 0x10,
    COVERAGE_ERRORS  = 0x20,
    SUBMIT_ERRORS    = 0x40
  };

  ///! Are we producing XML
  bool GetProduceXML();
  void SetProduceXML(bool v);

  //! Run command specialized for tests. Returns process status and retVal is
  // return value or exception.
  int RunTest(std::vector<const char*> args, std::string* output, int *retVal,
    std::ostream* logfile, double testTimeOut);

  /**
   * Execute handler and return its result. If the handler fails, it returns
   * negative value.
   */
  int ExecuteHandler(const char* handler);

  /*
   * Get the handler object
   */
  cmCTestGenericHandler* GetHandler(const char* handler);
  cmCTestGenericHandler* GetInitializedHandler(const char* handler);

  /*
   * Set the CTest variable from CMake variable
   */
  bool SetCTestConfigurationFromCMakeVariable(cmMakefile* mf,
    const char* dconfig, const char* cmake_var);

  //! Make string safe to be send as an URL
  static std::string MakeURLSafe(const std::string&);

  //! Should ctect configuration be updated. When using new style ctest
  // script, this should be true.
  void SetSuppressUpdatingCTestConfiguration(bool val)
    {
    this->SuppressUpdatingCTestConfiguration = val;
    }

  //! Add overwrite to ctest configuration.
  // The format is key=value
  void AddCTestConfigurationOverwrite(const char* encstr);

  //! Create XML file that contains all the notes specified
  int GenerateNotesFile(const std::vector<cmStdString> &files);

  //! Submit extra files to the server
  bool SubmitExtraFiles(const char* files);
  bool SubmitExtraFiles(const std::vector<cmStdString> &files);

  //! Set the output log file name
  void SetOutputLogFileName(const char* name);

  //! Set the visual studio or Xcode config type
  void SetConfigType(const char* ct);

  //! Various log types
  enum {
    DEBUG = 0,
    OUTPUT,
    HANDLER_OUTPUT,
    HANDLER_VERBOSE_OUTPUT,
    WARNING,
    ERROR_MESSAGE,
    OTHER
  };

  //! Add log to the output
  void Log(int logType, const char* file, int line, const char* msg);

  //! Get the version of dart server
  int GetDartVersion() { return this->DartVersion; }

  //! Add file to be submitted
  void AddSubmitFile(const char* name);
  SetOfStrings* GetSubmitFiles() { return &this->SubmitFiles; }

  //! Read the custom configuration files and apply them to the current ctest
  int ReadCustomConfigurationFileTree(const char* dir, cmMakefile* mf);

  std::vector<cmStdString> &GetInitialCommandLineArguments()
  { return this->InitialCommandLineArguments; };

  //! Set the track to submit to
  void SetSpecificTrack(const char* track);
  const char* GetSpecificTrack();

  bool GetVerbose() { return this->Verbose;}
  bool GetExtraVerbose() { return this->ExtraVerbose;}
private:
  std::string ConfigType;
  bool Verbose;
  bool ExtraVerbose;
  bool ProduceXML;

  bool ForceNewCTestProcess;

  bool RunConfigurationScript;

  int GenerateNotesFile(const char* files);

  // these are helper classes
  typedef std::map<cmStdString,cmCTestGenericHandler*> t_TestingHandlers;
  t_TestingHandlers TestingHandlers;

  bool ShowOnly;

  enum {
    FIRST_TEST     = 0,
    UPDATE_TEST    = 1,
    START_TEST     = 2,
    CONFIGURE_TEST = 3,
    BUILD_TEST     = 4,
    TEST_TEST      = 5,
    COVERAGE_TEST  = 6,
    MEMCHECK_TEST  = 7,
    SUBMIT_TEST    = 8,
    NOTES_TEST     = 9,
    ALL_TEST       = 10,
    LAST_TEST      = 11
  };

  //! Map of configuration properties
  typedef std::map<cmStdString, cmStdString> CTestConfigurationMap;

  std::string             CTestConfigFile;
  CTestConfigurationMap CTestConfiguration;
  CTestConfigurationMap CTestConfigurationOverwrites;
  int                     Tests[LAST_TEST];

  std::string             CurrentTag;
  bool                    TomorrowTag;

  int                     TestModel;
  std::string             SpecificTrack;

  double                  TimeOut;

  std::string             ParallelCacheFile;
  int                     ParallelLevel;
  int                     ParallelSubprocessId;
  bool                    ParallelSubprocess;
  int                     CompatibilityMode;

  // information for the --build-and-test options
  std::string              CMakeSelf;
  std::string              CTestSelf;
  std::string              BinaryDir;

  std::string              NotesFiles;


  bool                     InteractiveDebugMode;

  bool                     ShortDateFormat;

  bool                     CompressXMLFiles;

  void BlockTestErrorDiagnostics();


  //! parse the option after -D and convert it into the appropriate steps
  bool AddTestsForDashboardType(std::string &targ);

  //! parse and process most common command line arguments
  void HandleCommandLineArguments(size_t &i, 
                                  std::vector<std::string> &args);

  //! hande the -S -SP and -SR arguments
  void HandleScriptArguments(size_t &i, 
                             std::vector<std::string> &args,
                             bool &SRArgumentSpecified);

  //! Reread the configuration file
  bool UpdateCTestConfiguration();

  //! Create not from files.
  int GenerateCTestNotesOutput(std::ostream& os,
    const VectorOfStrings& files);

  ///! Find the running cmake
  void FindRunningCMake();

  //! Check if the argument is the one specified
  bool CheckArgument(const std::string& arg, const char* varg1,
    const char* varg2 = 0);

  bool                      SuppressUpdatingCTestConfiguration;

  bool Debug;
  bool ShowLineNumbers;
  bool Quiet;

  int  DartVersion;

  std::set<cmStdString> SubmitFiles;
  std::vector<cmStdString> InitialCommandLineArguments;

  int SubmitIndex;

  cmGeneratedFileStream* OutputLogFile;
  int OutputLogFileLastTag;
};

class cmCTestLogWrite
{
public:
  cmCTestLogWrite(const char* data, size_t length)
    : Data(data), Length(length) {}

  const char* Data;
  size_t Length;
};

inline std::ostream& operator<< (std::ostream& os, const cmCTestLogWrite& c)
{
  if (!c.Length)
    {
    return os;
    }
  os.write(c.Data, c.Length);
  os.flush();
  return os;
}

#endif
