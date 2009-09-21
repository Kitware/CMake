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

#ifndef cmCTestBatchTestHandler_h
#define cmCTestBatchTestHandler_h

#include <cmStandardIncludes.h>
#include <cmCTestTestHandler.h>
#include <cmCTestMultiProcessHandler.h>
#include <cmCTestRunTest.h>

/** \class cmCTestBatchTestHandler
 * \brief run parallel ctest
 *
 * cmCTestBatchTestHandler 
 */
class cmCTestBatchTestHandler : public cmCTestMultiProcessHandler
{
public:
  ~cmCTestBatchTestHandler();
  virtual void RunTests();
protected:
  void WriteBatchScript();
  void WriteSrunArgs(int test, std::fstream& fout);
  void WriteTestCommand(int test, std::fstream& fout);

  void SubmitBatchScript();

  std::string Script;
};

#endif
