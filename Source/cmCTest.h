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

class cmCTest
{
public:
  /**
   * Initialize and finalize testing
   */
  void Initialize();
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
  int TestDirectory();

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
  void ProcessDirectory(std::vector<std::string> &passed, 
                        std::vector<std::string> &failed);

  /**
   * Find the executable for a test
   */
  std::string FindTheExecutable(const char *exe);

  /**
   * Set the cmake test
   */
  bool SetTest(const char*);

  /**
   * Set the cmake test mode (experimental, nightly, continuous).
   */
  void SetTestModel(int mode)
    {
    m_TestModel = mode;
    }
  std::string GetTestModelString();

  /**
   * constructor
   */
  cmCTest();

  bool m_UseIncludeRegExp;
  std::string m_IncludeRegExp;

  bool m_UseExcludeRegExp;
  bool m_UseExcludeRegExpFirst;
  std::string m_ExcludeRegExp;

  std::string m_ConfigType;
  bool m_Verbose;
  bool m_DartMode;
  bool m_ShowOnly;

  enum {
    EXPERIMENTAL,
    NIGHTLY,
    CONTINUOUS
  };

private:
  enum {
    FIRST_TEST     = 0,
    UPDATE_TEST    = 1,
    START_TEST     = 2,
    CONFIGURE_TEST = 3,
    BUILD_TEST     = 4,
    TEST_TEST      = 5,
    COVERAGE_TEST  = 6,
    PURIFY_TEST    = 7,
    SUBMIT_TEST    = 8,
    ALL_TEST       = 9,
    LAST_TEST      = 10
  };

  struct cmCTestTestResult
  {
    std::string m_Name;
    std::string m_Path;
    std::string m_FullCommandLine;
    double      m_ExecutionTime;
    int         m_ReturnValue;
    std::string m_CompletionStatus;
    std::string m_Output;
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
        m_FullPath = "";
        m_Covered = false;
        m_Tested = 0;
        m_UnTested = 0;
        m_Lines.clear();
      }
    std::string      m_FullPath;
    bool             m_Covered;
    int              m_Tested;
    int              m_UnTested;
    std::vector<int> m_Lines;
  };

  typedef std::vector<cmCTestTestResult> tm_TestResultsVector;
  typedef std::map<std::string, std::string> tm_DartConfigurationMap;
  typedef std::map<std::string, cmCTestCoverage> tm_CoverageMap;

  tm_TestResultsVector    m_TestResults;
  std::string             m_ToplevelPath;
  tm_DartConfigurationMap m_DartConfiguration;
  int                     m_Tests[LAST_TEST];
  
  std::string             m_CurrentTag;

  std::string             m_StartBuild;
  std::string             m_EndBuild;
  std::string             m_StartTest;
  std::string             m_EndTest;

  int                     m_TestModel;

  /**
   * Generate the Dart compatible output
   */
  void GenerateDartOutput(std::ostream& os);
  void GenerateDartBuildOutput(std::ostream& os, 
                               std::vector<cmCTestBuildErrorWarning>);

  bool OpenOutputFile(const std::string& path, 
                      const std::string& name, std::ofstream& stream);  
  std::string MakeXMLSafe(const std::string&);
};

#endif
