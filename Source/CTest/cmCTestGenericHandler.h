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
  void SetVerbose(bool val) { m_HandlerVerbose = val; }

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
  virtual void Initialize() = 0;

  /**
   * Set the CTest instance
   */
  void SetCTestInstance(cmCTest* ctest) { m_CTest = ctest; }
  cmCTest* GetCTestInstance() { return m_CTest; }

  /**
   * Construct handler
   */
  cmCTestGenericHandler();
  virtual ~cmCTestGenericHandler();

  typedef std::map<cmStdString,cmStdString> t_StringToString;

  void SetOption(const char* op, const char* value);
  const char* GetOption(const char* op);

  void SetCommand(cmCTestCommand* command)
    {
    m_Command = command;
    }

protected:
  bool m_HandlerVerbose;
  cmCTest *m_CTest;
  t_StringToString m_Options;

  cmCTestCommand* m_Command;
};

#endif

