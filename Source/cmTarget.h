/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmTarget_h
#define cmTarget_h

#include "cmStandardIncludes.h"
#include "cmCustomCommand.h"
#include "cmSourceFile.h"

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from 
 * a makefile.
 */
class cmTarget
{
public:
  enum TargetType { EXECUTABLE, WIN32_EXECUTABLE, STATIC_LIBRARY,
                    SHARED_LIBRARY, MODULE_LIBRARY, UTILITY, INSTALL_FILES, 
                    INSTALL_PROGRAMS };

  /**
   * Return the type of target.
   */
  TargetType GetType() const
    {
      return m_TargetType;
    }
  
  void SetType(TargetType f) { m_TargetType = f; }
  
  /**
   * Indicate whether the target is part of the all target
   */
  bool IsInAll() const { return m_InAll; }
  bool GetInAll() const { return m_InAll; }
  void SetInAll(bool f) { m_InAll = f; }

  /**
   * Get the list of the custom commands for this target
   */
  const std::vector<cmCustomCommand> &GetCustomCommands() const {return m_CustomCommands;}
  std::vector<cmCustomCommand> &GetCustomCommands() {return m_CustomCommands;}

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

  ///! does this target have a cxx file in it
  bool HasCxx() const;
  /**
   * Get the list of the source files used by this target
   */
  enum LinkLibraryType {GENERAL, DEBUG, OPTIMIZED};
  typedef std::vector<std::pair<std::string,LinkLibraryType> > LinkLibraries;
  const LinkLibraries &GetLinkLibraries() const {return m_LinkLibraries;}

  const std::vector<std::string>& GetLinkDirectories() const {return m_LinkDirectories;}

  /**
   * Set the path where this target should be installed. This is relative to
   * INSTALL_PREFIX
   */
  std::string GetInstallPath() const {return m_InstallPath;}
  void SetInstallPath(const char *name) {m_InstallPath = name;}
  
  void AddLinkLibrary(cmMakefile& mf,
                      const char *target, const char* lib, 
                      LinkLibraryType llt);

  void AddLinkLibrary(const std::string& lib, 
                      LinkLibraryType llt);

  /**
   * Merge Link Libraries into this targets current list 
   */
  void MergeLibraries(const LinkLibraries &ll);

  /**
   * Merge Link Directories into this targets current list 
   */
  void MergeDirectories(const std::vector<std::string> &ld);
    
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
  std::set<std::string>const& GetUtilities() const { return m_Utilities; }

  void AnalyzeLibDependencies( const cmMakefile& mf );

private:
  /**
   * This map holds the dependency graph. map[x] returns a set of
   * direct dependencies of x.
   */
  typedef std::map< std::string, std::set< std::string > > DependencyMap;

  /**
   * For each library in the link line, return a canonical name. The
   * orginal library names have complicated forms, such as "x",
   * "libx.so", "/full/path/libx.a", "-lx", and "-framework x".
   */
  std::string CanonicalLibraryName( const std::string& lib ) const;

  /**
   * Emits the library \param lib and all its dependencies into
   * link_line.  \param emitted keeps track of the libraries that have
   * been emitted to avoid duplicates--it is more efficient than
   * searching link_line. \param visited is used detect cycles. Note
   * that \param link_line is in reverse order, in that the
   * dependencies of a library are listed before the library itself.
   */
  void Emit( const std::string& lib,
             const DependencyMap& dep_map,
             std::set<std::string>& emitted,
             std::set<std::string>& visited,
             std::vector<std::string>& link_line ) const;

  /**
   * Finds the explicit dependencies for \param lib, if they have been
   * specified, and inserts them into \param dep_map.
   */
  void GatherDependencies( const cmMakefile& mf, const std::string& lib,
                           DependencyMap& dep_map ) const;

  /**
   * Returns true if lib1 depends on lib2 according to \param
   * dep_map. \param visited is used to prevent infinite loops when
   * cycles are present.
   */
  bool DependsOn( const std::string& lib1, const std::string& lib2,
                  const DependencyMap& dep_map,
                  std::set<std::string>& visited ) const;

private:
  std::vector<cmCustomCommand> m_CustomCommands;
  std::vector<std::string> m_SourceLists;
  TargetType m_TargetType;
  std::vector<cmSourceFile*> m_SourceFiles;
  LinkLibraries m_LinkLibraries;
  std::vector<std::string> m_LinkDirectories;
  bool m_InAll;
  std::string m_InstallPath;
  std::set<std::string> m_Utilities;
};

typedef std::map<cmStdString,cmTarget> cmTargets;

#endif
