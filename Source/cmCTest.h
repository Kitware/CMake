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

class cmMakefile;

class cmCTest
{
public:
  typedef std::vector<cmStdString> tm_VectorOfStrings;
  typedef std::vector<cmListFileArgument> tm_VectorOfListFileArgs;

  ///! Process Command line arguments
  int Run(std::vector<std::string>const&, std::string* output = 0);
  
  /**
   * Run a dashboard using a specified confiuration script
   */
  int RunConfigurationScript();
  int RunConfigurationScript(const std::string& script);
  int RunConfigurationDashboard(cmMakefile *mf, 
                                const char *srcDir, const char *binDir,
                                const char *ctestRoot,
                                bool backup, const char *cvsCheckOut,
                                const char *ctestCmd);
  
  /**
   * Initialize and finalize testing
   */
  int Initialize();
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

  /**
   * Try to build the project
   */
  int BuildDirectory();

  /**
   * Try to run tests of the project
   */
  int TestDirectory(bool memcheck);

  /**
   * Try to get coverage of the project
   */
  int CoverageDirectory();

  /**
   * Do revision control update of directory
   */
  int UpdateDirectory();

  /**
   * Do configure the project
   */
  int ConfigureDirectory();

  /**
   * Do submit testing results
   */
  int SubmitResults();
  std::string GetSubmitResultsPrefix();

  /**
   * Check if CTest file exists
   */
  bool CTestFileExists(const std::string& filename);

  /**
   * Run the test for a directory and any subdirectories
   */
  void ProcessDirectory(tm_VectorOfStrings &passed, 
                        tm_VectorOfStrings &failed,
                        bool memcheck);

  /**
   * Find the executable for a test
   */
  std::string FindTheExecutable(const char *exe);

  /**
   * Set the cmake test
   */
  bool SetTest(const char*, bool report = true);

  /**
   * Set the cmake test mode (experimental, nightly, continuous).
   */
  void SetTestModel(int mode);

  std::string GetTestModelString();
  static int GetTestModelFromString(const char* str);

  /**
   * constructor
   */
  cmCTest();

  //! Set the notes files to be created.
  void SetNotesFiles(const char* notes);

  bool m_UseIncludeRegExp;
  std::string m_IncludeRegExp;

  bool m_UseExcludeRegExp;
  bool m_UseExcludeRegExpFirst;
  std::string m_ExcludeRegExp;

  std::string m_ConfigType;
  bool m_Verbose;
  bool m_DartMode;
  bool m_ShowOnly;

  bool m_ForceNewCTestProcess;

  bool m_RunConfigurationScript;
  tm_VectorOfStrings m_ConfigurationScripts;

  enum {
    EXPERIMENTAL,
    NIGHTLY,
    CONTINUOUS
  };

  int GenerateNotesFile(const char* files);

  void RestoreBackupDirectories(bool backup, 
                                const char *srcDir, const char *binDir,
                                const char *backupSrc, const char *backupBin);

private:
  void SetTestsToRunInformation(const char*);
  void ExpandTestsToRunInformation(int numPossibleTests);
  std::string TestsToRunString;
  
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
  
  enum { // Program statuses
    NOT_RUN = 0,
    TIMEOUT,
    SEGFAULT,
    ILLEGAL,
    INTERRUPT,
    NUMERICAL,
    OTHER_FAULT,
    FAILED,
    BAD_COMMAND,
    COMPLETED
  };

  enum { // Memory checkers
    UNKNOWN = 0,
    VALGRIND,
    PURIFY,
    BOUNDS_CHECKER
  };

  enum { // Memory faults
    ABR = 0,
    ABW,
    ABWL,
    COR,
    EXU,
    FFM,
    FIM,
    FMM,
    FMR,
    FMW,
    FUM,
    IPR,
    IPW,
    MAF,
    MLK,
    MPK,
    NPR,
    ODS,
    PAR,
    PLK,
    UMC,
    UMR,
    NO_MEMORY_FAULT
  };


  struct cmCTestTestResult
  {
    std::string m_Name;
    std::string m_Path;
    std::string m_FullCommandLine;
    double      m_ExecutionTime;
    int         m_ReturnValue;
    int         m_Status;
    std::string m_CompletionStatus;
    std::string m_Output;
    std::string m_RegressionImages;
    int         m_TestCount;
  };

  struct cmCTestBuildErrorWarning
  {
    bool        m_Error;
    int         m_LogLine;
    std::string m_Text;
    std::string m_SourceFile;
    std::string m_SourceFileTail;
    int         m_LineNumber;
    std::string m_PreContext;
    std::string m_PostContext;
  };

  struct cmCTestTestProperties
    {
    cmStdString m_Name;
    cmStdString m_Directory;
    tm_VectorOfListFileArgs m_Args;
    };

  typedef std::vector<cmCTestTestProperties> tm_ListOfTests;

  // Some structures needed for cvs update
  struct StringPair : 
    public std::pair<std::string, std::string>{};
  struct UpdateFiles : public std::vector<StringPair>{};
  struct AuthorsToUpdatesMap : 
    public std::map<std::string, UpdateFiles>{};

  struct cmCTestCoverage
  {
    cmCTestCoverage()
      {
        m_AbsolutePath = "";
        m_FullPath = "";
        m_Covered = false;
        m_Tested = 0;
        m_UnTested = 0;
        m_Lines.clear();
        m_Show = false;
      }
    std::string      m_AbsolutePath;
    std::string      m_FullPath;
    bool             m_Covered;
    int              m_Tested;
    int              m_UnTested;
    std::vector<int> m_Lines;
    bool             m_Show;
  };

