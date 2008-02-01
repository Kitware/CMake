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
#include "cmOrderRuntimeDirectories.h"

#include "cmGlobalGenerator.h"
#include "cmSystemTools.h"

#include <algorithm>

/*
Directory ordering computation.
  - Useful to compute a safe runtime library path order
  - Need runtime path for supporting INSTALL_RPATH_USE_LINK_PATH
  - Need runtime path at link time to pickup transitive link dependencies
    for shared libraries.
*/

//----------------------------------------------------------------------------
cmOrderRuntimeDirectories::cmOrderRuntimeDirectories(cmGlobalGenerator* gg,
                                                     const char* name,
                                                     const char* purpose)
{
  this->GlobalGenerator = gg;
  this->Name = name;
  this->Purpose = purpose;
  this->Computed = false;
}

//----------------------------------------------------------------------------
std::vector<std::string> const& cmOrderRuntimeDirectories::GetRuntimePath()
{
  if(!this->Computed)
    {
    this->Computed = true;
    this->CollectRuntimeDirectories();
    this->FindConflictingLibraries();
    this->OrderRuntimeSearchPath();
    }
  return this->RuntimeSearchPath;
}

//----------------------------------------------------------------------------
void cmOrderRuntimeDirectories::AddLibrary(std::string const& fullPath,
                                           const char* soname)
{
  // Add the runtime information at most once.
  if(this->LibraryRuntimeInfoEmmitted.insert(fullPath).second)
    {
    // Construct the runtime information entry for this library.
    LibraryRuntimeEntry entry;
    entry.FileName =  cmSystemTools::GetFilenameName(fullPath);
    entry.SOName = soname? soname : "";
    entry.Directory = cmSystemTools::GetFilenamePath(fullPath);
    this->LibraryRuntimeInfo.push_back(entry);
    }
  else
    {
    // This can happen if the same library is linked multiple times.
    // In that case the runtime information check need be done only
    // once anyway.  For shared libs we could add a check in AddItem
    // to not repeat them.
    }
}

//----------------------------------------------------------------------------
void
cmOrderRuntimeDirectories
::AddDirectories(std::vector<std::string> const& extra)
{
  this->UserDirectories.insert(this->UserDirectories.end(),
                               extra.begin(), extra.end());
}

//----------------------------------------------------------------------------
void cmOrderRuntimeDirectories::CollectRuntimeDirectories()
{
  // Get all directories that should be in the runtime search path.

  // Add directories containing libraries.
  for(std::vector<LibraryRuntimeEntry>::iterator
        ei = this->LibraryRuntimeInfo.begin();
      ei != this->LibraryRuntimeInfo.end(); ++ei)
    {
    ei->DirectoryIndex = this->AddRuntimeDirectory(ei->Directory);
    }

  // Add link directories specified for inclusion.
  for(std::vector<std::string>::const_iterator
        di = this->UserDirectories.begin();
      di != this->UserDirectories.end(); ++di)
    {
    this->AddRuntimeDirectory(*di);
    }
}

//----------------------------------------------------------------------------
int cmOrderRuntimeDirectories::AddRuntimeDirectory(std::string const& dir)
{
  // Add the runtime directory with a unique index.
  std::map<cmStdString, int>::iterator i =
    this->RuntimeDirectoryIndex.find(dir);
  if(i == this->RuntimeDirectoryIndex.end())
    {
    std::map<cmStdString, int>::value_type
      entry(dir, static_cast<int>(this->RuntimeDirectories.size()));
    i = this->RuntimeDirectoryIndex.insert(entry).first;
    this->RuntimeDirectories.push_back(dir);
    }

  return i->second;
}

//----------------------------------------------------------------------------
struct cmOrderRuntimeDirectoriesCompare
{
  typedef std::pair<int, int> RuntimeConflictPair;

  // The conflict pair is unique based on just the directory
  // (first).  The second element is only used for displaying
  // information about why the entry is present.
  bool operator()(RuntimeConflictPair const& l,
                  RuntimeConflictPair const& r)
    {
    return l.first == r.first;
    }
};

//----------------------------------------------------------------------------
void cmOrderRuntimeDirectories::FindConflictingLibraries()
{
  // Allocate the conflict graph.
  this->RuntimeConflictGraph.resize(this->RuntimeDirectories.size());
  this->RuntimeDirectoryVisited.resize(this->RuntimeDirectories.size(), 0);

  // Find all runtime directories providing each library.
  for(unsigned int lri = 0; lri < this->LibraryRuntimeInfo.size(); ++lri)
    {
    this->FindDirectoriesForLib(lri);
    }

  // Clean up the conflict graph representation.
  for(std::vector<RuntimeConflictList>::iterator
        i = this->RuntimeConflictGraph.begin();
      i != this->RuntimeConflictGraph.end(); ++i)
    {
    // Sort the outgoing edges for each graph node so that the
    // original order will be preserved as much as possible.
    std::sort(i->begin(), i->end());

    // Make the edge list unique so cycle detection will be reliable.
    RuntimeConflictList::iterator last =
      std::unique(i->begin(), i->end(), cmOrderRuntimeDirectoriesCompare());
    i->erase(last, i->end());
    }
}

