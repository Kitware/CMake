// pcbuilderdialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CMakeSetup.h"
#include "PathDialog.h"
#include "CMakeSetupDialog.h"
#include "../cmCacheManager.h"
#include "../cmake.h"
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
  m_RegistryKey  = "Software\\Kitware\\CMakeSetup\\Settings\\StartPath";
  
  //{{AFX_DATA_INIT(CMakeSetupDialog)
	m_WhereBuild = _T("");
	m_WhereSource = _T("");
	//}}AFX_DATA_INIT
  // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_BuildPathChanged = false;
  // Find the path to the cmake.exe executable
  char fname[1024];
  ::GetModuleFileName(NULL,fname,1023);
  // extract just the path part
  m_PathToExecutable = cmSystemTools::GetProgramPath(fname).c_str();
  // add the cmake.exe to the path
  m_PathToExecutable += "/cmake.exe";
}

void CMakeSetupDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMakeSetupDialog)
	DDX_Control(pDX, IDOK, m_CancelButton);
	DDX_Control(pDX, IDC_MouseHelpCaption, m_MouseHelp);
	DDX_Control(pDX, IDC_CMAKE_VERSION, m_VersionDisplay);
	DDX_Control(pDX, IDC_BuildProjects, m_BuildProjects);
	DDX_Control(pDX, IDC_FRAME, m_ListFrame);
	DDX_Control(pDX, IDC_WhereSource, m_WhereSourceControl);
	DDX_Control(pDX, IDC_WhereBuild, m_WhereBuildControl);
	DDX_Control(pDX, IDC_LIST2, m_CacheEntriesList);
	DDX_CBString(pDX, IDC_WhereBuild, m_WhereBuild);
	DDX_CBString(pDX, IDC_WhereSource, m_WhereSource);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMakeSetupDialog, CDialog)
  //{{AFX_MSG_MAP(CMakeSetupDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_BuildProjects, OnBuildProjects)
  ON_CBN_EDITCHANGE(IDC_WhereBuild, OnChangeWhereBuild)
  ON_CBN_EDITCHANGE(IDC_WhereSource, OnChangeWhereSource)
  ON_CBN_SELCHANGE(IDC_WhereBuild, OnSelendokWhereBuild)
  ON_BN_CLICKED(IDC_BUTTON2, OnBrowseWhereSource)
  ON_BN_CLICKED(IDC_BUTTON3, OnBrowseWhereBuild)
  ON_CBN_SELENDOK(IDC_WhereSource, OnSelendokWhereSource)
	ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
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
  // Load source and build dirs from registry
  this->LoadFromRegistry();
  // try to load the cmake cache from disk
  this->LoadCacheFromDiskToGUI();
  m_WhereBuildControl.LimitText(2048);
  m_WhereSourceControl.LimitText(2048);
  // Set the version number
  char tmp[1024];
  sprintf(tmp,"Version %d.%d", cmMakefile::GetMajorVersion(),
          cmMakefile::GetMinorVersion());
  SetDlgItemText(IDC_CMAKE_VERSION, tmp);
  this->UpdateData(FALSE);
  return TRUE;  // return TRUE  unless you set the focus to a control
}


// About dialog invoke
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



