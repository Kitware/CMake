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
