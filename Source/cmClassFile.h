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
#ifndef cmClassFile_h
#define cmClassFile_h

#include "cmStandardIncludes.h"

/** \class cmClassFile
 * \brief Represent a class loaded from a makefile.
 *
 * cmClassFile is represents a class loaded from 
 * a makefile.
 */
class cmClassFile
{
public:
  /**
   * Construct instance as a concrete class with both a
   * .h and .cxx file.
   */
  cmClassFile()
    {
    m_AbstractClass = false;
    m_HeaderFileOnly = false;
    m_IsExecutable = false;
    }
  
  /**
   * Set the name of the file, given the directory
   * the file should be in.  Various extensions are tried on 
   * the name (e.g., .cxx, .cpp) in the directory to find the actual file.
   */
  void SetName(const char* name, const char* dir);

  /**
   * Print the structure to std::cout.
   */
  void Print();

  /**
   * Indicate whether the class is abstract (non-instantiable).
   */
  bool m_AbstractClass;

  /**
   * Indicate whether this class is defined with only the header file.
   */
  bool m_HeaderFileOnly;

  /**
   * Indicate whether this class is an executable file
   */
  bool m_IsExecutable;

  /**
   * The full path to the file.
   */
  std::string m_FullPath;

  /**
   * The file name associated with stripped off directory and extension.
   * (In most cases this is the name of the class.)
   */
  std::string m_ClassName;

  /**
   * The dependencies of this class are gathered here.
   */
  std::vector<std::string> m_Depends;
};

#endif
