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
