/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "cmStandardIncludes.h"

class ctest
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
   * Do revision control update of directory
   */
  int UpdateDirectory();

  /**
   * Do configure the project
   */
  int ConfigureDirectory();

  /**
   * Run the test for a directory and any subdirectories
   */
  void ProcessDirectory(std::vector<std::string> &passed, 
                        std::vector<std::string> &failed);

  /**
   * Find the executable for a test
   */
  std::string FindExecutable(const char *exe);

  /**
   * Set the cmake test
   */
  bool SetTest(const char*);

  /**
   * constructor
   */
  ctest();

  bool m_UseIncludeRegExp;
  std::string m_IncludeRegExp;

  bool m_UseExcludeRegExp;
  bool m_UseExcludeRegExpFirst;
  std::string m_ExcludeRegExp;

  std::string m_ConfigType;
  bool m_Verbose;
  bool m_DartMode;

private:
  enum {
    FIRST_TEST    = 0,
    UPDATE_TEST,
    CONFIGURE_TEST,
    BUILD_TEST,
    TEST_TEST,
    COVERAGE_TEST,
    PURIFY_TEST,
    ALL_TEST,
    LAST_TEST
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

  typedef std::vector<cmCTestTestResult> tm_TestResultsVector;
  typedef std::map<std::string, std::string> tm_DartConfigurationMap;

  tm_TestResultsVector    m_TestResults;
  std::string             m_ToplevelPath;
  tm_DartConfigurationMap m_DartConfiguration;
  int                     m_Tests[LAST_TEST];
  
  std::string             m_CurrentTag;

  std::string             m_StartBuild;
  std::string             m_EndBuild;
  std::string             m_StartTest;
  std::string             m_EndTest;

  /**
   * Generate the Dart compatible output
   */
  void GenerateDartOutput(std::ostream& os);
  void GenerateDartBuildOutput(std::ostream& os, 
                               std::vector<cmCTestBuildErrorWarning>);
  
};