//----------------------------------------------------------------------------
void cmOrderRuntimeDirectories::FindDirectoriesForLib(unsigned int lri)
{
  // Search through the runtime directories to find those providing
  // this library.
  LibraryRuntimeEntry& re = this->LibraryRuntimeInfo[lri];
  for(unsigned int i = 0; i < this->RuntimeDirectories.size(); ++i)
    {
    // Skip the directory that is supposed to provide the library.
    if(this->RuntimeDirectories[i] == re.Directory)
      {
      continue;
      }

    // Determine which type of check to do.
    if(!re.SOName.empty())
      {
      // We have the library soname.  Check if it will be found.
      std::string file = this->RuntimeDirectories[i];
      file += "/";
      file += re.SOName;
      std::set<cmStdString> const& files =
        (this->GlobalGenerator
         ->GetDirectoryContent(this->RuntimeDirectories[i], false));
      if((std::set<cmStdString>::const_iterator(files.find(re.SOName)) !=
          files.end()) ||
         cmSystemTools::FileExists(file.c_str(), true))
        {
        // The library will be found in this directory but this is not
        // the directory named for it.  Add an entry to make sure the
        // desired directory comes before this one.
        RuntimeConflictPair p(re.DirectoryIndex, lri);
        this->RuntimeConflictGraph[i].push_back(p);
        }
      }
    else
      {
      // We do not have the soname.  Look for files in the directory
      // that may conflict.
      std::set<cmStdString> const& files =
        (this->GlobalGenerator
         ->GetDirectoryContent(this->RuntimeDirectories[i], true));

      // Get the set of files that might conflict.  Since we do not
      // know the soname just look at all files that start with the
      // file name.  Usually the soname starts with the library name.
      std::string base = re.FileName;
      std::set<cmStdString>::const_iterator first = files.lower_bound(base);
      ++base[base.size()-1];
      std::set<cmStdString>::const_iterator last = files.upper_bound(base);
      bool found = false;
      for(std::set<cmStdString>::const_iterator fi = first;
          !found && fi != last; ++fi)
        {
        found = true;
        }

      if(found)
        {
        // The library may be found in this directory but this is not
        // the directory named for it.  Add an entry to make sure the
        // desired directory comes before this one.
        RuntimeConflictPair p(re.DirectoryIndex, lri);
        this->RuntimeConflictGraph[i].push_back(p);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmOrderRuntimeDirectories::OrderRuntimeSearchPath()
{
  // Allow a cycle to be diagnosed once.
  this->CycleDiagnosed = false;
  this->WalkId = 0;

  // Iterate through the directories in the original order.
  for(unsigned int i=0; i < this->RuntimeDirectories.size(); ++i)
    {
    // Start a new DFS from this node.
    ++this->WalkId;
    this->VisitRuntimeDirectory(i);
    }
}

//----------------------------------------------------------------------------
void cmOrderRuntimeDirectories::VisitRuntimeDirectory(unsigned int i)
{
  // Skip nodes already visited.
  if(this->RuntimeDirectoryVisited[i])
    {
    if(this->RuntimeDirectoryVisited[i] == this->WalkId)
      {
      // We have reached a node previously visited on this DFS.
      // There is a cycle.
      this->DiagnoseCycle();
      }
    return;
    }

  // We are now visiting this node so mark it.
  this->RuntimeDirectoryVisited[i] = this->WalkId;

  // Visit the neighbors of the node first.
  RuntimeConflictList const& clist = this->RuntimeConflictGraph[i];
  for(RuntimeConflictList::const_iterator j = clist.begin();
      j != clist.end(); ++j)
    {
    this->VisitRuntimeDirectory(j->first);
    }

  // Now that all directories required to come before this one have
  // been emmitted, emit this directory.
  this->RuntimeSearchPath.push_back(this->RuntimeDirectories[i]);
}

//----------------------------------------------------------------------------
void cmOrderRuntimeDirectories::DiagnoseCycle()
{
  // Report the cycle at most once.
  if(this->CycleDiagnosed)
    {
    return;
    }
  this->CycleDiagnosed = true;

  // Construct the message.
  cmOStringStream e;
  e << "WARNING: Cannot generate a safe " << this->Purpose
    << " for target " << this->Name
    << " because there is a cycle in the constraint graph:\n";

  // Display the conflict graph.
  for(unsigned int i=0; i < this->RuntimeConflictGraph.size(); ++i)
    {
    RuntimeConflictList const& clist = this->RuntimeConflictGraph[i];
    e << "dir " << i << " is [" << this->RuntimeDirectories[i] << "]\n";
    for(RuntimeConflictList::const_iterator j = clist.begin();
        j != clist.end(); ++j)
      {
      e << "  dir " << j->first << " must precede it due to [";
      LibraryRuntimeEntry const& re = this->LibraryRuntimeInfo[j->second];
      if(re.SOName.empty())
        {
        e << re.FileName;
        }
      else
        {
        e << re.SOName;
        }
      e << "]\n";
      }
    }
  cmSystemTools::Message(e.str().c_str());
}
