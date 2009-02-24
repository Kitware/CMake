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
#ifndef cmCTestVC_h
#define cmCTestVC_h

#include "cmProcessTools.h"

class cmCTest;

/** \class cmCTestVC
 * \brief Base class for version control system handlers
 *
 */
class cmCTestVC: public cmProcessTools
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestVC(cmCTest* ctest, std::ostream& log);

  virtual ~cmCTestVC();

  /** Command line tool to invoke.  */
  void SetCommandLineTool(std::string const& tool);

  /** Top-level source directory.  */
  void SetSourceDirectory(std::string const& dir);

  /** Perform cleanup operations on the work tree.  */
  void Cleanup();

protected:
  // Internal API to be implemented by subclasses.
  virtual void CleanupImpl();

  /** Convert a list of arguments to a human-readable command line.  */
  static std::string ComputeCommandLine(char const* const* cmd);

  /** Run a command line and send output to given parsers.  */
  bool RunChild(char const* const* cmd, OutputParser* out,
                OutputParser* err, const char* workDir = 0);

  // Instance of cmCTest running the script.
  cmCTest* CTest;

  // A stream to which we write log information.
  std::ostream& Log;

  // Basic information about the working tree.
  std::string CommandLineTool;
  std::string SourceDirectory;
};

#endif
