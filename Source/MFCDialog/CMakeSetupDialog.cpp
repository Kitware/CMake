// pcbuilderdialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CMakeSetup.h"
#include "CMakeSetupDialog.h"
#include "../cmDSWMakefile.h"
#include "../cmWindowsConfigure.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

// Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
    };

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
  //{{AFX_MSG_MAP(CAboutDlg)
  // No message handlers
  //}}AFX_MSG_MAP
  END_MESSAGE_MAP();


/////////////////////////////////////////////////////////////////////////////
// CMakeSetupDialog dialog

CMakeSetupDialog::CMakeSetupDialog(CWnd* pParent /*=NULL*/)
  : CDialog(CMakeSetupDialog::IDD, pParent)
{
  CString startPath = _pgmptr;
  startPath.Replace('\\', '_');
  startPath.Replace(':', '_');
  startPath.Replace(".EXE", "");
  startPath.Replace(".exe", "");
  m_RegistryKey  = "Software\\Kitware\\CMakeSetup\\Settings\\";
  // _pgmptr should be the directory from which cmake was run from
  // use it as the unique key for the dialog
  m_RegistryKey += startPath;
  
  //{{AFX_DATA_INIT(CMakeSetupDialog)
  m_WhereSource = _T("");
  m_WhereBuild = _T("");
  //}}AFX_DATA_INIT
  // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  // Guess the initial source directory based on the location
  // of this program, it should be in CMake/Source/
  startPath = _pgmptr;
  int removePos = startPath.Find("\\CMake\\Source");
  if(removePos == -1)
    {
    removePos = startPath.Find("/CMake/Source");
    }
  if(removePos != -1)
    {
    startPath = startPath.Left(removePos);
    }
  m_WhereSource = startPath;
  this->LoadFromRegistry();
}

void CMakeSetupDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMakeSetupDialog)
  DDX_Text(pDX, IDC_WhereSource, m_WhereSource);
  DDX_Text(pDX, IDC_WhereBuild, m_WhereBuild);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMakeSetupDialog, CDialog)
  //{{AFX_MSG_MAP(CMakeSetupDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_EN_CHANGE(IDC_WhereSource, OnChangeEdit1)
  ON_BN_CLICKED(IDC_BUTTON2, OnBrowse)
  ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
  //}}AFX_MSG_MAP
  END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMakeSetupDialog message handlers

  BOOL CMakeSetupDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL)
    {
    CString strAboutMenu;
    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty())
      {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
      }
    }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);			// Set big icon
  SetIcon(m_hIcon, FALSE);		// Set small icon
	
  // TODO: Add extra initialization here
	
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMakeSetupDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
    }
  else
    {
    CDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMakeSetupDialog::OnPaint() 
{
  if (IsIconic())
    {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
    }
  else
    {
    CDialog::OnPaint();
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMakeSetupDialog::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}

void CMakeSetupDialog::OnChangeEdit1() 
{
  // TODO: If this is a RICHEDIT control, the control will not
  // send this notification unless you override the CDialog::OnInitDialog()
  // function and call CRichEditCtrl().SetEventMask()
  // with the ENM_CHANGE flag ORed into the mask.
	
  // TODO: Add your control notification handler code here
	
}

void CMakeSetupDialog::OnBrowse() 
{
  this->UpdateData();
  Browse(m_WhereSource, "Enter Path to Insight Source");
  this->UpdateData(false);
}

bool CMakeSetupDialog::Browse(CString &result, const char *title)
{
// don't know what to do with initial right now...
  char szPathName[4096];
  BROWSEINFO bi;
 
  bi.hwndOwner = m_hWnd;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = (LPTSTR)szPathName;
  bi.lpszTitle = title;
  bi.ulFlags = BIF_BROWSEINCLUDEFILES  ;
  bi.lpfn = NULL;

  LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

  bool bSuccess = (bool)SHGetPathFromIDList(pidl, szPathName);
  if(bSuccess)
    {
    result = szPathName;
    }
  
  return bSuccess;
}

void CMakeSetupDialog::OnOK() 
{ 
  // get all the info from the screen
  this->UpdateData();
  
  // configure the system for VC60
  cmWindowsConfigure config;
  config.SetWhereSource(m_WhereSource);
  config.SetWhereBuild(m_WhereBuild);
  std::string configSrc;
  configSrc = m_WhereSource;
  configSrc += "/CMakeSetupConfig.MSC";
  if(!config.Configure(configSrc.c_str()))
    { 
    std::string error = "Error: in configuring system from: ";
    error += configSrc;
    error += "\nProject NOT created!";
    ::MessageBox(0, error.c_str(), "config ERROR", MB_OK);
    return;
    }
  
  
  cmDSWMakefile builder;
  // Set the ITK home directory
  builder.SetHomeDirectory(m_WhereSource);
  // Set the CMakeLists.txt file
  CString makefileIn = m_WhereSource;
  makefileIn += "/CMakeLists.txt";
  builder.ReadMakefile(makefileIn);
  // Set the output directory
  builder.SetOutputDirectory(m_WhereBuild);
  // set the directory which contains the CMakeLists.txt
  builder.SetCurrentDirectory(m_WhereSource);
  // Create the master DSW file and all children dsp files for ITK
  builder.OutputDSWFile();
  CDialog::OnOK();
  this->SaveToRegistry();
}

void CMakeSetupDialog::OnButton3() 
{
  this->UpdateData();
  Browse(m_WhereBuild, "Enter Path to Insight Build");
  this->UpdateData(false);
}

void CMakeSetupDialog::SaveToRegistry()
{ 
  HKEY hKey;
  DWORD dwDummy;

  if(RegCreateKeyEx(HKEY_CURRENT_USER, 
		    m_RegistryKey,
		    0, "", REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, 
		    NULL, &hKey, &dwDummy) != ERROR_SUCCESS) 
    {
    return;
    }
  else
    {
    RegSetValueEx(hKey, _T("WhereSource"), 0, REG_SZ, 
		  (CONST BYTE *)(const char *)m_WhereSource, 
		  m_WhereSource.GetLength());
    RegSetValueEx(hKey, _T("WhereBuild"), 0, REG_SZ, 
		  (CONST BYTE *)(const char *)m_WhereBuild, 
		  m_WhereBuild.GetLength());
    
    }
  RegCloseKey(hKey);
}


void CMakeSetupDialog::ReadRegistryValue(HKEY hKey,
                                         CString *val,
                                         const char *key,
                                         const char *adefault)
{
  DWORD dwType, dwSize;
  char *pb;

  dwType = REG_SZ;
  pb = val->GetBuffer(MAX_PATH);
  dwSize = MAX_PATH;
  if(RegQueryValueEx(hKey,_T(key), NULL, &dwType, 
		     (BYTE *)pb, &dwSize) != ERROR_SUCCESS)
    {
    val->ReleaseBuffer();
    *val = _T(adefault);
    }
  else
    {
    val->ReleaseBuffer();
    }
}


void CMakeSetupDialog::LoadFromRegistry()
{ 
  HKEY hKey;
  if(RegOpenKeyEx(HKEY_CURRENT_USER, 
		  m_RegistryKey, 
		  0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
    return;
    }
  else
    {
    // save some values
    this->ReadRegistryValue(hKey, &(m_WhereSource),"WhereSource","C:\\Insight");
    this->ReadRegistryValue(hKey, &(m_WhereBuild),"WhereBuild",
			    "C:\\Insight");
    }
  RegCloseKey(hKey);
}