// Browse button
bool CMakeSetupDialog::Browse(CString &result, const char *title)
{
  CPathDialog dlg("Select Path", title, result); 
  if(dlg.DoModal()==IDOK)
    {
    result =  dlg.GetPathName();
    return true;
    }
  else
    {
    return false;
    }
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
    // load some values
    CString regvalue;
    this->ReadRegistryValue(hKey, &(regvalue),"WhereSource","C:\\");
    if(m_WhereSource != regvalue)
      {
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),"WhereSource3","");
      RegSetValueEx(hKey, _T("WhereSource4"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)regvalue, 
                    regvalue.GetLength());
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),"WhereSource2","");
      RegSetValueEx(hKey, _T("WhereSource3"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)regvalue, 
                    regvalue.GetLength());
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),"WhereSource","");
      RegSetValueEx(hKey, _T("WhereSource2"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)regvalue, 
                    regvalue.GetLength());
      RegSetValueEx(hKey, _T("WhereSource"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)m_WhereSource, 
                    m_WhereSource.GetLength());
      }
    this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild","C:\\");
    if(m_WhereBuild != regvalue)
      {
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild3","");
      RegSetValueEx(hKey, _T("WhereBuild4"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)regvalue, 
                    regvalue.GetLength());
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild2","");
      RegSetValueEx(hKey, _T("WhereBuild3"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)regvalue, 
                    regvalue.GetLength());
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild","");
      RegSetValueEx(hKey, _T("WhereBuild2"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)regvalue, 
                    regvalue.GetLength());
      RegSetValueEx(hKey, _T("WhereBuild"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)m_WhereBuild, 
                    m_WhereBuild.GetLength());
      }
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
    // load some values
    this->ReadRegistryValue(hKey, &(m_WhereSource),"WhereSource","C:\\");
    this->ReadRegistryValue(hKey, &(m_WhereBuild),"WhereBuild","C:\\");
    m_WhereSourceControl.AddString(m_WhereSource);
    m_WhereBuildControl.AddString(m_WhereBuild);

    CString regvalue;
    this->ReadRegistryValue(hKey, &(regvalue),"WhereSource2","C:\\");
    m_WhereSourceControl.AddString(regvalue);
    regvalue = "";
    this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild2","C:\\");
    m_WhereBuildControl.AddString(regvalue);

    regvalue = "";
    this->ReadRegistryValue(hKey, &(regvalue),"WhereSource3","C:\\");
    m_WhereSourceControl.AddString(regvalue);
    regvalue = "";
    this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild3","C:\\");
    m_WhereBuildControl.AddString(regvalue);

    regvalue = "";
    this->ReadRegistryValue(hKey, &(regvalue),"WhereSource4","C:\\");
    m_WhereSourceControl.AddString(regvalue);
    regvalue = "";
    this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild4","C:\\");
    m_WhereBuildControl.AddString(regvalue);
    }
  RegCloseKey(hKey);
}



// Callback for browse source button
void CMakeSetupDialog::OnBrowseWhereSource() 
{
  this->UpdateData();
  Browse(m_WhereSource, "Enter Path to Source");
  this->UpdateData(false);
  this->OnChangeWhereSource();
}

// Callback for browser build button
void CMakeSetupDialog::OnBrowseWhereBuild() 
{
  this->UpdateData();
  Browse(m_WhereBuild, "Enter Path to Build");
  this->UpdateData(false);
  this->OnChangeWhereBuild();
}

// Callback for build projects button
void CMakeSetupDialog::OnBuildProjects() 
{
  if(!cmSystemTools::FileExists(m_WhereBuild))
    {
    std::string message =
      "Build directory does not exist, should I create it?\n\n"
      "Directory: ";
    message += (const char*)m_WhereBuild;
    if(MessageBox(message.c_str(), "Create Directory", MB_OKCANCEL) == IDOK)
      {
      cmSystemTools::MakeDirectory(m_WhereBuild);
      }
    else
      {
      MessageBox("Build Project aborted, nothing done.");
      return;
      }
    }
  // set the wait cursor
  ::SetCursor(LoadCursor(NULL, IDC_WAIT));
  // get all the info from the dialog
  this->UpdateData();
  if(!m_BuildPathChanged)
    {
    // if the build path has not changed save the 
    // current GUI values to the cache
    this->SaveCacheFromGUI();
    }
  // Make sure we are working from the cache on disk
  this->LoadCacheFromDiskToGUI();
  // create a cmake object
  cmake make;
  // create the arguments for the cmake object
  std::vector<std::string> args;
  args.push_back((const char*)m_PathToExecutable);
  std::string arg;
  arg = "-H";
  arg += m_WhereSource;
  args.push_back(arg);
  arg = "-B";
  arg += m_WhereBuild;
  args.push_back(arg);
  // run the generate process
  if(make.Generate(args) != 0)
    {
    cmSystemTools::Error(
      "Error in generation process, project files may be invalid");
    cmSystemTools::ResetErrorOccuredFlag();
    }
  // update the GUI with any new values in the caused by the
  // generation process
  this->LoadCacheFromDiskToGUI();
  // save source and build paths to registry
  this->SaveToRegistry();
  // path is up-to-date now
  m_BuildPathChanged = false;
  // put the cursor back
  ::SetCursor(LoadCursor(NULL, IDC_ARROW));
}




// callback for combo box menu where build selection
void CMakeSetupDialog::OnSelendokWhereBuild() 
{
  m_WhereBuildControl.GetLBText(m_WhereBuildControl.GetCurSel(), 
                                m_WhereBuild);
  this->UpdateData(FALSE);
  this->OnChangeWhereBuild();
}

// callback for combo box menu where source selection
void CMakeSetupDialog::OnSelendokWhereSource() 
{
  m_WhereSourceControl.GetLBText(m_WhereSourceControl.GetCurSel(), 
                                 m_WhereSource);
  this->UpdateData(FALSE);
  this->OnChangeWhereSource();
}

