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
#ifndef cmTarget_h
#define cmTarget_h

#include "cmCustomCommand.h"
class cmSourceFile;
class cmGlobalGenerator;

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from 
 * a makefile.
 */
class cmTarget
{
public:
  enum TargetType { EXECUTABLE, STATIC_LIBRARY,
                    SHARED_LIBRARY, MODULE_LIBRARY, UTILITY, INSTALL_FILES, 
                    INSTALL_PROGRAMS };

  enum CustomCommandType { PRE_BUILD, PRE_LINK, POST_BUILD };

  /**
   * Return the type of target.
   */
  TargetType GetType() const
    {
      return m_TargetType;
    }
  
  /**
   * Set the target type
   */
  void SetType(TargetType f, const char* name);

  ///! Set/Get the name of the target
  const char* GetName() const {return m_Name.c_str();}

  /**
   * Indicate whether the target is part of the all target
   */
  bool IsInAll() const { return this->GetPropertyAsBool("IN_ALL"); }
  bool GetInAll() const { return this->GetPropertyAsBool("IN_ALL"); }
  void SetInAll(bool f) { this->SetProperty("IN_ALL", (f) ?  "TRUE" : "FALSE"); }

  /**
   * Get the list of the custom commands for this target
   */
  const std::vector<cmCustomCommand> &GetPreBuildCommands() const 
    {return m_PreBuildCommands;}
  std::vector<cmCustomCommand> &GetPreBuildCommands() 
    {return m_PreBuildCommands;}
  const std::vector<cmCustomCommand> &GetPreLinkCommands() const 
    {return m_PreLinkCommands;}
  std::vector<cmCustomCommand> &GetPreLinkCommands() 
    {return m_PreLinkCommands;}
  const std::vector<cmCustomCommand> &GetPostBuildCommands() const 
    {return m_PostBuildCommands;}
  std::vector<cmCustomCommand> &GetPostBuildCommands() 
    {return m_PostBuildCommands;}

  /**
   * Get the list of the source lists used by this target
   */
  const std::vector<std::string> &GetSourceLists() const 
    {return m_SourceLists;}
  std::vector<std::string> &GetSourceLists() {return m_SourceLists;}
  
  /**
   * Get the list of the source files used by this target
   */
  const std::vector<cmSourceFile*> &GetSourceFiles() const 
    {return m_SourceFiles;}
  std::vector<cmSourceFile*> &GetSourceFiles() {return m_SourceFiles;}

  /**
   * Get the list of the source files used by this target
   */
  enum LinkLibraryType {GENERAL, DEBUG, OPTIMIZED};
  typedef std::vector<std::pair<std::string,LinkLibraryType> > LinkLibraries;
  const LinkLibraries &GetLinkLibraries() const {return m_LinkLibraries;}

  /**
   * Clear the dependency information recorded for this target, if any.
   */
  void ClearDependencyInformation(cmMakefile& mf, const char* target);

  void AddLinkLibrary(cmMakefile& mf,
                      const char *target, const char* lib, 
                      LinkLibraryType llt);

  void AddLinkLibrary(const std::string& lib, 
                      LinkLibraryType llt);

  void MergeLinkLibraries( cmMakefile& mf, const char* selfname, const LinkLibraries& libs );

  const std::vector<std::string>& GetLinkDirectories() const {return m_LinkDirectories;}
  
  void AddLinkDirectory(const char* d);

  /**
   * Set the path where this target should be installed. This is relative to
   * INSTALL_PREFIX
   */
  std::string GetInstallPath() const {return m_InstallPath;}
  void SetInstallPath(const char *name) {m_InstallPath = name;}
  
  /**
   * Set the path where this target (if it has a runtime part) should be
   * installed. This is relative to INSTALL_PREFIX
   */
  std::string GetRuntimeInstallPath() const {return m_RuntimeInstallPath;}
  void SetRuntimeInstallPath(const char *name) {m_RuntimeInstallPath = name;}
  
  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  
   */
  void GenerateSourceFilesFromSourceLists(cmMakefile &mf);

