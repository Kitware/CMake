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
 * cmClassFile a structure that represents a class loaded from 
 * a makefile.
 */
#ifndef cmClassFile_h
#define cmClassFile_h
#include "cmStandardIncludes.h"


struct cmClassFile
{
  cmClassFile()
    {
      m_AbstractClass = false;
      m_HeaderFileOnly = false;
    }
  
  /**
   * Set the name of the file, given the directory
   * the file should be in.   Extensions are tried on 
   * the name in the directory to find the actual file.
   */
  void SetName(const char* name, const char* dir);
  /**
   * print the structure to cout
   */
  void Print();

  bool m_AbstractClass;         // is this an abstract class
  bool m_HeaderFileOnly;        // is this file only a header file
  std::string m_FullPath;       // full path to the file
  std::string m_ClassName;      // class name
  // list of files that this file depends on
  std::vector<std::string> m_Depends;
};

#endif
