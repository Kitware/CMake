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
#include "cmOrderDirectories.h"

#include "cmGlobalGenerator.h"
#include "cmSystemTools.h"

#include <assert.h>

#include <algorithm>

/*
Directory ordering computation.
  - Useful to compute a safe runtime library path order
  - Need runtime path for supporting INSTALL_RPATH_USE_LINK_PATH
  - Need runtime path at link time to pickup transitive link dependencies
    for shared libraries.
*/

//----------------------------------------------------------------------------
class cmOrderDirectoriesConstraint
{
public:
  cmOrderDirectoriesConstraint(cmOrderDirectories* od,
                               std::string const& file):
    OD(od), GlobalGenerator(od->GlobalGenerator)
    {
    this->FullPath = file;
    this->Directory = cmSystemTools::GetFilenamePath(file);
    this->FileName = cmSystemTools::GetFilenameName(file);
    }
  virtual ~cmOrderDirectoriesConstraint() {}

  void AddDirectory()
    {
    this->DirectoryIndex = this->OD->AddOriginalDirectory(this->Directory);
    }

  virtual void Report(std::ostream& e) = 0;

  void FindConflicts(unsigned int index)
    {
    for(unsigned int i=0; i < this->OD->OriginalDirectories.size(); ++i)
      {
      // Check if this directory conflicts with they entry.
      std::string const& dir = this->OD->OriginalDirectories[i];
      if(dir != this->Directory && this->FindConflict(dir))
        {
        // The library will be found in this directory but this is not
        // the directory named for it.  Add an entry to make sure the
        // desired directory comes before this one.
        cmOrderDirectories::ConflictPair p(this->DirectoryIndex, index);
        this->OD->ConflictGraph[i].push_back(p);
        }
      }
    }
protected:
  virtual bool FindConflict(std::string const& dir) = 0;

  bool FileMayConflict(std::string const& dir, std::string const& name);

  cmOrderDirectories* OD;
  cmGlobalGenerator* GlobalGenerator;

  // The location in which the item is supposed to be found.
  std::string FullPath;
  std::string Directory;
  std::string FileName;

  // The index assigned to the directory.
  int DirectoryIndex;
};

