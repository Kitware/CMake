// CMakeSetupDialogDlg.h : header file
//

#if !defined(AFX_CMakeSetupDialogDLG_H__AC17A6F6_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_)
#define AFX_CMakeSetupDialogDLG_H__AC17A6F6_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
  void ReadRegistryValue(HKEY hKey,
			 CString *val,
			 char *key,
			 char *adefault);
// Dialog Data
  //{{AFX_DATA(CMakeSetupDialog)
	enum { IDD = IDD_CMakeSetupDialog_DIALOG };
	CComboBox	m_WhereSourceControl;
	CComboBox	m_WhereBuildControl;
	CListBox	m_CacheEntriesList;
  CString	m_WhereITK;
  CString	m_WhereBuildITK;
	CString	m_WhereBuild;
	CString	m_WhereSource;
	//}}AFX_DATA
  
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMakeSetupDialog)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  //}}AFX_VIRTUAL
  
// Implementation
protected:
  HICON m_hIcon;
  
  // Generated message map functions
  //{{AFX_MSG(CMakeSetupDialog)
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnBrowse();
  virtual void OnOK();
  afx_msg void OnButton3();
	afx_msg void OnBuildProjects();
	afx_msg void OnChangeWhereBuild();
	afx_msg void OnChangeWhereSource();
	afx_msg void OnEditchangeWhereBuild();
	afx_msg void OnEditchangeWhereSource();
	afx_msg void OnSelchangeWhereSource();
	afx_msg void OnSelchangeWhereBuild();
	afx_msg void OnEditupdateWhereBuild();
	afx_msg void OnCloseupWhereBuild();
	afx_msg void OnSelendokWhereBuild();
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMakeSetupDialogDLG_H__AC17A6F6_4634_11D4_8F21_00A0CC33FCD3__INCLUDED_)
