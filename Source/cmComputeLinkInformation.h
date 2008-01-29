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
#ifndef cmComputeLinkInformation_h
#define cmComputeLinkInformation_h

#include "cmStandardIncludes.h"

#include <cmsys/RegularExpression.hxx>

class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmTarget;

/** \class cmComputeLinkInformation
 * \brief Compute link information for a target in one configuration.
 */
class cmComputeLinkInformation
{
public:
  cmComputeLinkInformation(cmTarget* target, const char* config);
  bool Compute();

  struct Item
  {
    Item(): Value(), IsPath(true) {}
    Item(Item const& item): Value(item.Value), IsPath(item.IsPath) {}
    Item(std::string const& v, bool p): Value(v), IsPath(p) {}
    std::string Value;
    bool IsPath;
  };
  typedef std::vector<Item> ItemVector;
  ItemVector const& GetItems();
  std::vector<std::string> const& GetDirectories();
  std::vector<std::string> const& GetDepends();
  std::vector<std::string> const& GetFrameworkPaths();
  const char* GetLinkLanguage() const { return this->LinkLanguage; }
  std::vector<std::string> const& GetRuntimeSearchPath();
  std::string const& GetRuntimeFlag() const { return this->RuntimeFlag; }
  std::string const& GetRuntimeSep() const { return this->RuntimeSep; }
  void GetRPath(std::vector<std::string>& runtimeDirs, bool for_install);
  std::string GetRPathString(bool for_install);
  std::string GetChrpathString();
  std::string GetChrpathTool();
private:
  void AddItem(std::string const& item, cmTarget* tgt);

  // Output information.
  ItemVector Items;
  std::vector<std::string> Directories;
  std::vector<std::string> Depends;
  std::vector<std::string> FrameworkPaths;
  std::vector<std::string> RuntimeSearchPath;

  // Context information.
  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator* GlobalGenerator;

  // Configuration information.
  const char* Config;
  const char* LinkLanguage;

  // System info.
  bool UseImportLibrary;
  const char* LoaderFlag;
  std::string LibLinkFlag;
  std::string LibLinkFileFlag;
  std::string LibLinkSuffix;
  std::string RuntimeFlag;
  std::string RuntimeSep;
  std::string RuntimeAlways;
  bool RuntimeUseChrpath;

  // Link type adjustment.
  void ComputeLinkTypeInfo();
  enum LinkType { LinkUnknown, LinkStatic, LinkShared };
  LinkType StartLinkType;
  LinkType CurrentLinkType;
  std::string StaticLinkTypeFlag;
  std::string SharedLinkTypeFlag;
  bool LinkTypeEnabled;
  void SetCurrentLinkType(LinkType lt);

  // Link item parsing.
  void ComputeItemParserInfo();
  std::vector<std::string> StaticLinkExtensions;
  std::vector<std::string> SharedLinkExtensions;
  std::vector<std::string> LinkExtensions;
  std::set<cmStdString> LinkPrefixes;
  cmsys::RegularExpression RemoveLibraryExtension;
  cmsys::RegularExpression ExtractStaticLibraryName;
  cmsys::RegularExpression ExtractSharedLibraryName;
  cmsys::RegularExpression ExtractAnyLibraryName;
  void AddLinkPrefix(const char* p);
  void AddLinkExtension(const char* e, LinkType type);
  std::string CreateExtensionRegex(std::vector<std::string> const& exts);
  std::string NoCaseExpression(const char* str);

  // Handling of link items that are not targets or full file paths.
  void AddTargetItem(std::string const& item, cmTarget* target);
  void AddFullItem(std::string const& item);
  void AddUserItem(std::string const& item);
  void AddDirectoryItem(std::string const& item);
  void AddFrameworkItem(std::string const& item);
  void DropDirectoryItem(std::string const& item);

  // Framework info.
  void ComputeFrameworkInfo();
  void AddFrameworkPath(std::string const& p);
  std::set<cmStdString> FrameworkPathsEmmitted;
  cmsys::RegularExpression SplitFramework;

  // Linker search path computation.
  void ComputeLinkerSearchDirectories();
  void AddLinkerSearchDirectories(std::vector<std::string> const& dirs);
  std::set<cmStdString> DirectoriesEmmitted;

  // Linker search path compatibility mode.
  std::vector<std::string> OldLinkDirs;
  bool OldLinkDirMode;
  bool HaveUserFlagItem;

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
  std::set<cmStdString> LibraryRuntimeInfoEmmitted;
  std::vector<std::string> RuntimeDirectories;
  std::map<cmStdString, int> RuntimeDirectoryIndex;
  std::vector<int> RuntimeDirectoryVisited;
  void AddLibraryRuntimeInfo(std::string const& fullPath, cmTarget* target);
  void AddLibraryRuntimeInfo(std::string const& fullPath,
                             const char* soname = 0);
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
