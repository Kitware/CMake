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
};

#endif