//----------------------------------------------------------------------------
bool cmOrderDirectoriesConstraint::FileMayConflict(std::string const& dir,
                                                   std::string const& name)
{
  // Check if the file will be built by cmake.
  std::set<cmStdString> const& files =
    (this->GlobalGenerator->GetDirectoryContent(dir, false));
  if(std::set<cmStdString>::const_iterator(files.find(name)) != files.end())
    {
    return true;
    }

  // Check if the file exists on disk and is not a symlink back to the
  // original file.
  std::string file = dir;
  file += "/";
  file += name;
  if(cmSystemTools::FileExists(file.c_str(), true) &&
     !cmSystemTools::SameFile(this->FullPath.c_str(), file.c_str()))
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
class cmOrderDirectoriesConstraintSOName: public cmOrderDirectoriesConstraint
{
public:
  cmOrderDirectoriesConstraintSOName(cmOrderDirectories* od,
                                     std::string const& file,
                                     const char* soname):
    cmOrderDirectoriesConstraint(od, file), SOName(soname? soname : "")
    {
    }

  virtual void Report(std::ostream& e)
    {
    e << "runtime library [";
    if(this->SOName.empty())
      {
      e << this->FileName;
      }
    else
      {
      e << this->SOName;
      }
    e << "]";
    }

  virtual bool FindConflict(std::string const& dir);
private:
  // The soname of the shared library if it is known.
  std::string SOName;
};

//----------------------------------------------------------------------------
bool cmOrderDirectoriesConstraintSOName::FindConflict(std::string const& dir)
{
  // Determine which type of check to do.
  if(!this->SOName.empty())
    {
    // We have the library soname.  Check if it will be found.
    if(this->FileMayConflict(dir, this->SOName))
      {
      return true;
      }
    }
  else
    {
    // We do not have the soname.  Look for files in the directory
    // that may conflict.
    std::set<cmStdString> const& files =
      (this->GlobalGenerator
       ->GetDirectoryContent(dir, true));

    // Get the set of files that might conflict.  Since we do not
    // know the soname just look at all files that start with the
    // file name.  Usually the soname starts with the library name.
    // TODO: Check if the library is a symlink and guess the soname.
    std::string base = this->FileName;
    std::set<cmStdString>::const_iterator first = files.lower_bound(base);
    ++base[base.size()-1];
    std::set<cmStdString>::const_iterator last = files.upper_bound(base);
    bool found = false;
    for(std::set<cmStdString>::const_iterator fi = first; fi != last; ++fi)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
class cmOrderDirectoriesConstraintLibrary: public cmOrderDirectoriesConstraint
{
public:
  cmOrderDirectoriesConstraintLibrary(cmOrderDirectories* od,
                                      std::string const& file):
    cmOrderDirectoriesConstraint(od, file)
    {
    }

  virtual void Report(std::ostream& e)
    {
    e << "link library [" << this->FileName << "]";
    }

  virtual bool FindConflict(std::string const& dir);
};

//----------------------------------------------------------------------------
bool cmOrderDirectoriesConstraintLibrary::FindConflict(std::string const& dir)
{
  // We have the library file name.  Check if it will be found.
  if(this->FileMayConflict(dir, this->FileName))
    {
    return true;
    }

  // Now check if the file exists with other extensions the linker
  // might consider.
  if(!this->OD->LinkExtensions.empty() &&
     this->OD->RemoveLibraryExtension.find(this->FileName))
    {
    cmStdString lib = this->OD->RemoveLibraryExtension.match(1);
    cmStdString ext = this->OD->RemoveLibraryExtension.match(2);
    for(std::vector<std::string>::iterator
          i = this->OD->LinkExtensions.begin();
        i != this->OD->LinkExtensions.end(); ++i)
      {
      if(*i != ext)
        {
        std::string fname = lib;
        fname += *i;
        if(this->FileMayConflict(dir, fname.c_str()))
          {
          return true;
          }
        }
      }
    }
  return false;
}

//----------------------------------------------------------------------------
cmOrderDirectories::cmOrderDirectories(cmGlobalGenerator* gg,
                                       const char* name,
                                       const char* purpose)
{
  this->GlobalGenerator = gg;
  this->Name = name;
  this->Purpose = purpose;
  this->Computed = false;
}

//----------------------------------------------------------------------------
cmOrderDirectories::~cmOrderDirectories()
{
  for(std::vector<cmOrderDirectoriesConstraint*>::iterator
        i = this->ConstraintEntries.begin();
      i != this->ConstraintEntries.end(); ++i)
    {
    delete *i;
    }
}

//----------------------------------------------------------------------------
std::vector<std::string> const& cmOrderDirectories::GetOrderedDirectories()
{
  if(!this->Computed)
    {
    this->Computed = true;
    this->CollectOriginalDirectories();
    this->FindConflicts();
    this->OrderDirectories();
    }
  return this->OrderedDirectories;
}

//----------------------------------------------------------------------------
void cmOrderDirectories::AddRuntimeLibrary(std::string const& fullPath,
                                           const char* soname)
{
  // Add the runtime library at most once.
  if(this->EmmittedConstraintSOName.insert(fullPath).second)
    {
    // Avoid dealing with implicit directories.
    if(!this->ImplicitDirectories.empty())
      {
      std::string dir = cmSystemTools::GetFilenamePath(fullPath);
      if(this->ImplicitDirectories.find(dir) !=
         this->ImplicitDirectories.end())
        {
        return;
        }
      }

    // Construct the runtime information entry for this library.
    this->ConstraintEntries.push_back(
      new cmOrderDirectoriesConstraintSOName(this, fullPath, soname));
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
void cmOrderDirectories::AddLinkLibrary(std::string const& fullPath)
{
  // Link extension info is required for library constraints.
  assert(!this->LinkExtensions.empty());

  // Add the link library at most once.
  if(this->EmmittedConstraintLibrary.insert(fullPath).second)
    {
    // Avoid dealing with implicit directories.
    if(!this->ImplicitDirectories.empty())
      {
      std::string dir = cmSystemTools::GetFilenamePath(fullPath);
      if(this->ImplicitDirectories.find(dir) !=
         this->ImplicitDirectories.end())
        {
        return;
        }
      }

    // Construct the link library entry.
    this->ConstraintEntries.push_back(
      new cmOrderDirectoriesConstraintLibrary(this, fullPath));
    }
}

//----------------------------------------------------------------------------
void
cmOrderDirectories
::AddUserDirectories(std::vector<std::string> const& extra)
{
  this->UserDirectories.insert(this->UserDirectories.end(),
                               extra.begin(), extra.end());
}

//----------------------------------------------------------------------------
void
cmOrderDirectories
::SetImplicitDirectories(std::set<cmStdString> const& implicitDirs)
{
  this->ImplicitDirectories = implicitDirs;
}

//----------------------------------------------------------------------------
void
cmOrderDirectories
::SetLinkExtensionInfo(std::vector<std::string> const& linkExtensions,
                       std::string const& removeExtRegex)
{
  this->LinkExtensions = linkExtensions;
  this->RemoveLibraryExtension.compile(removeExtRegex.c_str());
}

//----------------------------------------------------------------------------
void cmOrderDirectories::CollectOriginalDirectories()
{
  // Add user directories specified for inclusion.  These should be
  // indexed first so their original order is preserved as much as
  // possible subject to the constraints.
  for(std::vector<std::string>::const_iterator
        di = this->UserDirectories.begin();
      di != this->UserDirectories.end(); ++di)
    {
    // Avoid dealing with implicit directories.
    if(this->ImplicitDirectories.find(*di) !=
       this->ImplicitDirectories.end())
      {
      continue;
      }

    // Skip the empty string.
    if(di->empty())
      {
      continue;
      }

    // Add this directory.
    this->AddOriginalDirectory(*di);
    }

  // Add directories containing constraints.
  for(unsigned int i=0; i < this->ConstraintEntries.size(); ++i)
    {
    this->ConstraintEntries[i]->AddDirectory();
    }
}

//----------------------------------------------------------------------------
int cmOrderDirectories::AddOriginalDirectory(std::string const& dir)
{
  // Add the runtime directory with a unique index.
  std::map<cmStdString, int>::iterator i =
    this->DirectoryIndex.find(dir);
  if(i == this->DirectoryIndex.end())
    {
    std::map<cmStdString, int>::value_type
      entry(dir, static_cast<int>(this->OriginalDirectories.size()));
    i = this->DirectoryIndex.insert(entry).first;
    this->OriginalDirectories.push_back(dir);
    }

  return i->second;
}

//----------------------------------------------------------------------------
struct cmOrderDirectoriesCompare
{
  typedef std::pair<int, int> ConflictPair;

  // The conflict pair is unique based on just the directory
  // (first).  The second element is only used for displaying
  // information about why the entry is present.
  bool operator()(ConflictPair const& l,
                  ConflictPair const& r)
    {
    return l.first == r.first;
    }
};

//----------------------------------------------------------------------------
void cmOrderDirectories::FindConflicts()
{
  // Allocate the conflict graph.
  this->ConflictGraph.resize(this->OriginalDirectories.size());
  this->DirectoryVisited.resize(this->OriginalDirectories.size(), 0);

  // Find directories conflicting with each entry.
  for(unsigned int i=0; i < this->ConstraintEntries.size(); ++i)
    {
    this->ConstraintEntries[i]->FindConflicts(i);
    }

  // Clean up the conflict graph representation.
  for(std::vector<ConflictList>::iterator
        i = this->ConflictGraph.begin();
      i != this->ConflictGraph.end(); ++i)
    {
    // Sort the outgoing edges for each graph node so that the
    // original order will be preserved as much as possible.
    std::sort(i->begin(), i->end());

    // Make the edge list unique so cycle detection will be reliable.
    ConflictList::iterator last =
      std::unique(i->begin(), i->end(), cmOrderDirectoriesCompare());
    i->erase(last, i->end());
    }
}

//----------------------------------------------------------------------------
void cmOrderDirectories::OrderDirectories()
{
  // Allow a cycle to be diagnosed once.
  this->CycleDiagnosed = false;
  this->WalkId = 0;

  // Iterate through the directories in the original order.
  for(unsigned int i=0; i < this->OriginalDirectories.size(); ++i)
    {
    // Start a new DFS from this node.
    ++this->WalkId;
    this->VisitDirectory(i);
    }
}

//----------------------------------------------------------------------------
void cmOrderDirectories::VisitDirectory(unsigned int i)
{
  // Skip nodes already visited.
  if(this->DirectoryVisited[i])
    {
    if(this->DirectoryVisited[i] == this->WalkId)
      {
      // We have reached a node previously visited on this DFS.
      // There is a cycle.
      this->DiagnoseCycle();
      }
    return;
    }

  // We are now visiting this node so mark it.
  this->DirectoryVisited[i] = this->WalkId;

  // Visit the neighbors of the node first.
  ConflictList const& clist = this->ConflictGraph[i];
  for(ConflictList::const_iterator j = clist.begin();
      j != clist.end(); ++j)
    {
    this->VisitDirectory(j->first);
    }

  // Now that all directories required to come before this one have
  // been emmitted, emit this directory.
  this->OrderedDirectories.push_back(this->OriginalDirectories[i]);
}

//----------------------------------------------------------------------------
void cmOrderDirectories::DiagnoseCycle()
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
  for(unsigned int i=0; i < this->ConflictGraph.size(); ++i)
    {
    ConflictList const& clist = this->ConflictGraph[i];
    e << "dir " << i << " is [" << this->OriginalDirectories[i] << "]\n";
    for(ConflictList::const_iterator j = clist.begin();
        j != clist.end(); ++j)
      {
      e << "  dir " << j->first << " must precede it due to ";
      this->ConstraintEntries[j->second]->Report(e);
      e << "\n";
      }
    }
  cmSystemTools::Message(e.str().c_str());
}
