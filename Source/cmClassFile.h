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
#include <string>
#include <vector>

// helper function returns true if a file exits
bool cmFileExists(const char* filename);

struct cmClassFile
{
  // Set the name of the file
  void SetName(const char* name, const char* dir);
  void Print();

  bool m_AbstractClass;
  bool m_HeaderFileOnly;
  std::string m_FullPath;
  std::string m_ClassName;
  std::vector<std::string> m_Depends;
};

#endif
