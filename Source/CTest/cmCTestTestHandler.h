/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCTestTestHandler_h
#define cmCTestTestHandler_h


#include "cmStandardIncludes.h"
#include "cmListFileCache.h"

class cmCTest;
class cmMakefile;

/** \class cmCTestTestHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestTestHandler
{
public:

  /*
   * The main entry point for this class
   */
  int TestDirectory(cmCTest *, bool memcheck);
  
  /*
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { m_Verbose = val; }
  
  void PopulateCustomVectors(cmMakefile *mf);

  ///! Control the use of the regular expresisons, call these methods to turn
  ///them on
  void UseIncludeRegExp();
  void UseExcludeRegExp();
  void SetIncludeRegExp(const char *);
  void SetExcludeRegExp(const char *);
  
  cmCTestTestHandler();

  ///! pass the -I argument down
  void SetTestsToRunInformation(const char*);

  typedef std::vector<cmListFileArgument> tm_VectorOfListFileArgs;

private:

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

  bool m_Verbose;
  cmCTest *m_CTest;

  std::string              m_MemoryTester;
  std::vector<cmStdString> m_MemoryTesterOptionsParsed;
  std::string              m_MemoryTesterOptions;
  int                      m_MemoryTesterStyle;
  std::string              m_MemoryTesterOutputFile;
  int                      m_MemoryTesterGlobalResults[NO_MEMORY_FAULT];

  
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

  typedef std::vector<cmCTestTestResult> tm_TestResultsVector;
  tm_TestResultsVector    m_TestResults;

  int ExecuteCommands(std::vector<cmStdString>& vec);

  ///! Initialize memory checking subsystem.
  bool InitializeMemoryChecking();

  /**
   * Generate the Dart compatible output
   */
  void GenerateDartTestOutput(std::ostream& os);
  void GenerateDartMemCheckOutput(std::ostream& os);

  /**
   * Run the test for a directory and any subdirectories
   */
  void ProcessDirectory(std::vector<cmStdString> &passed, 
                        std::vector<cmStdString> &failed,
                        bool memcheck);

  struct cmCTestTestProperties
    {
    cmStdString m_Name;
    cmStdString m_Directory;
    tm_VectorOfListFileArgs m_Args;
    };

  typedef std::vector<cmCTestTestProperties> tm_ListOfTests;
  /**
   * Get the list of tests in directory and subdirectories.
   */
  void GetListOfTests(tm_ListOfTests* testlist, bool memcheck);

  /**
   * Find the executable for a test
   */
  std::string FindTheExecutable(const char *exe);

  const char* GetTestStatus(int status);
  void ExpandTestsToRunInformation(int numPossibleTests);

  std::vector<cmStdString> m_CustomPreTest;
  std::vector<cmStdString> m_CustomPostTest;
  std::vector<cmStdString> m_CustomPreMemCheck;
  std::vector<cmStdString> m_CustomPostMemCheck;
  std::vector<cmStdString> m_CustomTestsIgnore;
  std::vector<cmStdString> m_CustomMemCheckIgnore;

  std::string             m_StartTest;
  std::string             m_EndTest;
  double                  m_ElapsedTestingTime;
  std::vector<int>        m_TestsToRun;

  bool m_UseIncludeRegExp;
  bool m_UseExcludeRegExp;
  bool m_UseExcludeRegExpFirst;
  std::string m_IncludeRegExp;
  std::string m_ExcludeRegExp;

  std::string GenerateRegressionImages(const std::string& xml);

  //! Parse Valgrind/Purify/Bounds Checker result out of the output
  //string. After running, log holds the output and results hold the
  //different memmory errors.
  bool ProcessMemCheckOutput(const std::string& str, 
                             std::string& log, int* results);
  bool ProcessMemCheckValgrindOutput(const std::string& str, 
                                     std::string& log, int* results);
  bool ProcessMemCheckPurifyOutput(const std::string& str, 
                                   std::string& log, int* results);

  ///! Maximum size of testing string
  std::string::size_type m_MaximumPassedTestResultSize;
  std::string::size_type m_MaximumFailedTestResultSize;

  std::string TestsToRunString;
  
};

#endif
