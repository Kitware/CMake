/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestHG_h
#define cmCTestHG_h

#include "cmCTestGlobalVC.h"

/** \class cmCTestHG
 * \brief Interaction with Mercurial command-line tool
 *
 */
class cmCTestHG : public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestHG(cmCTest* ctest, std::ostream& log);

  ~cmCTestHG() CM_OVERRIDE;

private:
  std::string GetWorkingRevision();
  void NoteOldRevision() CM_OVERRIDE;
  void NoteNewRevision() CM_OVERRIDE;
  bool UpdateImpl() CM_OVERRIDE;

  void LoadRevisions() CM_OVERRIDE;
  void LoadModifications() CM_OVERRIDE;

  // Parsing helper classes.
  class IdentifyParser;
  class StatusParser;
  class LogParser;
  friend class IdentifyParser;
  friend class StatusParser;
  friend class LogParser;
};

#endif
