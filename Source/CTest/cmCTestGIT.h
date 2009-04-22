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
#ifndef cmCTestGIT_h
#define cmCTestGIT_h

#include "cmCTestGlobalVC.h"

/** \class cmCTestGIT
 * \brief Interaction with git command-line tool
 *
 */
class cmCTestGIT: public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestGIT(cmCTest* ctest, std::ostream& log);

  virtual ~cmCTestGIT();

private:
  std::string GetWorkingRevision();
  virtual void NoteOldRevision();
  virtual void NoteNewRevision();
  virtual bool UpdateImpl();

  void LoadRevisions();
  void LoadModifications();

  // Parsing helper classes.
  class OneLineParser;
  class DiffParser;
  class CommitParser;
  friend class OneLineParser;
  friend class DiffParser;
  friend class CommitParser;
};

#endif
