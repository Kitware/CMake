/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmOrderDirectories_h
#define cmOrderDirectories_h

#include "cmStandardIncludes.h"

#include <cmsys/RegularExpression.hxx>

class cmGlobalGenerator;
class cmOrderDirectoriesConstraint;
class cmOrderDirectoriesConstraintLibrary;
class cmTarget;

/** \class cmOrderDirectories
 * \brief Compute a safe runtime path order for a set of shared libraries.
 */
class cmOrderDirectories
{
public:
  cmOrderDirectories(cmGlobalGenerator* gg, cmTarget* target,
                     const char* purpose);
  ~cmOrderDirectories();
  void AddRuntimeLibrary(std::string const& fullPath, const char* soname = 0);
  void AddLinkLibrary(std::string const& fullPath);
  void AddUserDirectories(std::vector<std::string> const& extra);
  void SetImplicitDirectories(std::set<cmStdString> const& implicitDirs);
  void SetLinkExtensionInfo(std::vector<std::string> const& linkExtensions,
                            std::string const& removeExtRegex);

  std::vector<std::string> const& GetOrderedDirectories();
private:
  cmGlobalGenerator* GlobalGenerator;
  cmTarget* Target;
  std::string Purpose;

  bool Computed;

  std::vector<std::string> OrderedDirectories;

  bool OrderedDirectoriesComputed;
  std::vector<cmOrderDirectoriesConstraint*> ConstraintEntries;
  std::vector<cmOrderDirectoriesConstraint*> ImplicitDirEntries;
  std::vector<std::string> UserDirectories;
  cmsys::RegularExpression RemoveLibraryExtension;
  std::vector<std::string> LinkExtensions;
  std::set<cmStdString> ImplicitDirectories;
  std::set<cmStdString> EmmittedConstraintSOName;
  std::set<cmStdString> EmmittedConstraintLibrary;
  std::vector<std::string> OriginalDirectories;
  std::map<cmStdString, int> DirectoryIndex;
  std::vector<int> DirectoryVisited;
  void CollectOriginalDirectories();
  int AddOriginalDirectory(std::string const& dir);
  void AddOriginalDirectories(std::vector<std::string> const& dirs);
  void FindConflicts();
  void FindImplicitConflicts();
  void OrderDirectories();
  void VisitDirectory(unsigned int i);
  void DiagnoseCycle();
  bool CycleDiagnosed;
  int WalkId;

  // Adjacency-list representation of runtime path ordering graph.
  // This maps from directory to those that must come *before* it.
  // Each entry that must come before is a pair.  The first element is
  // the index of the directory that must come first.  The second
  // element is the index of the runtime library that added the
  // constraint.
  typedef std::pair<int, int> ConflictPair;
  struct ConflictList: public std::vector<ConflictPair> {};
  std::vector<ConflictList> ConflictGraph;

  friend class cmOrderDirectoriesConstraint;
  friend class cmOrderDirectoriesConstraintLibrary;
};

#endif
