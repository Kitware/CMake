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
#include "cmClassFile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"



// Set the name of the class and the full path to the file.
// The class must be found in dir and end in name.cxx, name.txx, 
// name.c or it will be considered a header file only class
// and not included in the build process
void cmClassFile::SetName(const char* name, const char* dir)
{
  m_HeaderFileOnly = true;
  m_ClassName = name;
  std::string pathname = dir;
  if(pathname != "")
    {
    pathname += "/";
    }
  
  // First try and see whether the listed file can be found
  // as is without extensions added on.
  pathname += m_ClassName;
  std::string hname = pathname;
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    m_HeaderFileOnly = false;
    m_FullPath = hname;
    return;
    }
  
  // Try various extentions
  hname = pathname;
  hname += ".cxx";
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    m_ClassExtension = "cxx";
    m_HeaderFileOnly = false;
    m_FullPath = hname;
    return;
    }
  
  hname = pathname;
  hname += ".c";
  if(cmSystemTools::FileExists(hname.c_str()))
  {
    m_HeaderFileOnly = false;
    m_ClassExtension = "c";
    m_FullPath = hname;
    return;
  }
  hname = pathname;
  hname += ".txx";
  if(cmSystemTools::FileExists(hname.c_str()))
  {
    m_HeaderFileOnly = false;
    m_ClassExtension = "txx";
    m_FullPath = hname;
    return;
  }
  hname = pathname;
  hname += ".h";
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    m_ClassExtension = "h";
    m_FullPath = hname;
    return;
    }
  
  cmSystemTools::Error("can not find file ", hname.c_str());
  cmSystemTools::Error("Tried .txx .cxx .c for ", hname.c_str());
}

void cmClassFile::SetName(const char* name, const char* dir, const char *ext,
                          bool hfo)
{
  m_HeaderFileOnly = hfo;
  m_ClassName = name;
  std::string pathname = dir;
  if(pathname != "")
    {
    pathname += "/";
    }
  
  pathname += m_ClassName + "." + ext;
  m_FullPath = pathname;
  m_ClassExtension = ext;
  return;
}

void cmClassFile::Print()
{
  if(m_AbstractClass)
    std::cout <<  "Abstract ";
  else
    std::cout << "Concrete ";
  if(m_HeaderFileOnly)
    std::cout << "Header file ";
  else
    std::cout << "CXX file ";
  if(m_IsExecutable)
    std::cout << "Executable ";
  else
    std::cout << "Non Executable ";
  std::cout << m_ClassName << std::endl;
}
