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
#if !defined(CMAKE_GEN_DIALOG_INCLUDED)
#define CMAKE_GEN_DIALOG_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMakeGenDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCMakeGenDialog dialog

class cmake;

class CCMakeGenDialog : public CDialog
{
// Construction
public:
        CCMakeGenDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CCMakeGenDialog)
        enum { IDD = IDD_GEN_DIALOG };
        CStatic       m_BuildForLabel;
        CComboBox       m_GeneratorChoice;
        CString m_GeneratorChoiceString;
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CCMakeGenDialog)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CCMakeGenDialog)
        virtual BOOL OnInitDialog();
        afx_msg void OnEditchangeGenerator();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
          
          public:
        cmake *m_CMakeInstance;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
