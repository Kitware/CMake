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

#ifndef cmCTestBuildHandler_h
#define cmCTestBuildHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

#include <cmsys/RegularExpression.hxx>

class cmMakefile;

/** \class cmCTestBuildHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestBuildHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestBuildHandler, cmCTestGenericHandler);

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  cmCTestBuildHandler();

  void PopulateCustomVectors(cmMakefile *mf);

  /**
   * Initialize handler
   */
  virtual void Initialize();

private:
  //! Run command specialized for make and configure. Returns process status
  // and retVal is return value or exception.
  int RunMakeCommand(const char* command,
    int* retVal, const char* dir, int timeout,
    std::ofstream& ofs);

  enum {
    b_REGULAR_LINE,
    b_WARNING_LINE,
    b_ERROR_LINE
  };

  class cmCTestCompileErrorWarningRex
    {
  public:
    cmCTestCompileErrorWarningRex() {}
    int FileIndex;
    int LineIndex;
    cmsys::RegularExpression RegularExpression;
    };

  struct cmCTestBuildErrorWarning
  {
    bool        Error;
    int         LogLine;
    std::string Text;
    std::string SourceFile;
    std::string SourceFileTail;
    int         LineNumber;
    std::string PreContext;
    std::string PostContext;
  };

  // generate the XML output
  void GenerateDartBuildOutput(std::ostream& os,
                               std::vector<cmCTestBuildErrorWarning>,
                               double elapsed_time);


  std::string             StartBuild;
  std::string             EndBuild;
  double                  StartBuildTime;
  double                  EndBuildTime;

  std::vector<cmStdString> CustomErrorMatches;
  std::vector<cmStdString> CustomErrorExceptions;
  std::vector<cmStdString> CustomWarningMatches;
  std::vector<cmStdString> CustomWarningExceptions;
  std::vector<cmCTestCompileErrorWarningRex> ErrorWarningFileLineRegex;

  std::vector<cmsys::RegularExpression> ErrorMatchRegex;
  std::vector<cmsys::RegularExpression> ErrorExceptionRegex;
  std::vector<cmsys::RegularExpression> WarningMatchRegex;
  std::vector<cmsys::RegularExpression> WarningExceptionRegex;

  typedef std::deque<char> t_BuildProcessingQueueType;

  void ProcessBuffer(const char* data, int length, size_t& tick,
    size_t tick_len, std::ofstream& ofs, t_BuildProcessingQueueType* queue);
  int ProcessSingleLine(const char* data);

  t_BuildProcessingQueueType            BuildProcessingQueue;
  t_BuildProcessingQueueType            BuildProcessingErrorQueue;
  size_t                                BuildOutputLogSize;
  std::vector<char>                     CurrentProcessingLine;

  cmStdString                           SimplifySourceDir;
  cmStdString                           SimplifyBuildDir;
  size_t                                OutputLineCounter;
  typedef std::vector<cmCTestBuildErrorWarning> t_ErrorsAndWarningsVector;
  t_ErrorsAndWarningsVector             ErrorsAndWarnings;
  t_ErrorsAndWarningsVector::iterator   LastErrorOrWarning;
  size_t                                PostContextCount;
  size_t                                MaxPreContext;
  size_t                                MaxPostContext;
  std::deque<cmStdString>               PreContext;

  int                                   TotalErrors;
  int                                   TotalWarnings;
  char                                  LastTickChar;

  bool                                  ErrorQuotaReached;
  bool                                  WarningQuotaReached;

  int                                   MaxErrors;
  int                                   MaxWarnings;
};

#endif
