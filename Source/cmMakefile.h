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
 * cmMakefile - used to parse and store the contents of a
 * CMakeLists.txt makefile in memory.
 */
#ifndef cmMakefile_h
#define cmMakefile_h
#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "cmClassFile.h"
#include "cmCollectFlags.h"
#include <vector>
#include <fstream>
#include <iostream>

class cmMakefile
{
public:
  cmMakefile();
  // Parse a Makfile.in file
  bool ReadMakefile(const char* makefile); 
  // Print useful stuff to stdout
  void Print();
  // Set the home directory for the project
  void SetHomeDirectory(const char* dir) 
    {
      m_cmHomeDirectory = dir;
    }
  const char* GetHomeDirectory() 
    {
      return m_cmHomeDirectory.c_str();
    }
  // Set the current directory in the project
  void SetCurrentDirectory(const char* dir) 
    {
      m_cmCurrentDirectory = dir;
    }
  const char* GetCurrentDirectory() 
    {
      return m_cmCurrentDirectory.c_str();
    }
  // Set the name of the library that is built by this makefile
  void SetLibraryName(const char* lib)
    {
      m_LibraryName = lib;
    }
  const char* GetLibraryName()
    {
      return m_LibraryName.c_str();
    }
  // Set the name of the library that is built by this makefile
  void SetProjectName(const char* lib)
    {
      m_ProjectName = lib;
    }
  const char* GetProjectName()
    {
      return m_ProjectName.c_str();
    }
  
  // Set the name of the library that is built by this makefile
  void SetOutputDirectory(const char* lib)
    {
      m_OutputDirectory = lib;
    }
  const char* GetOutputDirectory()
    {
      return m_OutputDirectory.c_str();
    }
  
  // Set the name of the library that is built by this makefile
  void SetOutputHomeDirectory(const char* lib)
    {
      m_OutputHomeDirectory = lib;
    }
  const char* GetOutputHomeDirectory()
    {
      return m_OutputHomeDirectory.c_str();
    }
  cmCollectFlags& GetBuildFlags() 
    {
      return m_BuildFlags;
    }
  const std::vector<std::string>& GetSubDirectories()
    { 
      return m_SubDirectories;
    }
  
  bool HasExecutables() 
    {
      return m_Executables;
    }
  
private:
  void ReadTemplateInstanceDirectory(std::string&);
  void ReadClasses(std::ifstream& fin, bool t);
  friend class cmMakeDepend;	// make depend needs direct access 
				// to the m_Classes array
protected:
  bool m_Executables;
  std::string m_Prefix;
  std::vector<std::string> m_TemplateDirectories; // Template directory name if found in file
  std::string m_OutputDirectory; // Current output directory for makefile
  std::string m_OutputHomeDirectory; // Top level output directory
  std::string m_cmHomeDirectory; // Home directory for source
  std::string m_cmCurrentDirectory; // current directory in source
  std::string m_LibraryName;	// library name
  std::string m_ProjectName;	// project name
  std::vector<cmClassFile> m_Classes; // list of classes in makefile
  std::vector<std::string> m_SubDirectories; // list of sub directories
  std::vector<std::string> m_MakeVerbatim; // lines copied from input file
  cmCollectFlags m_BuildFlags;
};


#endif
