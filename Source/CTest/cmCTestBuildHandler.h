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


#include "cmStandardIncludes.h"
#include "cmListFileCache.h"

#include <cmsys/RegularExpression.hxx>

class cmCTest;
class cmMakefile;

/** \class cmCTestBuildHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestBuildHandler
{
public:

  /*
   * The main entry point for this class
   */
  int BuildDirectory(cmCTest *);
  
  /*
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { m_Verbose = val; }
  
  cmCTestBuildHandler();
  
  void PopulateCustomVectors(cmMakefile *mf);

  struct cmCTestCompileErrorWarningRex
    {
    char* m_RegularExpressionString;
    int m_FileIndex;
    int m_LineIndex;
    cmsys::RegularExpression m_RegularExpression;
    };

private:
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
  

  bool m_Verbose;
  cmCTest *m_CTest;

  std::string             m_StartBuild;
  std::string             m_EndBuild;
  
  std::vector<cmStdString> m_CustomErrorMatches;
  std::vector<cmStdString> m_CustomErrorExceptions;
  std::vector<cmStdString> m_CustomWarningMatches;
  std::vector<cmStdString> m_CustomWarningExceptions;
};

#endif
