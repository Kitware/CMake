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

#ifndef cmCTestGenericHandler_h
#define cmCTestGenericHandler_h


#include "cmObject.h"

class cmCTest;
class cmMakefile;
class cmCTestCommand;
class cmGeneratedFileStream;

/** \class cmCTestGenericHandler
 * \brief A superclass of all CTest Handlers
 *
 */
class cmCTestGenericHandler : public cmObject
{
public:
  /**
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { this->HandlerVerbose = val; }

  /**
   * Populate internals from CTest custom scripts
   */
  virtual void PopulateCustomVectors(cmMakefile *) {}

  /**
   * Do the actual processing. Subclass has to override it.
   * Return < 0 if error.
   */
  virtual int ProcessHandler() = 0;

  /**
   * Process command line arguments that are applicable for the handler
   */
  virtual int ProcessCommandLineArguments(
    const std::string& /*currentArg*/, size_t& /*idx*/,
    const std::vector<std::string>& /*allArgs*/) { return 1; }

  /**
   * Initialize handler
   */
  virtual void Initialize();

  /**
   * Set the CTest instance
   */
  void SetCTestInstance(cmCTest* ctest) { this->CTest = ctest; }
  cmCTest* GetCTestInstance() { return this->CTest; }

  /**
   * Construct handler
   */
  cmCTestGenericHandler();
  virtual ~cmCTestGenericHandler();

  typedef std::map<cmStdString,cmStdString> t_StringToString;

  
  void SetPersistentOption(const char* op, const char* value);
  void SetOption(const char* op, const char* value);
  const char* GetOption(const char* op);

  void SetCommand(cmCTestCommand* command)
    {
    this->Command = command;
    }

  void SetSubmitIndex(int idx) { this->SubmitIndex = idx; }
  int GetSubmitIndex() { return this->SubmitIndex; }

  void SetAppendXML(bool b) { this->AppendXML = b; }

protected:
  bool StartResultingXML(const char* name, cmGeneratedFileStream& xofs);
  bool StartLogFile(const char* name, cmGeneratedFileStream& xofs);

  bool AppendXML;
  bool HandlerVerbose;
  cmCTest *CTest;
  t_StringToString Options;
  t_StringToString PersistentOptions;

  cmCTestCommand* Command;
  int SubmitIndex;
};

#endif

