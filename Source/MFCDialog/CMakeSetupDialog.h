// CMakeSetupDialogDlg.h : header file
//

#if !defined(AFX_CMakeSetupDialogDLG_H__AC17A6F6_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_)
#define AFX_CMakeSetupDialogDLG_H__AC17A6F6_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../cmMakefile.h"
#include "PropertyList.h"
/////////////////////////////////////////////////////////////////////////////
// CMakeSetupDialog dialog

class CMakeSetupDialog : public CDialog
{
// Construction
public:
  CMakeSetupDialog(CWnd* pParent = NULL);	// standard constructor
protected:
  bool Browse(CString&, const char* title);
  void SaveToRegistry();
  void LoadFromRegistry();
  void InitMakefile();
  void ReadRegistryValue(HKEY hKey,
			 CString *val,
			 const char *key,
			 const char *aadefault);
// Dialog Data
  //{{AFX_DATA(CMakeSetupDialog)
  enum { IDD = IDD_CMakeSetupDialog_DIALOG };
  cmMakefile    m_Makefile;
  bool          m_InitMakefile;
  bool          m_GUIInitialized;
  CString	m_WhereSource;
  CString	m_WhereBuild;
  CString	m_WhereSourceLast;
  CString	m_WhereBuildLast;
  CPropertyList m_CacheEntriesList;
  //}}AFX_DATA
  
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMakeSetupDialog)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  //}}AFX_VIRTUAL
  
// Implementation
protected:
  
  HICON m_hIcon;
  CString m_RegistryKey;
  // Generated message map functions
  //{{AFX_MSG(CMakeSetupDialog)
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnChangeEdit1();
  afx_msg void OnBrowse();
  virtual void OnOK();
  virtual void OnBuildProjects();
  afx_msg void OnButton3();
  
  // copy from the cache manager to the cache edit list box
  void FillCacheEditorFromCacheManager();
  // copy from the list box to the cache manager
  void FillCacheManagerFromCacheEditor();
  
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMakeSetupDialogDLG_H__AC17A6F6_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_)
