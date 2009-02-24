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
#ifndef cmCTestSVN_h
#define cmCTestSVN_h

#include "cmCTestVC.h"

/** \class cmCTestSVN
 * \brief Interaction with subversion command-line tool
 *
 */
class cmCTestSVN: public cmCTestVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestSVN(cmCTest* ctest, std::ostream& log);

  virtual ~cmCTestSVN();

  int GetOldRevision() { return atoi(this->OldRevision.c_str()); }
  int GetNewRevision() { return atoi(this->NewRevision.c_str()); }
private:
  // Implement cmCTestVC internal API.
  virtual void CleanupImpl();
  virtual void NoteOldRevision();
  virtual void NoteNewRevision();

  // Old and new repository revisions.
  std::string OldRevision;
  std::string NewRevision;

  // URL of repository directory checked out in the working tree.
  std::string URL;

  // URL of repository root directory.
  std::string Root;

  // Directory under repository root checked out in working tree.
  std::string Base;

  std::string LoadInfo();

  // Parsing helper classes.
  class InfoParser;
  friend class InfoParser;
};

#endif
