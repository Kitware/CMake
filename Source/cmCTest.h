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

class cmMakefile;
class cmCTestBuildHandler;
class cmCTestConfigureHandler;
class cmCTestCoverageHandler;
class cmCTestScriptHandler;
class cmCTestTestHandler;
class cmCTestUpdateHandler;

class cmCTest
{
public:
  typedef std::vector<cmStdString> tm_VectorOfStrings;

  ///! Process Command line arguments
  int Run(std::vector<std::string>const&, std::string* output = 0);
  
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

  /*
   * A utility function that returns the nightly time
   */
  static struct tm* GetNightlyTime(std::string str, 
                                   bool verbose, 
                                   bool tomorrowtag);
  
  /*
   * Is the tomorrow tag set?
   */
  bool GetTomorrowTag() { return m_TomorrowTag; };
      
  /**
   * Try to run tests of the project
   */
  int TestDirectory(bool memcheck);

  /**
   * Do submit testing results
   */
  int SubmitResults();
  std::string GetSubmitResultsPrefix();

  ///! what is the configuraiton type, e.g. Debug, Release etc.
  std::string GetConfigType();
  
  /**
   * Check if CTest file exists
   */
  bool CTestFileExists(const std::string& filename);

  /**
   * Set the cmake test
   */
  bool SetTest(const char*, bool report = true);

  /**
   * Set the cmake test mode (experimental, nightly, continuous).
   */
  void SetTestModel(int mode);
  int GetTestModel() { return m_TestModel; };
  
  std::string GetTestModelString();
  static int GetTestModelFromString(const char* str);
  static std::string CleanString(const std::string& str);
  std::string GetDartConfiguration(const char *name);
  
  /**
   * constructor and destructor
   */
  cmCTest();
  ~cmCTest();
  
  //! Set the notes files to be created.
  void SetNotesFiles(const char* notes);

  std::string m_ConfigType;
  bool m_Verbose;
  bool m_DartMode;

  bool m_ForceNewCTestProcess;

  bool m_RunConfigurationScript;

  enum {
    EXPERIMENTAL,
    NIGHTLY,
    CONTINUOUS
  };

  // provide some more detailed info on the return code for ctest
  enum {
    UPDATE_ERRORS = 0x01,
    CONFIGURE_ERRORS = 0x02,
    BUILD_ERRORS = 0x04,
    TEST_ERRORS = 0x08,
    MEMORY_ERRORS = 0x10
  };

  int GenerateNotesFile(const char* files);

  bool OpenOutputFile(const std::string& path, 
                      const std::string& name, std::ofstream& stream);  
  static std::string MakeXMLSafe(const std::string&);
  static std::string MakeURLSafe(const std::string&);
  
  /*
   * return the current tag
   */
  std::string GetCurrentTag();

  ///! Get the current time as string
  std::string CurrentTime();
  
  ///! Should we only show what we would do?
  bool GetShowOnly();
  
  ///! Are we producing XML
  bool GetProduceXML();

  //! Start CTest XML output file
  void StartXML(std::ostream& ostr);

  //! End CTest XML output file
  void EndXML(std::ostream& ostr);

  //! Run command specialized for make and configure. Returns process status
  // and retVal is return value or exception.
  int RunMakeCommand(const char* command, std::string* output,
    int* retVal, const char* dir, bool verbose, int timeout, 
    std::ofstream& ofs);

  static void PopulateCustomVector(cmMakefile* mf, const char* definition, 
                                   tm_VectorOfStrings& vec);

  std::string GetToplevelPath();
  
  //! Run command specialized for tests. Returns process status and retVal is
  // return value or exception.
  int RunTest(std::vector<const char*> args, std::string* output, int *retVal, 
    std::ostream* logfile);

  //! Get the path to CTest
  const char* GetCTestExecutable() { return m_CTestSelf.c_str(); }


private:
  // these are helper classes
  cmCTestBuildHandler     *BuildHandler;
  cmCTestCoverageHandler  *CoverageHandler;
  cmCTestScriptHandler    *ScriptHandler;
  cmCTestTestHandler      *TestHandler;
  cmCTestUpdateHandler    *UpdateHandler;
  cmCTestConfigureHandler *ConfigureHandler;
  
  bool m_ShowOnly;

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
  typedef std::map<cmStdString, cmStdString> tm_DartConfigurationMap;

  std::string             m_ToplevelPath;
  tm_DartConfigurationMap m_DartConfiguration;
  int                     m_Tests[LAST_TEST];
  
  std::string             m_CurrentTag;
  bool                    m_TomorrowTag;

  int                     m_TestModel;

  double                  m_TimeOut;

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
  

  int ReadCustomConfigurationFileTree(const char* dir);

  bool                     m_InteractiveDebugMode;

  bool                     m_ShortDateFormat;
  
  void BlockTestErrorDiagnostics();
  

  //! Reread the configuration file
  void UpdateCTestConfiguration();

  //! Create not from files.
  int GenerateDartNotesOutput(std::ostream& os, const tm_VectorOfStrings& files);

  ///! Run CMake and build a test and then run it as a single test.
  int RunCMakeAndTest(std::string* output);
  ///! Find the running cmake
  void FindRunningCMake(const char* arg0);

};

#endif
