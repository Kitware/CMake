/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef cmMakeDepend_h
#define cmMakeDepend_h

#include "cmMakefile.h"
#include "cmClassFile.h"
#include "cmRegularExpression.h"
#include "cmStandardIncludes.h"

/** \class cmDependInformation
 * \brief Store dependency information for a single source file.
 *
 * This structure stores the depend information for a single source file.
 */
struct cmDependInformation
{
  /**
   * Construct with dependency generation marked not done; instance
   * not placed in cmMakefile's list.
   */
  cmDependInformation() 
    {
    m_DependDone = false;
    m_ClassFileIndex = 0;
    }

  /**
   * A set of indices into the m_DependInformation array of cmMakeDepend.
   * The index represents the files that this file depends on.
   * This must be a "set" to keep indices unique.
   */
  typedef std::set<int> IndexSet;
  IndexSet m_IndexSet;	

  /**
   * Full path to this file.
   */
  std::string m_FullPath;	

  /**
   * Name that the include directive uses.
   */
  std::string m_IncludeName;

  /**
   * The index into the cmMakefile::m_Classes list.
   * The index value of 0 indicates that it is not in the list.
   */
  cmClassFile *m_ClassFileIndex;	
  
  /**
   * This flag indicates whether dependency checking has been
   * performed for this file.
   */
  bool m_DependDone;
  
  /**
   * This method adds the dependencies of another file to this one.
   */
  void MergeInfo(cmDependInformation*);
};


// cmMakeDepend is used to generate dependancy information for
// the classes in a makefile
class cmMakeDepend
{
public:
  /**
   * Construct the object with verbose turned off.
   */
  cmMakeDepend();

  /**
   * Destructor.
   */
  ~cmMakeDepend();
  
  /** 
   * Set the makefile that is used as a source of classes.
   */
  void SetMakefile(cmMakefile* makefile); 

  /** 
   * Generate the depend information
   */
  void DoDepends();

  /**
   * Add a directory to the search path for include files.
   */
  void AddSearchPath(const char*);

private: 
  /**
   * Add a source file to the search path.
   */
  void AddFileToSearchPath(const char* filepath);

  /**
   * Find the index into the m_DependInformation array
   * that matches the given m_IncludeName.
   */
  int FindInformation(const char* includeName);

  /**
   * Compute the depend information for this class.
   */
  void Depend(cmDependInformation* info);

  /**
   * Compute the depend information for this class.
   */
  void DependWalk(cmDependInformation* info, const char* file);
  
  /**
   * Add a dependency.  Possibly walk it for more dependencies.
   */
  void AddDependency(cmDependInformation* info, const char* file);

  /** 
   * Find the full path name for the given file name.
   * This uses the include directories.
   */
  std::string FullPath(const char*);

  cmMakefile* m_Makefile;
  bool m_Verbose;
  cmRegularExpression m_IncludeFileRegularExpression;
  typedef std::vector<cmDependInformation*> DependArray;
  DependArray m_DependInformation;
  std::vector<std::string> m_IncludeDirectories;
};

#endif