// callback for chaing source directory
void CMakeSetupDialog::OnChangeWhereSource() 
{
}

// callback for changing the build directory
void CMakeSetupDialog::OnChangeWhereBuild() 
{
  this->UpdateData();
  std::string cachefile = m_WhereBuild;
  cachefile += "/CMakeCache.txt";
  m_CacheEntriesList.RemoveAll();
  if(cmSystemTools::FileExists(cachefile.c_str()))
    {
    m_CacheEntriesList.ShowWindow(SW_SHOW);
    this->LoadCacheFromDiskToGUI();
    m_BuildPathChanged = true;
    }
}


// copy from the cache manager to the cache edit list box
void CMakeSetupDialog::FillCacheGUIFromCacheManager()
{
  const cmCacheManager::CacheEntryMap &cache =
    cmCacheManager::GetInstance()->GetCacheMap();
  for(cmCacheManager::CacheEntryMap::const_iterator i = cache.begin();
      i != cache.end(); ++i)
    {
    const char* key = i->first.c_str();
    const cmCacheManager::CacheEntry& value = i->second;
    switch(value.m_Type )
      {
      case cmCacheManager::BOOL:
        if(cmCacheManager::GetInstance()->IsOn(key))
          {
          m_CacheEntriesList.AddProperty(key,
                                         "ON",
                                         value.m_HelpString.c_str(),
                                         CPropertyList::CHECKBOX,"");
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         value.m_HelpString.c_str(),
                                         CPropertyList::CHECKBOX,"");
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       CPropertyList::PATH,"");
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       CPropertyList::FILE,"");
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key,
                                       value.m_Value.c_str(),
                                       value.m_HelpString.c_str(),
                                       CPropertyList::EDIT,"");
        break;
      case cmCacheManager::INTERNAL:
        break;
      }
    }
}

// copy from the list box to the cache manager
void CMakeSetupDialog::FillCacheManagerFromCacheGUI()
{ 
  cmCacheManager::GetInstance()->GetCacheMap();
  std::set<CPropertyItem*> items = m_CacheEntriesList.GetItems();
  for(std::set<CPropertyItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    {
    CPropertyItem* item = *i; 
    cmCacheManager::CacheEntry *entry = 
      cmCacheManager::GetInstance()->GetCacheEntry(
        (const char*)item->m_propName);
    if (entry)
      {
      entry->m_Value = item->m_curValue;
      }
    }
}

  

//! Load cache file from m_WhereBuild and display in GUI editor
void CMakeSetupDialog::LoadCacheFromDiskToGUI()
{
  if(m_WhereBuild != "")
    {
    cmCacheManager::GetInstance()->LoadCache(m_WhereBuild);
    this->FillCacheGUIFromCacheManager();
    }
}

//! Save GUI values to cmCacheManager and then save to disk.
void CMakeSetupDialog::SaveCacheFromGUI()
{
  this->FillCacheManagerFromCacheGUI();
  if(m_WhereBuild != "")
    {
    cmCacheManager::GetInstance()->SaveCache(m_WhereBuild);
    }
}



void CMakeSetupDialog::OnSize(UINT nType, int cx, int cy) 
{
  CDialog::OnSize(nType, cx, cy);
  // TODO: Add your message handler code here
  if(m_CacheEntriesList.m_hWnd)
    {
    m_ListFrame.SetWindowPos(&wndTop, 0, 0, cx-28, cy-137,
                             SWP_NOMOVE | SWP_NOZORDER);
    m_CacheEntriesList.SetWindowPos(&wndTop, 0, 0, cx-48, cy-168,
                                    SWP_NOMOVE | SWP_NOZORDER);
    m_BuildProjects.SetWindowPos(&wndTop, 143, cy-33, 0, 0, 
                                 SWP_NOSIZE | SWP_NOZORDER);
    m_MouseHelp.SetWindowPos(&wndTop, 159, cy-57,
                             0, 0,
                             SWP_NOSIZE | SWP_NOZORDER);
    m_CancelButton.SetWindowPos(&wndTop, 329, cy-33, 0, 0, 
                                 SWP_NOSIZE | SWP_NOZORDER);
    m_VersionDisplay.SetWindowPos(&wndTop, 5, cy-23, 0, 0,
                                  SWP_NOSIZE | SWP_NOZORDER);
    }
  
}


void CMakeSetupDialog::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
  lpMMI->ptMinTrackSize.x = 550;
  lpMMI->ptMinTrackSize.y = 272;
}

