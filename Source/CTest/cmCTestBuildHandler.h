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

  /*
   * The main entry point for this class
   */
  int ProcessHandler();
  
  cmCTestBuildHandler();
  
  void PopulateCustomVectors(cmMakefile *mf);

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
    int m_FileIndex;
    int m_LineIndex;
    cmsys::RegularExpression m_RegularExpression;
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

  // generate the XML output
  void GenerateDartBuildOutput(std::ostream& os, 
                               std::vector<cmCTestBuildErrorWarning>,
                               double elapsed_time);
  

  std::string             m_StartBuild;
  std::string             m_EndBuild;
  
  std::vector<cmStdString> m_CustomErrorMatches;
  std::vector<cmStdString> m_CustomErrorExceptions;
  std::vector<cmStdString> m_CustomWarningMatches;
  std::vector<cmStdString> m_CustomWarningExceptions;
  std::vector<cmCTestCompileErrorWarningRex> m_ErrorWarningFileLineRegex;

  std::vector<cmsys::RegularExpression> m_ErrorMatchRegex;
  std::vector<cmsys::RegularExpression> m_ErrorExceptionRegex;
  std::vector<cmsys::RegularExpression> m_WarningMatchRegex;
  std::vector<cmsys::RegularExpression> m_WarningExceptionRegex;

  void ProcessBuffer(const char* data, int length, size_t& tick, size_t tick_len, 
    std::ofstream& ofs);
  int ProcessSingleLine(const char* data);

  typedef std::deque<char> t_BuildProcessingQueueType;
  t_BuildProcessingQueueType            m_BuildProcessingQueue;
  t_BuildProcessingQueueType::iterator  m_BuildProcessingQueueLocation;
  size_t                                m_BuildOutputLogSize;
  std::vector<char>                     m_CurrentProcessingLine;

  cmStdString                           m_SimplifySourceDir;
  cmStdString                           m_SimplifyBuildDir;
  size_t                                m_OutputLineCounter;
  typedef std::vector<cmCTestBuildErrorWarning> t_ErrorsAndWarningsVector;
  t_ErrorsAndWarningsVector             m_ErrorsAndWarnings;
  t_ErrorsAndWarningsVector::iterator   m_LastErrorOrWarning;
  size_t                                m_PostContextCount;
  size_t                                m_MaxPreContext;
  size_t                                m_MaxPostContext;
  std::deque<cmStdString>               m_PreContext;

  int                                   m_TotalErrors;
  int                                   m_TotalWarnings;
  char                                  m_LastTickChar;
};

#endif
