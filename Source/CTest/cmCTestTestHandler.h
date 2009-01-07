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
#include <cmsys/RegularExpression.hxx>

class cmMakefile;

/** \class cmCTestTestHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestTestHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestTestHandler, cmCTestGenericHandler);

  /**
   * The main entry point for this class
   */
  int ProcessHandler();

  /**
   * When both -R and -I are used should te resulting test list be the
   * intersection or the union of the lists. By default it is the
   * intersection.
   */
  void SetUseUnion(bool val) { this->UseUnion = val; }

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

  /*
   * Add the test to the list of tests to be executed
   */
  bool AddTest(const std::vector<std::string>& args);

  /*
   * Set tests properties
   */
  bool SetTestsProperties(const std::vector<std::string>& args);

  void Initialize();

  // NOTE: This struct is Saved/Restored
  // in cmCTestTestHandler, if you add to this class
  // then you must add the new members to that code or
  // ctest -j N will break for that feature
  struct cmCTestTestProperties
  {
    cmStdString Name;
    cmStdString Directory;
    std::vector<std::string> Args;
    std::vector<std::string> Depends;
    std::vector<std::pair<cmsys::RegularExpression,
                          std::string> > ErrorRegularExpressions;
    std::vector<std::pair<cmsys::RegularExpression,
                          std::string> > RequiredRegularExpressions;
    std::map<cmStdString, cmStdString> Measurements;
    bool IsInBasedOnREOptions;
    bool WillFail;
    double Timeout;
    int Index;
    std::vector<std::string> Environment;
    std::vector<std::string> Labels;
  };

  struct cmCTestTestResult
  {
    std::string Name;
    std::string Path;
    std::string FullCommandLine;
    double      ExecutionTime;
    int         ReturnValue;
    int         Status;
    std::string CompletionStatus;
    std::string Output;
    std::string RegressionImages;
    int         TestCount;
    cmCTestTestProperties* Properties;
  };

  // add configuraitons to a search path for an executable
  static void AddConfigurations(cmCTest *ctest, 
                                std::vector<std::string> &attempted,
                                std::vector<std::string> &attemptedConfigs,
                                std::string filepath,
                                std::string &filename);

  // full signature static method to find an executable
  static std::string FindExecutable(cmCTest *ctest,
                                    const char *testCommand,
                                    std::string &resultingConfig,
                                    std::vector<std::string> &extraPaths,
                                    std::vector<std::string> &failed);

protected:
  // comput a final test list
  virtual int PreProcessHandler();
  virtual int PostProcessHandler();
  virtual void GenerateTestCommand(std::vector<const char*>& args);
  int ExecuteCommands(std::vector<cmStdString>& vec);

  //! Clean test output to specified length
  bool CleanTestOutput(std::string& output, size_t length);

  double                  ElapsedTestingTime;

  typedef std::vector<cmCTestTestResult> TestResultsVector;
  TestResultsVector    TestResults;

  std::vector<cmStdString> CustomTestsIgnore;
  std::string             StartTest;
  std::string             EndTest;
  unsigned int            StartTestTime;
  unsigned int            EndTestTime;
  bool MemCheck;
  int CustomMaximumPassedTestOutputSize;
  int CustomMaximumFailedTestOutputSize;
protected:
  /**
   *  Run one test
   */
  virtual void ProcessOneTest(cmCTestTestProperties *props,
                              std::vector<cmStdString> &passed,
                              std::vector<cmStdString> &failed,
                              int count, int tmsize);



public:
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

private:
  /**
   * Generate the Dart compatible output
   */
  virtual void GenerateDartOutput(std::ostream& os);

  /**
   * Run the tests for a directory and any subdirectories
   */
  void ProcessDirectory(std::vector<cmStdString> &passed,
                        std::vector<cmStdString> &failed);

  typedef std::vector<cmCTestTestProperties> ListOfTests;
  /**
   * Get the list of tests in directory and subdirectories.
   */
  void GetListOfTests();
  // compute the lists of tests that will actually run
  // based on union regex and -I stuff
  void ComputeTestList();
  
  // Save the state of the test list and return the file
  // name it was saved to
  std::string SaveTestList();
  void LoadTestList();
  bool GetValue(const char* tag,
                std::string& value,
                std::ifstream& fin);
  bool GetValue(const char* tag,
                int& value,
                std::ifstream& fin);
  bool GetValue(const char* tag,
                size_t& value,
                std::ifstream& fin);
  bool GetValue(const char* tag,
                bool& value,
                std::ifstream& fin);
  bool GetValue(const char* tag,
                double& value,
                std::ifstream& fin);
  // run in -j N mode
  void ProcessParallel(std::vector<cmStdString> &passed,
                       std::vector<cmStdString> &failed);
  /**
   * Find the executable for a test
   */
  std::string FindTheExecutable(const char *exe);

  const char* GetTestStatus(int status);
  void ExpandTestsToRunInformation(size_t numPossibleTests);

  std::vector<cmStdString> CustomPreTest;
  std::vector<cmStdString> CustomPostTest;

  std::vector<int>        TestsToRun;

  bool UseIncludeRegExpFlag;
  bool UseExcludeRegExpFlag;
  bool UseExcludeRegExpFirst;
  std::string IncludeRegExp;
  std::string ExcludeRegExp;
  cmsys::RegularExpression IncludeTestsRegularExpression;
  cmsys::RegularExpression ExcludeTestsRegularExpression;

  std::string GenerateRegressionImages(const std::string& xml);

  std::string TestsToRunString;
  bool UseUnion;
  ListOfTests TestList;
  size_t TotalNumberOfTests;
  cmsys::RegularExpression DartStuff;

  std::ostream* LogFile;
};

#endif
