/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestBZR_h
#define cmCTestBZR_h

#include "cmCTestGlobalVC.h"

/** \class cmCTestBZR
 * \brief Interaction with bzr command-line tool
 *
 */
class cmCTestBZR : public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestBZR(cmCTest* ctest, std::ostream& log);

  ~cmCTestBZR() CM_OVERRIDE;

private:
  // Implement cmCTestVC internal API.
  void NoteOldRevision() CM_OVERRIDE;
  void NoteNewRevision() CM_OVERRIDE;
  bool UpdateImpl() CM_OVERRIDE;

  // URL of repository directory checked out in the working tree.
  std::string URL;

  std::string LoadInfo();
  void LoadModifications() CM_OVERRIDE;
  void LoadRevisions() CM_OVERRIDE;

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
