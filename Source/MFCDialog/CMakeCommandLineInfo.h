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
// CMakeCommandLineInfo.h : main header file for the command line arguments
//

#if !defined(CMAKECOMMANDLINEINFO_H)
#define CMAKECOMMANDLINEINFO_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "../cmStandardIncludes.h"

///////////////////////////////////////////////////////////////
// CMakeCommandLineInfo:
// See CMakeCommandLineInfo.cpp for the implementation of this class
//

class CMakeCommandLineInfo : public CCommandLineInfo
{ 
  // Construction
public:
  CMakeCommandLineInfo(); 

  // Attributes
public:
  CString m_WhereSource;
  CString m_WhereBuild;
  BOOL m_AdvancedValues;
  CString m_GeneratorChoiceString;
  CString m_LastUnknownParameter;
  
  int GetArgC() { return static_cast<int>(m_Argv.size()); }
  const char*const* GetArgV() { return &*m_Argv.begin(); }
  
  std::string m_Argv0;
  std::vector<cmStdString> m_Arguments;
  std::vector<const char*> m_Argv;
  
  // Operations
public:
  void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast); 

  // Implementation
public:
  virtual ~CMakeCommandLineInfo();
protected:
  static int GetBoolValue(const CString&);
};

#endif // !defined(CMAKECOMMANDLINEINFO_H)
