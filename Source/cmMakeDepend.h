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
/**
 * cmMakeDepend 
 */
#ifndef cmMakeDepend_h
#define cmMakeDepend_h

#include "cmMakefile.h"
#include "cmClassFile.h"
#include "cmRegularExpression.h"
#include <vector>
#include <string>


// This structure stores the depend information 
// for a single source file
struct cmDependInformation
{
  cmDependInformation() 
    {
      m_DependDone = false;
      m_ClassFileIndex = -1;
    }
// index into m_DependInformation array of cmMakeDepend 
// class, represents the files that this file depends on
  std::vector<int> m_Indices;	

// full path to file
  std::string m_FullPath;	

// name as include directive uses
  std::string m_IncludeName;

// refers back to the index of the  cmMakefile's array
// of cmClassFile objects which this class class describes,
// -1 for files not in the array
  int m_ClassFileIndex;	
  
// flag to determine if depends have
// been done for this file
  bool m_DependDone;
  
// function to add the depends of another file to this one
  void MergeInfo(cmDependInformation*);
  
// remove duplicate depends from the index list
  void RemoveDuplicateIndices();
};


// cmMakeDepend is used to generate dependancy information for
// the classes in a makefile
class cmMakeDepend
{
public:
  cmMakeDepend();
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
   * Set a regular expression that include files must match
   * in order to be considered as part of the depend information
   */
  void SetIncludeRegularExpression(const char* regex);
  /**
   * Add a directory to the search path for include files
   */
  void AddSearchPath(const char*);
private: 
  void AddFileToSearchPath(const char* filepath);
  /**
   * Find the index into the m_DependInformation array
   * that matches the given m_IncludeName
   */
  int FindInformation(const char* includeName);
  /**
   * Compute the depend information for this class
   */
  void Depend(cmDependInformation* info);
  /** 
   * Find the full path name for the given file name.
   * This uses the include directories
   */
  std::string FullPath(const char*);
private:
  cmMakefile* m_Makefile;
  bool m_Verbose;
  cmRegularExpression m_IncludeFileRegularExpression;
  typedef std::vector<cmDependInformation*> DependArray;
  DependArray m_DependInformation;
  std::vector<std::string> m_IncludeDirectories;
};

#endif