  /** Add a utility on which this project depends. A utility is an executable
   * name as would be specified to the ADD_EXECUTABLE or UTILITY_SOURCE
   * commands. It is not a full path nor does it have an extension.  
   */
  void AddUtility(const char* u) { m_Utilities.insert(u);}
  ///! Get the utilities used by this target
  std::set<cmStdString>const& GetUtilities() const { return m_Utilities; }

  void AnalyzeLibDependencies( const cmMakefile& mf );

  ///! Set/Get a property of this target file
  void SetProperty(const char *prop, const char *value);
  const char *GetProperty(const char *prop) const;
  bool GetPropertyAsBool(const char *prop) const;

  /**
   * Trace through the source files in this target and add al source files
   * that they depend on, used by the visual studio generators
   */
  void TraceVSDependencies(std::string projName, cmMakefile *mf);  

  ///! Return the prefered linker language for this target
  const char* GetLinkerLanguage(cmGlobalGenerator*) const;
  
  ///! Return the rule variable used to create this type of target, 
  //  need to add CMAKE_(LANG) for full name.
  const char* GetCreateRuleVariable();
  ///! Return the name of the variable to look up the target suffix
  const char* GetSuffixVariable() const;
  ///! Return the name of the variable to look up the target suffix
  const char* GetPrefixVariable() const;
private:
  /**
   * A list of direct dependencies. Use in conjunction with DependencyMap.
   */
  typedef std::vector<cmStdString> DependencyList;

  /**
   * This map holds the dependency graph. map[x] returns a set of
   * direct dependencies of x. Note that the direct depenencies are
   * ordered. This is necessary to handle direct dependencies that
   * themselves have no dependency information.
   */
  typedef std::map< cmStdString, std::vector< cmStdString > > DependencyMap;

  /**
   * Maps a library name to its internal structure
   */
  typedef std::map< cmStdString, std::pair<cmStdString,LinkLibraryType> > LibTypeMap;

  /**
   * Inserts \a dep at the end of the dependency list of \a lib.
   */
  void InsertDependency( DependencyMap& depMap,
                         const cmStdString& lib,
                         const cmStdString& dep ) const;

  /*
   * Deletes \a dep from the dependency list of \a lib.
   */
  void DeleteDependency( DependencyMap& depMap,
                         const cmStdString& lib,
                         const cmStdString& dep ) const;

  /**
   * Emits the library \a lib and all its dependencies into link_line.
   * \a emitted keeps track of the libraries that have been emitted to
   * avoid duplicates--it is more efficient than searching
   * link_line. \a visited is used detect cycles. Note that \a
   * link_line is in reverse order, in that the dependencies of a
   * library are listed before the library itself.
   */
  void Emit( const std::string& lib,
             const DependencyMap& dep_map,
             std::set<cmStdString>& emitted,
             std::set<cmStdString>& visited,
             std::vector<std::string>& link_line ) const;

  /**
   * Finds the dependencies for \a lib and inserts them into \a
   * dep_map.
   */
  void GatherDependencies( const cmMakefile& mf, const std::string& lib,
                           DependencyMap& dep_map ); 

  
private:
  std::string m_Name;
  std::vector<cmCustomCommand> m_PreBuildCommands;
  std::vector<cmCustomCommand> m_PreLinkCommands;
  std::vector<cmCustomCommand> m_PostBuildCommands;
  std::vector<std::string> m_SourceLists;
  TargetType m_TargetType;
  std::vector<cmSourceFile*> m_SourceFiles;
  LinkLibraries m_LinkLibraries;
  LinkLibraries m_PrevLinkedLibraries;
  std::vector<std::string> m_LinkDirectories;
  std::string m_InstallPath;
  std::string m_RuntimeInstallPath;
  std::set<cmStdString> m_Utilities;
  bool m_RecordDependencies; 
  std::map<cmStdString,cmStdString> m_Properties;
};

typedef std::map<cmStdString,cmTarget> cmTargets;

#endif
