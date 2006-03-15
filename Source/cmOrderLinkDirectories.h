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
#ifndef cmOrderLinkDirectories_h
#define cmOrderLinkDirectories_h

#include <cmStandardIncludes.h>
#include <map>
#include <vector>
#include "cmTarget.h"
#include "cmsys/RegularExpression.hxx"


/** \class cmOrderLinkDirectories
 * \brief Compute the best -L path order
 *
 * This class computes the best order for -L paths.
 * It tries to make sure full path specified libraries are 
 * used.  For example if you have /usr/mylib/libfoo.a on as
 * a link library for a target, and you also have /usr/lib/libbar.a
 * and you also have /usr/lib/libfoo.a, then you would
 * want -L/usr/mylib -L/usr/lib to make sure the correct libfoo.a is 
 * found by the linker.  The algorithm is as follows:
 * - foreach library create a vector of directories it exists in.
 * - foreach directory create a vector of directories that must come
 *   after it, put this in a map<dir, vector<dir>> mapping from a directory
 *   to the vector of directories that it must be before.
 * - put all directories into a vector
 * - sort the vector with a compare function CanBeBefore
 *   CanBeBefore returns true if a directory is OK to be before
 *   another directory.  This is determined by looking at the 
 *   map<dir vector<dir>> and seeing if d1 is in the vector for d2.
 */
class cmOrderLinkDirectories
{
public:
  cmOrderLinkDirectories();
  ///! set link information from the target
  void SetLinkInformation(const char* targetName,
                          const std::vector<std::string>& linkLibraries,
                          const std::vector<std::string>& linkDirectories);
  ///! Compute the best order for -L paths from GetLinkLibraries
  bool DetermineLibraryPathOrder();
  ///! Get the results from DetermineLibraryPathOrder
  void GetLinkerInformation(std::vector<cmStdString>& searchPaths,
                            std::vector<cmStdString>& linkItems)
  {
    linkItems = this->LinkItems;
    searchPaths = this->SortedSearchPaths;
  }
  // should be set from CMAKE_STATIC_LIBRARY_SUFFIX,
  // CMAKE_SHARED_LIBRARY_SUFFIX
  // CMAKE_LINK_LIBRARY_SUFFIX
  void AddLinkExtension(const char* e)
    {
    if(e && *e)
      {
      this->LinkExtensions.push_back(e);
      }
    }
  // should be set from CMAKE_STATIC_LIBRARY_PREFIX
  void SetLinkPrefix(const char* s)
    {
    if(s)
      {
      this->LinkPrefix = s;
      }
    }
  // Return any warnings if the exist
  std::string GetWarnings();
  // return a list of all full path libraries
  void GetFullPathLibraries(std::vector<cmStdString>& libs);

  // structure to hold a full path library link item
  struct Library
  {
    cmStdString FullPath;
    cmStdString File;
    cmStdString Path;
  };
  friend struct cmOrderLinkDirectoriesCompare;
  void DebugOn() 
    {
      this->Debug = true;
    }
  
private:
  void CreateRegularExpressions();
  void DetermineLibraryPathOrder(std::vector<cmStdString>& searchPaths,
                                 std::vector<cmStdString>& libs,
                                 std::vector<cmStdString>& sortedPaths);
  void PrepareLinkTargets();
  bool LibraryInDirectory(const char* dir, const char* lib);
  void FindLibrariesInSeachPaths();
  void FindIndividualLibraryOrders();
  void PrintMap(const char* name,
                std::map<cmStdString, std::vector<cmStdString> >& m);
  void OrderPaths(std::vector<cmStdString>& paths);
  bool FindPathNotInDirectoryToAfterList(cmStdString& path);
  std::string NoCaseExpression(const char* str);
private:
  // map from library to directories that it is in other than its full path
  std::map<cmStdString, std::vector<cmStdString> > LibraryToDirectories;
  // map from directory to vector of directories that must be after it
  std::map<cmStdString, std::vector<cmStdString> > DirectoryToAfterList;
  // map from full path to a Library struct
  std::map<cmStdString, Library> FullPathLibraries;
  // libraries that are found in multiple directories
  std::vector<Library> MultiDirectoryLibraries;
  // libraries that are only found in one directory
  std::vector<Library> SingleDirectoryLibraries;
  // This is a vector of all the link objects -lm or m
  std::vector<cmStdString> LinkItems;
  // Unprocessed link items
  std::vector<cmStdString> RawLinkItems;
  // This vector holds the sorted -L paths
  std::vector<cmStdString> SortedSearchPaths;
  // This vector holds the -F paths
  std::set<cmStdString> EmittedFrameworkPaths;
  // This is the set of -L paths unsorted, but unique
  std::set<cmStdString> LinkPathSet;
  // the names of link extensions
  std::vector<cmStdString> LinkExtensions;
  // the names of link prefixes
  cmStdString LinkPrefix;
  // set of directories that can not be put in the correct order
  std::set<cmStdString> ImpossibleDirectories;
  // Name of target
  cmStdString TargetName;
  // library regular expressions
  cmsys::RegularExpression RemoveLibraryExtension;
  cmsys::RegularExpression ExtractBaseLibraryName;
  cmsys::RegularExpression ExtractBaseLibraryNameNoPrefix;
  cmsys::RegularExpression SplitFramework;
  bool Debug;
};

#endif