  typedef std::vector<cmCTestTestResult> tm_TestResultsVector;
  //! Map of configuration properties
  typedef std::map<std::string, std::string> tm_DartConfigurationMap;
  typedef std::map<std::string, cmCTestCoverage> tm_CoverageMap;

  tm_TestResultsVector    m_TestResults;
  std::string             m_ToplevelPath;
  tm_DartConfigurationMap m_DartConfiguration;
  int                     m_Tests[LAST_TEST];
  
  std::string             m_CurrentTag;
  bool                    m_TomorrowTag;

  std::string             m_StartBuild;
  std::string             m_EndBuild;
  std::string             m_StartTest;
  std::string             m_EndTest;
  double                  m_ElapsedTestingTime;

  int                     m_TestModel;

  double                  m_TimeOut;

  std::string             m_MemoryTester;
  std::string             m_MemoryTesterOptions;
  int                     m_MemoryTesterStyle;
  std::string             m_MemoryTesterOutputFile;
  tm_VectorOfStrings      m_MemoryTesterOptionsParsed;
  int                     m_MemoryTesterGlobalResults[NO_MEMORY_FAULT];

  int                     m_CompatibilityMode;

  // information for the --build-and-test options
  std::string              m_ExecutableDirectory;
  std::string              m_CMakeSelf;
  std::string              m_CTestSelf;
  std::string              m_SourceDir;
  std::string              m_BinaryDir;
  std::string              m_BuildRunDir;
  std::string              m_BuildGenerator;
  std::string              m_BuildMakeProgram;
  std::string              m_BuildProject;
  std::string              m_BuildTarget;
  std::vector<std::string> m_BuildOptions;
  std::string              m_TestCommand;
  std::vector<std::string> m_TestCommandArgs;
  bool                     m_BuildTwoConfig;
  bool                     m_BuildNoClean;
  bool                     m_BuildNoCMake;
  std::string              m_NotesFiles;
  std::vector<int>         m_TestsToRun;
  

  int ReadCustomConfigurationFileTree(const char* dir);
  void PopulateCustomVector(cmMakefile* mf, const char* definition, tm_VectorOfStrings& vec);

  tm_VectorOfStrings       m_CustomErrorMatches;
  tm_VectorOfStrings       m_CustomErrorExceptions;
  tm_VectorOfStrings       m_CustomWarningMatches;
  tm_VectorOfStrings       m_CustomWarningExceptions;

  tm_VectorOfStrings       m_CustomTestsIgnore;
  tm_VectorOfStrings       m_CustomMemCheckIgnore;

  tm_VectorOfStrings       m_CustomPreTest;
  tm_VectorOfStrings       m_CustomPostTest;
  tm_VectorOfStrings       m_CustomPreMemCheck;
  tm_VectorOfStrings       m_CustomPostMemCheck;
  bool                     m_InteractiveDebugMode;
  
  void BlockTestErrorDiagnostics();
  
  int ExecuteCommands(tm_VectorOfStrings& vec);

  /**
   * Get the list of tests in directory and subdirectories.
   */
  void GetListOfTests(tm_ListOfTests* testlist, bool memcheck);

  //! Reread the configuration file
  void UpdateCTestConfiguration();

  /**
   * Generate the Dart compatible output
   */
  void GenerateDartTestOutput(std::ostream& os);
  void GenerateDartMemCheckOutput(std::ostream& os);
  void GenerateDartBuildOutput(std::ostream& os, 
                               std::vector<cmCTestBuildErrorWarning>,
                               double elapsed_time);

  bool OpenOutputFile(const std::string& path, 
                      const std::string& name, std::ofstream& stream);  
  std::string MakeXMLSafe(const std::string&);
  std::string MakeURLSafe(const std::string&);

  //! Run command specialized for make and configure. Returns process status
  // and retVal is return value or exception.
  int RunMakeCommand(const char* command, std::string* output,
    int* retVal, const char* dir, bool verbose, int timeout, 
    std::ofstream& ofs);

  //! Run command specialized for tests. Returns process status and retVal is
  // return value or exception.
  int RunTest(std::vector<const char*> args, std::string* output, int *retVal, 
    std::ostream* logfile);

  std::string GenerateRegressionImages(const std::string& xml);
  const char* GetTestStatus(int status);

  //! Start CTest XML output file
  void StartXML(std::ostream& ostr);

  //! End CTest XML output file
  void EndXML(std::ostream& ostr);

  //! Create not from files.
  int GenerateDartNotesOutput(std::ostream& os, const tm_VectorOfStrings& files);

  //! Parse Valgrind/Purify/Bounds Checker result out of the output string. After running,
  // log holds the output and results hold the different memmory errors.
  bool ProcessMemCheckOutput(const std::string& str, std::string& log, int* results);
  bool ProcessMemCheckValgrindOutput(const std::string& str, std::string& log, int* results);
  bool ProcessMemCheckPurifyOutput(const std::string& str, std::string& log, int* results);

  ///! Run CMake and build a test and then run it as a single test.
  int RunCMakeAndTest(std::string* output);
  ///! Initialize memory checking subsystem.
  bool InitializeMemoryChecking();
  ///! Find the running cmake
  void FindRunningCMake(const char* arg0);

  ///! Get the current time as string
  std::string CurrentTime();

  ///! Maximum size of testing string
  std::string::size_type m_MaximumPassedTestResultSize;
  std::string::size_type m_MaximumFailedTestResultSize;
};

#endif
