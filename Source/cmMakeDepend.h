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
#ifndef cmMakeDepend_h
#define cmMakeDepend_h

#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmRegularExpression.h"
#include "cmStandardIncludes.h"

/** \class cmDependInformation
 * \brief Store dependency information for a single source file.
 *
 * This structure stores the depend information for a single source file.
 */
class cmDependInformation
{
public:
  /**
   * Construct with dependency generation marked not done; instance
   * not placed in cmMakefile's list.
   */
  cmDependInformation(): m_DependDone(false), m_cmSourceFile(0) {}

  /**
   * The set of files on which this one depends.
   */
  typedef std::set<cmDependInformation*> DependencySet;
  DependencySet m_DependencySet;

  /**
   * This flag indicates whether dependency checking has been
   * performed for this file.
   */
  bool m_DependDone;

  /**
   * If this object corresponds to a cmSourceFile instance, this points
   * to it.
   */
  const cmSourceFile *m_cmSourceFile;
  
  /**
   * Full path to this file.
   */
  std::string m_FullPath;
  
  /**
   * Name used to #include this file.
   */
  std::string m_IncludeName;
  
  /**
   * This method adds the dependencies of another file to this one.
   */
  void AddDependencies(cmDependInformation*);  
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
  virtual ~cmMakeDepend();
  
  /** 
   * Set the makefile that is used as a source of classes.
   */
  virtual void SetMakefile(const cmMakefile* makefile); 

  /** 
   * Get the depend info struct for a source file
   */
  const cmDependInformation *GetDependInformationForSourceFile(const cmSourceFile &sf) const;

  /**
   * Add a directory to the search path for include files.
   */
  virtual void AddSearchPath(const char*);

  /**
   * Generate dependencies for all the sources of all the targets
   * in the makefile.
   */
  void GenerateMakefileDependencies();

  /**
   * Generate dependencies for the file given.  Returns a pointer to
   * the cmDependInformation object for the file.
   */
  const cmDependInformation* FindDependencies(const char* file);

protected: 
  /**
   * Add a source file to the search path.
   */
  void AddFileToSearchPath(const char* filepath);

  /**
   * Compute the depend information for this class.
   */
  virtual void DependWalk(cmDependInformation* info);
  
  /**
   * Add a dependency.  Possibly walk it for more dependencies.
   */
  virtual void AddDependency(cmDependInformation* info, const char* file);
  
  /**
   * Fill in the given object with dependency information.  If the
   * information is already complete, nothing is done.
   */
  void GenerateDependInformation(cmDependInformation* info);
  
  /**
   * Get an instance of cmDependInformation corresponding to the given file
   * name.
   */
  cmDependInformation* GetDependInformation(const char* file, const char *extraPath);  
  
  /** 
   * Find the full path name for the given file name.
   * This uses the include directories.
   * TODO: Cache path conversions to reduce FileExists calls.
   */
  std::string FullPath(const char *filename, const char *extraPath);

  const cmMakefile* m_Makefile;
  bool m_Verbose;
  cmRegularExpression m_IncludeFileRegularExpression;
  cmRegularExpression m_ComplainFileRegularExpression;
  std::vector<std::string> m_IncludeDirectories;
  typedef std::map<cmStdString, cmDependInformation*> DependInformationMap;
  DependInformationMap m_DependInformationMap;
};

#endif
