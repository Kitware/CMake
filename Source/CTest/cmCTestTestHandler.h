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


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

class cmMakefile;

/** \class cmCTestTestHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestTestHandler : public cmCTestGenericHandler
{
public:

  /**
   * The main entry point for this class
   */
  int ProcessHandler();
  
  /**
   * When both -R and -I are used should te resulting test list be the
   * intersection or the union of the lists. By default it is the
   * intersection.
   */
  void SetUseUnion(bool val) { m_UseUnion = val; }

  /**
   * This method is called when reading CTest custom file
   */
  void PopulateCustomVectors(cmMakefile *mf);
  
  ///! Control the use of the regular expresisons, call these methods to turn
  ///them on
  void UseIncludeRegExp();
  void UseExcludeRegExp();
  void SetIncludeRegExp(const char *);
  void SetExcludeRegExp(const char *);
  

  ///! pass the -I argument down
  void SetTestsToRunInformation(const char*);

  cmCTestTestHandler();

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

  typedef std::vector<cmListFileArgument> tm_VectorOfListFileArgs;

protected:
  struct cmCTestTestProperties
  {
    cmStdString m_Name;
    cmStdString m_Directory;
    tm_VectorOfListFileArgs m_Args;
    bool m_IsInBasedOnREOptions;
  };


  virtual int PreProcessHandler();
  virtual int PostProcessHandler();
  virtual void GenerateTestCommand(std::vector<const char*>& args);
  int ExecuteCommands(std::vector<cmStdString>& vec);

  double                  m_ElapsedTestingTime;

  typedef std::vector<cmCTestTestResult> tm_TestResultsVector;
  tm_TestResultsVector    m_TestResults;

  std::vector<cmStdString> m_CustomTestsIgnore;
  std::string             m_StartTest;
  std::string             m_EndTest;
  bool m_MemCheck;

private:
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


  /**
   * Generate the Dart compatible output
   */
  virtual void GenerateDartOutput(std::ostream& os);

  /**
   * Run the test for a directory and any subdirectories
   */
  void ProcessDirectory(std::vector<cmStdString> &passed, 
                        std::vector<cmStdString> &failed);
  

  typedef std::vector<cmCTestTestProperties> tm_ListOfTests;
  /**
   * Get the list of tests in directory and subdirectories.
   */
  void GetListOfTests(tm_ListOfTests* testlist);

  /**
   * Find the executable for a test
   */
  std::string FindTheExecutable(const char *exe);

  const char* GetTestStatus(int status);
  void ExpandTestsToRunInformation(int numPossibleTests);

  std::vector<cmStdString> m_CustomPreTest;
  std::vector<cmStdString> m_CustomPostTest;

  int m_CustomMaximumPassedTestOutputSize;
  int m_CustomMaximumFailedTestOutputSize;

  std::vector<int>        m_TestsToRun;

  bool m_UseIncludeRegExp;
  bool m_UseExcludeRegExp;
  bool m_UseExcludeRegExpFirst;
  std::string m_IncludeRegExp;
  std::string m_ExcludeRegExp;

  std::string GenerateRegressionImages(const std::string& xml);

  //! Clean test output to specified length
  bool CleanTestOutput(std::string& output, size_t length);

  std::string TestsToRunString;
  bool m_UseUnion;
};

#endif
