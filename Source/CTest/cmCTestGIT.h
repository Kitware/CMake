/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestGIT_h
#define cmCTestGIT_h

#include "cmCTestGlobalVC.h"

/** \class cmCTestGIT
 * \brief Interaction with git command-line tool
 *
 */
class cmCTestGIT : public cmCTestGlobalVC
{
public:
  /** Construct with a CTest instance and update log stream.  */
  cmCTestGIT(cmCTest* ctest, std::ostream& log);

  ~cmCTestGIT() CM_OVERRIDE;

private:
  unsigned int CurrentGitVersion;
  unsigned int GetGitVersion();
  std::string GetWorkingRevision();
  void NoteOldRevision() CM_OVERRIDE;
  void NoteNewRevision() CM_OVERRIDE;
  bool UpdateImpl() CM_OVERRIDE;

  std::string FindGitDir();
  std::string FindTopDir();

  bool UpdateByFetchAndReset();
  bool UpdateByCustom(std::string const& custom);
  bool UpdateInternal();

  void LoadRevisions() CM_OVERRIDE;
  void LoadModifications() CM_OVERRIDE;

  // "public" needed by older Sun compilers
public:
  // Parsing helper classes.
  class OneLineParser;
  class DiffParser;
  class CommitParser;
  friend class OneLineParser;
  friend class DiffParser;
  friend class CommitParser;
};

#endif
