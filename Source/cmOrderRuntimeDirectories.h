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
#ifndef cmOrderRuntimeDirectories_h
#define cmOrderRuntimeDirectories_h

#include "cmStandardIncludes.h"

class cmGlobalGenerator;

/** \class cmOrderRuntimeDirectories
 * \brief Compute a safe runtime path order for a set of shared libraries.
 */
class cmOrderRuntimeDirectories
{
public:
  cmOrderRuntimeDirectories(cmGlobalGenerator* gg, const char* name,
                            const char* purpose);
  void AddLibrary(std::string const& fullPath, const char* soname = 0);
  void AddDirectories(std::vector<std::string> const& extra);

  std::vector<std::string> const& GetRuntimePath();
private:
  cmGlobalGenerator* GlobalGenerator;
  std::string Name;
  std::string Purpose;

  bool Computed;

  std::vector<std::string> RuntimeSearchPath;

  // Runtime path computation.
  struct LibraryRuntimeEntry
  {
    // The file name of the library.
    std::string FileName;

    // The soname of the shared library if it is known.
    std::string SOName;

    // The directory in which the library is supposed to be found.
    std::string Directory;

    // The index assigned to the directory.
    int DirectoryIndex;
  };
  bool RuntimeSearchPathComputed;
  std::vector<LibraryRuntimeEntry> LibraryRuntimeInfo;
  std::vector<std::string> UserDirectories;
  std::set<cmStdString> LibraryRuntimeInfoEmmitted;
  std::vector<std::string> RuntimeDirectories;
  std::map<cmStdString, int> RuntimeDirectoryIndex;
  std::vector<int> RuntimeDirectoryVisited;
  void CollectRuntimeDirectories();
  int AddRuntimeDirectory(std::string const& dir);
  void FindConflictingLibraries();
  void FindDirectoriesForLib(unsigned int lri);
  void OrderRuntimeSearchPath();
  void VisitRuntimeDirectory(unsigned int i);
  void DiagnoseCycle();
  bool CycleDiagnosed;
  int WalkId;

  // Adjacency-list representation of runtime path ordering graph.
  // This maps from directory to those that must come *before* it.
  // Each entry that must come before is a pair.  The first element is
  // the index of the directory that must come first.  The second
  // element is the index of the runtime library that added the
  // constraint.
  typedef std::pair<int, int> RuntimeConflictPair;
  struct RuntimeConflictList: public std::vector<RuntimeConflictPair> {};
  std::vector<RuntimeConflictList> RuntimeConflictGraph;
};

#endif
