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

private:
  // Implement cmCTestVC internal API.
  virtual void CleanupImpl();
  virtual void NoteOldRevision();
  virtual void NoteNewRevision();
  virtual bool UpdateImpl();
  virtual bool WriteXMLUpdates(std::ostream& xml);

  /** Represent a subversion-reported action for one path in a revision.  */
  struct Change
  {
    char Action;
    std::string Path;
    Change(): Action('?') {}
  };

  // Update status for files in each directory.
  class Directory: public std::map<cmStdString, File> {};
  std::map<cmStdString, Directory> Dirs;

  // Old and new repository revisions.
  std::string OldRevision;
  std::string NewRevision;

  // URL of repository directory checked out in the working tree.
  std::string URL;

  // URL of repository root directory.
  std::string Root;

  // Directory under repository root checked out in working tree.
  std::string Base;

  // Information known about old revision.
  Revision PriorRev;

  // Information about revisions from a svn log.
  std::list<Revision> Revisions;

  std::string LoadInfo();
  void LoadModifications();
  void LoadRevisions();

  void GuessBase(std::vector<Change> const& changes);
  const char* LocalPath(std::string const& path);

  void DoRevision(Revision const& revision,
                  std::vector<Change> const& changes);
  void WriteXMLDirectory(std::ostream& xml, std::string const& path,
                         Directory const& dir);

  // Parsing helper classes.
  class InfoParser;
  class LogParser;
  class StatusParser;
  class UpdateParser;
  friend class InfoParser;
  friend class LogParser;
  friend class StatusParser;
  friend class UpdateParser;
};

#endif
