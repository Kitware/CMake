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
// CMakeSetupdialog.h : main header file for the CMakeSetupDIALOG application
//

#if !defined(AFX_CMakeSetupDIALOG_H__AC17A6F4_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_)
#define AFX_CMakeSetupDIALOG_H__AC17A6F4_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"           // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMakeSetup:
// See CMakeSetupdialog.cpp for the implementation of this class
//

class CMakeSetup : public CWinApp
{
public:
  CMakeSetup();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMakeSetup)
public:
  virtual BOOL InitInstance();
  //}}AFX_VIRTUAL

// Implementation

  //{{AFX_MSG(CMakeSetup)
  // NOTE - the ClassWizard will add and remove member functions here.
  //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMakeSetupDIALOG_H__AC17A6F4_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_)
