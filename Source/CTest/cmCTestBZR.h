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
#ifndef cmCTestBZR_h
#define cmCTestBZR_h

#include "cmCTestGlobalVC.h"

/** \class cmCTestBZR
 * \brief Interaction with bzr command-line tool
 *
 */
class cmCTestBZR: public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestBZR(cmCTest* ctest, std::ostream& log);

  virtual ~cmCTestBZR();

private:
  // Implement cmCTestVC internal API.
  virtual void NoteOldRevision();
  virtual void NoteNewRevision();
  virtual bool UpdateImpl();

  // URL of repository directory checked out in the working tree.
  std::string URL;

  std::string LoadInfo();
  void LoadModifications();
  void LoadRevisions();

  // Parsing helper classes.
  class InfoParser;
  class RevnoParser;
  class LogParser;
  class UpdateParser;
  class StatusParser;
  friend class InfoParser;
  friend class RevnoParser;
  friend class LogParser;
  friend class UpdateParser;
  friend class StatusParser;
};

#endif
