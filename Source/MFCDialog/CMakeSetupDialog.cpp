// pcbuilderdialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "shellapi.h"
// a fun undef for DOT NET
#undef DEBUG
#include "CMakeSetup.h"
#include "MakeHelp.h"
#include "PathDialog.h"
#include "CMakeSetupDialog.h"
#include "CMakeCommandLineInfo.h"
#include "../cmListFileCache.h"
#include "../cmCacheManager.h"
#include "../cmake.h"
#include "../cmGlobalGenerator.h"
#include "../cmDynamicLoader.h"
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


void MFCMessageCallback(const char* m, const char* title, bool& nomore, void*)
{ 
  std::string message = m;
  message += "\n\n(Press  Cancel to suppress any further messages.)";
  if(::MessageBox(0, message.c_str(), title, 
                  MB_OKCANCEL|MB_TASKMODAL) == IDCANCEL)
    {
    nomore = true;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMakeSetupDialog dialog
void updateProgress(const char *msg, float prog, void *cd)
{
  char* tmp = new char[strlen(msg) + 40];
  if (prog >= 0)
    {
    sprintf(tmp,"%s %i%%",msg,(int)(100*prog));
    }
  else
    {
    sprintf(tmp,"%s",msg);    
    }
  CMakeSetupDialog *self = (CMakeSetupDialog *)cd;
  self->SetDlgItemText(IDC_PROGRESS, tmp); 
  CWnd* cancel = self->GetDlgItem(IDCANCEL);
  //
  // Retrieve and dispatch any waiting messages.
  //
  MSG wmsg;
  while (::PeekMessage (&wmsg, NULL, 0, 0, PM_REMOVE))
    {
    switch(wmsg.message)
      {
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_LBUTTONDBLCLK:
      {
      if(wmsg.hwnd == cancel->m_hWnd)
        {
        ::DispatchMessage(&wmsg);
        }
      }
      break;
      case WM_COMMAND:
      case WM_SETCURSOR:
      case WM_PAINT:
        ::DispatchMessage(&wmsg);
      break;
      }
    }
  delete [] tmp;
}

// Convert to Win32 path (slashes). This calls the system tools one and then
// removes the spaces. It is not in system tools because we don't want any
// generators accidentally use it
std::string ConvertToWindowsPath(const char* path)
{
  // Convert to output path.
  // Remove the "" around it (if any) since it's an output path for
  // the shell. If another shell-oriented feature is not designed 
  // for a GUI use, then we are in trouble.
  // save the value of the force to unix path option
  bool saveForce = cmSystemTools::GetForceUnixPaths();
  // make sure we get windows paths no matter what for the GUI
  cmSystemTools::SetForceUnixPaths(false);
  std::string s = cmSystemTools::ConvertToOutputPath(path);
  // now restore the force unix path to its previous value
  cmSystemTools::SetForceUnixPaths(saveForce);
  if (s.size())
    {
    std::string::iterator i = s.begin();
    if (*i == '\"')
      {
      s.erase(i, i + 1);
      }
    i = s.begin() + s.length() - 1;
    if (*i == '\"')
      {
      s.erase(i, i + 1);
      }
    }
  return s;
}

CMakeSetupDialog::CMakeSetupDialog(const CMakeCommandLineInfo& cmdInfo,
                                   CWnd* pParent /*=NULL*/)
  : CDialog(CMakeSetupDialog::IDD, pParent)
{ 
  m_GeneratorPicked = false;
  m_Cursor = LoadCursor(NULL, IDC_ARROW);
  m_RunningConfigure = false;
  cmSystemTools::SetRunCommandHideConsole(true);
  cmSystemTools::SetErrorCallback(MFCMessageCallback);
  m_RegistryKey  = "Software\\Kitware\\CMakeSetup\\Settings\\StartPath";
  m_CacheEntriesList.m_CMakeSetupDialog = this;

  m_CMakeInstance = new cmake;
  m_CMakeInstance->SetProgressCallback(updateProgress, (void *)this);

  //{{AFX_DATA_INIT(CMakeSetupDialog)
        //}}AFX_DATA_INIT

  // Get the parameters from the command line info
  // If an unknown parameter is found, try to interpret it too, since it
  // is likely to be a file dropped on the shortcut :)
  if (cmdInfo.m_LastUnknownParameter.IsEmpty())
    {
    this->m_WhereSource = cmdInfo.m_WhereSource;
    this->m_WhereBuild = cmdInfo.m_WhereBuild;
    this->m_GeneratorDialog.m_GeneratorChoiceString = 
      cmdInfo.m_GeneratorChoiceString;
    this->m_AdvancedValues = cmdInfo.m_AdvancedValues;
    }
  else
    {
    this->m_WhereSource = _T("");
    this->m_WhereBuild = _T("");
    this->m_AdvancedValues = FALSE;
    this->m_GeneratorDialog.m_GeneratorChoiceString = 
      cmdInfo.m_GeneratorChoiceString;
    this->ChangeDirectoriesFromFile((LPCTSTR)cmdInfo.m_LastUnknownParameter);
    }

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

  m_oldCX = -1;
  m_deltaXRemainder = 0;
}

CMakeSetupDialog::~CMakeSetupDialog()
{
  delete m_CMakeInstance;
  // clean up globals 
  cmDynamicLoader::FlushCache();
}

void CMakeSetupDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMakeSetupDialog)
        DDX_Control(pDX, IDC_AdvancedValues, m_AdvancedValuesControl);
        DDX_Control(pDX, IDC_BROWSE_SOURCE, m_BrowseSource);
        DDX_Control(pDX, IDC_BROWSE_BUILD, m_BrowseBuild);
        DDX_Control(pDX, IDC_DELETE_BUTTON, m_DeleteButton);
        DDX_Control(pDX, IDC_HELP_BUTTON, m_HelpButton);
        DDX_Control(pDX, IDC_OK, m_OKButton);
        DDX_Control(pDX, IDCANCEL, m_CancelButton);
        DDX_CBStringExact(pDX, IDC_WhereSource, m_WhereSource);
        DDX_CBStringExact(pDX, IDC_WhereBuild, m_WhereBuild);
        DDX_Control(pDX, IDC_FRAME, m_ListFrame);
        DDX_Control(pDX, IDC_WhereSource, m_WhereSourceControl);
        DDX_Control(pDX, IDC_WhereBuild, m_WhereBuildControl);
        DDX_Control(pDX, IDC_LIST2, m_CacheEntriesList);
        DDX_Control(pDX, IDC_MouseHelpCaption, m_MouseHelp);
        DDX_Control(pDX, IDC_PROGRESS, m_StatusDisplay);
        DDX_Control(pDX, IDC_BuildProjects, m_Configure);
        DDX_Check(pDX, IDC_AdvancedValues, m_AdvancedValues);
        //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMakeSetupDialog, CDialog)
  //{{AFX_MSG_MAP(CMakeSetupDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_BUTTON2, OnBrowseWhereSource)
  ON_BN_CLICKED(IDC_BuildProjects, OnConfigure)
  ON_BN_CLICKED(IDC_BUTTON3, OnBrowseWhereBuild)
  ON_CBN_EDITCHANGE(IDC_WhereBuild, OnChangeWhereBuild)
  ON_CBN_SELCHANGE(IDC_WhereBuild, OnSelendokWhereBuild)
  ON_CBN_EDITCHANGE(IDC_WhereSource, OnChangeWhereSource)
  ON_CBN_SELENDOK(IDC_WhereSource, OnSelendokWhereSource)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
  ON_BN_CLICKED(IDC_OK, OnOk)
  ON_BN_CLICKED(IDC_DELETE_BUTTON, OnDeleteButton)
  ON_BN_CLICKED(IDC_HELP_BUTTON, OnHelpButton)
  ON_BN_CLICKED(IDC_AdvancedValues, OnAdvancedValues)
  ON_BN_DOUBLECLICKED(IDC_AdvancedValues, OnDoubleclickedAdvancedValues)
  ON_WM_DROPFILES()
  ON_BN_CLICKED(IDCANCEL, OnCancel)
        ON_WM_SETCURSOR()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMakeSetupDialog message handlers

BOOL CMakeSetupDialog::OnInitDialog()
{
  CDialog::OnInitDialog();
  this->DragAcceptFiles(true);

  // Add "Create shortcut" menu item to system menu.

  // IDM_CREATESHORTCUT must be in the system command range.
  ASSERT((IDM_CREATESHORTCUT & 0xFFF0) == IDM_CREATESHORTCUT);
  ASSERT(IDM_CREATESHORTCUT < 0xF000);

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL)
    {
    CString strCreateShortcutMenu;
    strCreateShortcutMenu.LoadString(IDS_CREATESHORTCUT);
    if (!strCreateShortcutMenu.IsEmpty())
      {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, 
                           IDM_CREATESHORTCUT, 
                           strCreateShortcutMenu);
      }

    CString strAboutMenu;
    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty())
      {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, 
                           IDM_ABOUTBOX, 
                           strAboutMenu);
      }
    }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);                        // Set big icon
  SetIcon(m_hIcon, FALSE);                // Set small icon
  // Load source and build dirs from registry
  this->LoadFromRegistry();

  // try to load the cmake cache from disk
  this->LoadCacheFromDiskToGUI();
  m_WhereBuildControl.LimitText(2048);
  m_WhereSourceControl.LimitText(2048);
    
  // Set the version number
  char tmp[1024];
  sprintf(tmp,"CMake %d.%d - %s", cmake::GetMajorVersion(),
          cmake::GetMinorVersion(), cmake::GetReleaseVersion());
  SetDlgItemText(IDC_PROGRESS, "");
  this->SetWindowText(tmp);
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
  else if ((nID & 0xFFF0) == IDM_CREATESHORTCUT)
    {
    CreateShortcut();
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
  CString initialDir = result;
  initialDir.Replace("/", "\\");
  CPathDialog dlg("Select Path", title, initialDir); 
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
    // save some values
    CString regvalue;

    // write the size of the dialog
    CRect size;
    this->GetWindowRect(&size);
    unsigned long width = size.Width();
    unsigned long height = size.Height();    
    RegSetValueEx(hKey, _T("Width"), 0, REG_DWORD, 
                  (CONST BYTE *)(&width),4);
    RegSetValueEx(hKey, _T("Height"), 0, REG_DWORD, 
                  (CONST BYTE *)(&height),4);

    this->ReadRegistryValue(hKey, &(regvalue),"WhereSource1","C:\\");
    int shiftEnd = 9;
    if(m_WhereSource != regvalue)
      {
      char keyName[1024];
      char keyName2[1024];
      int i;
      for (i = 2; i < 10; ++i)
        {
        regvalue = "";
        sprintf(keyName,"WhereSource%i",i);
        this->ReadRegistryValue(hKey, &(regvalue),keyName,"");
        // check for short circuit, if the new value is already in
        // the list then we stop
        if (m_WhereSource == regvalue)
          {
          shiftEnd = i - 1;
          }
        }
      
      for (i = shiftEnd; i; --i)
        {
        regvalue = "";
        sprintf(keyName,"WhereSource%i",i);
        sprintf(keyName2,"WhereSource%i",i+1);
        
        this->ReadRegistryValue(hKey, &(regvalue),keyName,"");
        if (strlen(regvalue))
          {
          RegSetValueEx(hKey, _T(keyName2), 0, REG_SZ, 
                        (CONST BYTE *)(const char *)regvalue, 
                        regvalue.GetLength());
          }
        }
      RegSetValueEx(hKey, _T("WhereSource1"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)m_WhereSource, 
                    m_WhereSource.GetLength());
      }
    
    this->ReadRegistryValue(hKey, &(regvalue),"WhereBuild1","C:\\");
    if(m_WhereBuild != regvalue)
      {
      int i;
      char keyName[1024];
      char keyName2[1024];
      for (i = 2; i < 10; ++i)
        {
        regvalue = "";
        sprintf(keyName,"WhereBuild%i",i);
        this->ReadRegistryValue(hKey, &(regvalue),keyName,"");
        // check for short circuit, if the new value is already in
        // the list then we stop
        if (m_WhereBuild == regvalue)
          {
          shiftEnd = i - 1;
          }
        }
      for (i = shiftEnd; i; --i)
        {
        regvalue = "";
        sprintf(keyName,"WhereBuild%i",i);
        sprintf(keyName2,"WhereBuild%i",i+1);
        
        this->ReadRegistryValue(hKey, &(regvalue),keyName,"");
        if (strlen(regvalue))
          {
          RegSetValueEx(hKey, _T(keyName2), 0, REG_SZ, 
                        (CONST BYTE *)(const char *)regvalue, 
                        regvalue.GetLength());
          }
        }
      RegSetValueEx(hKey, _T("WhereBuild1"), 0, REG_SZ, 
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
    DWORD dwSize = 4;
    DWORD width, height;
    
    if (RegQueryValueEx(hKey,_T("Width"), NULL, NULL, 
                        (BYTE *)&width, &dwSize) == ERROR_SUCCESS &&      
        RegQueryValueEx(hKey,_T("Height"), NULL, NULL, 
                        (BYTE *)&height, &dwSize) == ERROR_SUCCESS)
      {
      this->SetWindowPos(0,0,0,width,height,SWP_NOZORDER | SWP_NOMOVE);
      }

    if (m_WhereSource.IsEmpty()) 
      {
      this->ReadRegistryValue(hKey, &(m_WhereSource),"WhereSource1","C:\\");
      }
    if (m_WhereBuild.IsEmpty()) 
      {
      this->ReadRegistryValue(hKey, &(m_WhereBuild),"WhereBuild1","C:\\");
      }
    m_WhereSourceControl.AddString(m_WhereSource);
    m_WhereBuildControl.AddString(m_WhereBuild);

    char keyname[1024];
    CString regvalue;
    int i;
    for (i = 2; i <= 10; ++i)
      {
      sprintf(keyname,"WhereSource%i",i);
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),keyname,"C:\\");
      if (strcmp("C:\\",regvalue))
        {
        m_WhereSourceControl.AddString(regvalue);
        }
      sprintf(keyname,"WhereBuild%i",i);
      regvalue = "";
      this->ReadRegistryValue(hKey, &(regvalue),keyname,"C:\\");
      if (strcmp("C:\\",regvalue))
        {
        m_WhereBuildControl.AddString(regvalue);
        }
      }
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

void CMakeSetupDialog::RunCMake(bool generateProjectFiles)
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
  m_Cursor = LoadCursor(NULL, IDC_WAIT);
  ::SetCursor(m_Cursor);
  m_RunningConfigure = true;
  
  // get all the info from the dialog
  this->UpdateData();
  // always save the current gui values to disk
  this->SaveCacheFromGUI();
  // Make sure we are working from the cache on disk
  this->LoadCacheFromDiskToGUI(); 
  m_OKButton.EnableWindow(false);

  // setup the cmake instance
  if (generateProjectFiles)
    {
    if(m_CMakeInstance->Generate() != 0)
      {
      cmSystemTools::Error(
        "Error in generation process, project files may be invalid");
      }
    }
  else
    {
    m_CMakeInstance->SetHomeDirectory(m_WhereSource);
    m_CMakeInstance->SetStartDirectory(m_WhereSource);
    m_CMakeInstance->SetHomeOutputDirectory(m_WhereBuild);
    m_CMakeInstance->SetStartOutputDirectory(m_WhereBuild);
    m_CMakeInstance->SetGlobalGenerator(
      m_CMakeInstance->CreateGlobalGenerator(m_GeneratorDialog.m_GeneratorChoiceString));
    m_CMakeInstance->SetCMakeCommand(m_PathToExecutable);
    m_CMakeInstance->LoadCache();
    if(m_CMakeInstance->Configure() != 0)
      {
      cmSystemTools::Error(
        "Error in configuration process, project files may be invalid");
      }
    // update the GUI with any new values in the caused by the
    // generation process
    this->LoadCacheFromDiskToGUI();
    }

  // save source and build paths to registry
  this->SaveToRegistry();
  // path is up-to-date now
  m_BuildPathChanged = false;
  // put the cursor back 
  m_Cursor = LoadCursor(NULL, IDC_ARROW);
  ::SetCursor(m_Cursor);
  m_RunningConfigure = false;
  cmSystemTools::ResetErrorOccuredFlag();
}


// Callback for build projects button
void CMakeSetupDialog::OnConfigure() 
{
  if(!m_GeneratorPicked)
    {
    m_GeneratorDialog.m_CMakeInstance = this->m_CMakeInstance;
    if(m_GeneratorDialog.DoModal() != IDOK)
      {
      return;
      }
    // save the generator choice in the registry
    HKEY hKey;
    DWORD dwDummy;
    
    if(RegCreateKeyEx(HKEY_CURRENT_USER, 
                      m_RegistryKey,
                      0, "", REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, 
                      NULL, &hKey, &dwDummy) == ERROR_SUCCESS) 
      {
      // save some values
      RegSetValueEx(hKey, _T("LastGenerator"), 0, REG_SZ, 
                    (CONST BYTE *)(const char *)m_GeneratorDialog.m_GeneratorChoiceString, 
                    m_GeneratorDialog.m_GeneratorChoiceString.GetLength());
      }
    }
  
  // enable error messages each time configure is pressed
  cmSystemTools::EnableMessages();
  this->RunCMake(false);
}




// callback for combo box menu where build selection
void CMakeSetupDialog::OnSelendokWhereBuild() 
{
  m_WhereBuildControl.GetLBText(m_WhereBuildControl.GetCurSel(), 
                                m_WhereBuild);
  m_WhereBuildControl.SetWindowText( m_WhereBuild);
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

  // The build dir has changed, check if there is a cache, and 
  // grab the source dir from it

  std::string path = this->m_WhereBuild;
  cmSystemTools::ConvertToUnixSlashes(path);

  // adjust the cmake instance
  m_CMakeInstance->SetHomeOutputDirectory(m_WhereBuild);
  m_CMakeInstance->SetStartOutputDirectory(m_WhereBuild);

  std::string cache_file = path;
  cache_file += "/CMakeCache.txt";

  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  cmCacheManager::CacheIterator it = cachem->NewIterator();

  m_GeneratorPicked = false;

  // make sure we have a normal cache file, specifically if one exists make
  // sure it can be read
  if (cmSystemTools::FileExists(cache_file.c_str()))
    {
    if (cachem->LoadCache(path.c_str()))
      {
      if (it.Find("CMAKE_HOME_DIRECTORY"))
        {
        path = ConvertToWindowsPath(it.GetValue());
        this->m_WhereSource = path.c_str();
        this->m_WhereSourceControl.SetWindowText(this->m_WhereSource);
        this->OnChangeWhereSource();
        m_GeneratorPicked = true;
        }
      }
    else
      {
      //file exists but cqnnot be read
      cmSystemTools::Error("There is a CMakeCache.txt file for the current binary tree but cmake does not have permission to read it. Please check the permissions of the directory you are trying to run CMake on.");
      return;
      }
    }
  
  m_CacheEntriesList.RemoveAll();
  m_CacheEntriesList.ShowWindow(SW_SHOW);
  this->LoadCacheFromDiskToGUI();
  m_BuildPathChanged = true;
}


// copy from the cache manager to the cache edit list box
void CMakeSetupDialog::FillCacheGUIFromCacheManager()
{ 
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  cmCacheManager::CacheIterator it = cachem->NewIterator();
  size_t size = m_CacheEntriesList.GetItems().size();
  // if there are already entries in the cache, then
  // put the new ones in the top, so they show up first
  bool reverseOrder = false;
  // all the current values are not new any more
  std::set<CPropertyItem*> items = m_CacheEntriesList.GetItems();
  for(std::set<CPropertyItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    {
    // first check to see if it is still in the cache
    CPropertyItem* item = *i;
    if ( !it.Find((const char*)item->m_propName) )
      {
      m_CacheEntriesList.RemoveProperty((const char*)item->m_propName);
      }
    else
      {
      // if it is still in the cache then it is no longer new
      item->m_NewValue = false;
      }
    }
  for(cmCacheManager::CacheIterator i = cachem->NewIterator();
      !i.IsAtEnd(); i.Next())
    {
    const char* key = i.GetName();

    // if value has trailing space or tab, enclose it in single quotes
    // to enforce the fact that it has 'invisible' trailing stuff
    std::string value = i.GetValue();
    if (value.size() && 
        (value[value.size() - 1] == ' ' || 
         value[value.size() - 1] == '\t'))
      {
      value = '\'' + value +  '\'';
      }
    bool advanced = i.GetPropertyAsBool("ADVANCED");
    switch(i.GetType() )
      {
      case cmCacheManager::BOOL:
        if(cmSystemTools::IsOn(value.c_str()))
          {
          m_CacheEntriesList.AddProperty(key,
                                         "ON",
                                         i.GetProperty("HELPSTRING"),
                                         CPropertyList::COMBO,"ON|OFF",
                                         reverseOrder,
                                         advanced
            );
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         i.GetProperty("HELPSTRING"),
                                         CPropertyList::COMBO,"ON|OFF",
                                         reverseOrder, advanced
            );
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.c_str(),
                                       i.GetProperty("HELPSTRING"),
                                       CPropertyList::PATH,"",
                                       reverseOrder, advanced
          );
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.c_str(),
                                       i.GetProperty("HELPSTRING"),
                                       CPropertyList::FILE,"",
                                       reverseOrder, advanced
          );
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key,
                                       value.c_str(),
                                       i.GetProperty("HELPSTRING"),
                                       CPropertyList::EDIT,"",
                                       reverseOrder, advanced
          );
        break;
      case cmCacheManager::INTERNAL:
        m_CacheEntriesList.RemoveProperty(key);
        break;
      }
    }
  if(m_CacheEntriesList.GetShowAdvanced())
    {
    m_CacheEntriesList.ShowAdvanced();
    }
  else
    {
    m_CacheEntriesList.HideAdvanced();
    }
  
  m_OKButton.EnableWindow(false);
  if(cachem->GetSize() > 0 && !cmSystemTools::GetErrorOccuredFlag())
    {
    bool enable = true;
    items = m_CacheEntriesList.GetItems();
    for(std::set<CPropertyItem*>::iterator i = items.begin();
        i != items.end(); ++i)
      {
      CPropertyItem* item = *i;
      if(item->m_Advanced )
        {
        if(item->m_NewValue && m_CacheEntriesList.GetShowAdvanced())
          {
          enable = false;
          break;
          }
        }
      else
        {
        if(item->m_NewValue)
          {
          // if one new value then disable to OK button
          enable = false;
          break;
          }
        }
      }
    if(enable)
      {
      m_OKButton.EnableWindow(true);
      }
    }

  // redraw the list
  m_CacheEntriesList.SetTopIndex(0);
  m_CacheEntriesList.Invalidate();
}

// copy from the list box to the cache manager
void CMakeSetupDialog::FillCacheManagerFromCacheGUI()
{ 
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  std::set<CPropertyItem*> items = m_CacheEntriesList.GetItems();
  cmCacheManager::CacheIterator it = cachem->NewIterator();
  for(std::set<CPropertyItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    {
    CPropertyItem* item = *i; 
    if ( it.Find((const char*)item->m_propName) )
      {
      // if value is enclosed in single quotes ('foo') then remove them
      // they were used to enforce the fact that it had 'invisible' 
      // trailing stuff
      if (item->m_curValue.GetLength() >= 2 &&
          item->m_curValue[0] == '\'' && 
          item->m_curValue[item->m_curValue.GetLength() - 1] == '\'') 
        {
        it.SetValue(item->m_curValue.Mid(
          1, item->m_curValue.GetLength() - 2));
        }
      else
        {
        it.SetValue(item->m_curValue);
        }
      }
    }
}

  

//! Load cache file from m_WhereBuild and display in GUI editor
void CMakeSetupDialog::LoadCacheFromDiskToGUI()
{
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  if(m_WhereBuild != "")
    {
    if (!cachem->LoadCache(m_WhereBuild))
      {
      // if it does exist, but isn;t readable then warn the user
      std::string cacheFile = m_WhereBuild;
      cacheFile += "/CMakeCache.txt";
      if(cmSystemTools::FileExists(cacheFile.c_str()))
        {
        cmSystemTools::Error("There is a CMakeCache.txt file for the current binary tree but cmake does not have permission to read it. Please check the permissions of the directory you are trying to run CMake on.");
        return;
        }
      }
    cmCacheManager::CacheIterator itm = cachem->NewIterator();
    if ( itm.Find("CMAKE_HOME_DIRECTORY"))
      {
      std::string path = ConvertToWindowsPath(itm.GetValue());
      this->m_WhereSource = path.c_str();
      this->m_WhereSourceControl.SetWindowText(this->m_WhereSource);
      this->OnChangeWhereSource();
      }
    m_CMakeInstance->SetHomeDirectory(m_WhereSource);
    m_CMakeInstance->SetStartDirectory(m_WhereSource);
    m_CMakeInstance->SetHomeOutputDirectory(m_WhereBuild);
    m_CMakeInstance->SetStartOutputDirectory(m_WhereBuild);
    m_CMakeInstance->PreLoadCMakeFiles();
    this->FillCacheGUIFromCacheManager();
    cmCacheManager::CacheIterator it = 
      cachem->GetCacheIterator("CMAKE_GENERATOR");
    if(!it.IsAtEnd())
      {
      m_GeneratorPicked = true;
      std::string curGen = it.GetValue();
      if(m_GeneratorDialog.m_GeneratorChoiceString != curGen.c_str())
        {
        m_GeneratorDialog.m_GeneratorChoiceString = curGen.c_str();
        this->UpdateData(FALSE);
        }
      }
    }
}

//! Save GUI values to cmCacheManager and then save to disk.
void CMakeSetupDialog::SaveCacheFromGUI()
{
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  this->FillCacheManagerFromCacheGUI();
  if(m_WhereBuild != "")
    {
    cachem->SaveCache(m_WhereBuild);
    }
}


void CMakeSetupDialog::OnSize(UINT nType, int cx, int cy) 
{
  if (nType == SIZE_MINIMIZED)
    {
    CDialog::OnSize(nType, cx, cy);
    return;
    }  
  if (m_oldCX == -1)
    {
    m_oldCX = cx;
    m_oldCY = cy;
    }
  int deltax = cx - m_oldCX;
  int deltay = cy - m_oldCY;

  m_oldCX = cx;
  m_oldCY = cy;

  CDialog::OnSize(nType, cx, cy);

  if (deltax == 0 && deltay == 0)
    {
    return;
    }
  
  if(m_CacheEntriesList.m_hWnd)
    {
    // get the original sizes/positions
    CRect cRect;
    m_AdvancedValuesControl.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_AdvancedValuesControl.SetWindowPos(&wndTop, cRect.left + deltax, 
                                         cRect.top, 
                                         0, 0,
                                         SWP_NOCOPYBITS | 
                                         SWP_NOSIZE | SWP_NOZORDER);
    m_BrowseSource.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_BrowseSource.SetWindowPos(&wndTop, cRect.left + deltax, 
                                cRect.top, 
                                0, 0,
                                SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
    m_BrowseBuild.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_BrowseBuild.SetWindowPos(&wndTop, cRect.left + deltax, 
                               cRect.top, 
                               0, 0,
                               SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
    
    m_WhereSourceControl.GetWindowRect(&cRect);
    m_WhereSourceControl.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                                      cRect.Width() + deltax, 
                                      cRect.Height(), 
                                      SWP_NOCOPYBITS | 
                                      SWP_NOMOVE | SWP_NOZORDER);
    m_WhereBuildControl.GetWindowRect(&cRect);
    m_WhereBuildControl.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                                     cRect.Width() + deltax, 
                                     cRect.Height(), 
                                     SWP_NOCOPYBITS | 
                                     SWP_NOMOVE | SWP_NOZORDER);
    m_ListFrame.GetWindowRect(&cRect);
    m_ListFrame.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                             cRect.Width() + deltax, 
                             cRect.Height() + deltay, 
                             SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
    m_CacheEntriesList.GetWindowRect(&cRect);
    m_CacheEntriesList.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                                    cRect.Width() + deltax, 
                                    cRect.Height() + deltay, 
                                    SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);

    m_StatusDisplay.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_StatusDisplay.SetWindowPos(&wndBottom, cRect.left, 
                                 cRect.top + deltay, 
                                 cRect.Width() + deltax,  cRect.Height(),
                                 SWP_NOCOPYBITS);

    m_MouseHelp.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_MouseHelp.SetWindowPos(&wndTop, cRect.left , 
                             cRect.top + deltay, 
                             cRect.Width() +  deltax, cRect.Height(),
                             SWP_NOCOPYBITS | SWP_NOZORDER);
    
    deltax = int(deltax + m_deltaXRemainder);
    m_deltaXRemainder = float(deltax%2);


    m_Configure.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_Configure.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                                 cRect.top + deltay, 
                                 0, 0,
                                 SWP_NOCOPYBITS | SWP_NOSIZE);
    m_CancelButton.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_CancelButton.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                                cRect.top + deltay, 
                                0, 0,
                                SWP_NOCOPYBITS | SWP_NOSIZE);
    m_OKButton.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_OKButton.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                            cRect.top + deltay, 
                            0, 0,
                            SWP_NOCOPYBITS | SWP_NOSIZE);
    m_DeleteButton.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_DeleteButton.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                                cRect.top + deltay, 
                                0, 0,
                                SWP_NOCOPYBITS | SWP_NOSIZE);
    m_HelpButton.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_HelpButton.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                              cRect.top + deltay, 
                              0, 0,
                              SWP_NOCOPYBITS | SWP_NOSIZE);
    }
  
}


void CMakeSetupDialog::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
  lpMMI->ptMinTrackSize.x = 550;
  lpMMI->ptMinTrackSize.y = 272;
}

void CMakeSetupDialog::OnCancel()
{
  if(m_RunningConfigure)
    {
     if(MessageBox("You are in the middle of a Configure.\n"
                   "If you Cancel now the configure information will be lost.\n"
                   "Are you sure you want to Cancel?", "Confirm Exit",
                   MB_YESNO) == IDYES)
       {
       cmSystemTools::SetFatalErrorOccured();
       }
     return;
    }
  if(m_CacheEntriesList.IsDirty())
    {
    if(MessageBox("You have changed options but not rebuilt, "
                  "are you sure you want to exit?", "Confirm Exit",
                  MB_YESNO) == IDYES)
      {
      CDialog::OnOK();
      }
    }
  else
    {
    CDialog::OnOK();
    }
}

void CMakeSetupDialog::OnOk() 
{
  // enable error messages each time configure is pressed
  cmSystemTools::EnableMessages();
  m_CacheEntriesList.ClearDirty();
  this->RunCMake(true);

  // save the size of the dialog
  

  if (!(::GetKeyState(VK_SHIFT) & 0x1000))
    {
    CDialog::OnOK();
    }
}

// Create a shortcut on the desktop with the current Source/Build dir.
int CMakeSetupDialog::CreateShortcut() 
{
  // Find the desktop folder and create the link name

  HKEY hKey;
  if(RegOpenKeyEx(HKEY_CURRENT_USER, 
      "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
                  0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
    AfxMessageBox ("Create shortcut: unable to find 'Shell Folders' key in registry!");
    return 1;
    }
  
  DWORD dwType, dwSize;
#define MAXPATH 1024
  char link_name[MAXPATH];
  dwSize = MAXPATH;
  if(RegQueryValueEx(hKey, 
                     (LPCTSTR)"Desktop", 
                     NULL, 
                     &dwType, 
                     (BYTE *)link_name, 
                     &dwSize) != ERROR_SUCCESS)
    {
    AfxMessageBox ("Create shortcut: unable to find 'Desktop' registry value in 'Shell Folders' key!");
    return 1;
    }
  
  if(dwType != REG_SZ)
    {
    AfxMessageBox ("Create shortcut: 'Desktop' registry value in 'Shell Folders' key has wrong type!");
    return 1;
    }

  strcat(link_name, "\\CMake - ");
  std::string current_dir = cmSystemTools::GetFilenameName((LPCTSTR)m_WhereSource);
  strcat(link_name, current_dir.c_str());
  strcat(link_name, ".lnk");
  
  // Find the path to the current executable

  char path_to_current_exe[MAXPATH];
  ::GetModuleFileName(NULL, path_to_current_exe, MAXPATH);

  // Create the shortcut

  HRESULT hres;
  IShellLink *psl;

  // Initialize the COM library

  hres = CoInitialize(NULL);

  if (! SUCCEEDED (hres))
    {
    AfxMessageBox ("Create shortcut: unable to initialize the COM library!");
    return 1;
    }

  // Create an IShellLink object and get a pointer to the IShellLink 
  // interface (returned from CoCreateInstance).

  hres = CoCreateInstance(CLSID_ShellLink, 
                          NULL, 
                          CLSCTX_INPROC_SERVER,
                          IID_IShellLink, 
                          (void **)&psl);

  if (! SUCCEEDED (hres))
    {
    AfxMessageBox ("Create shortcut: unable to create IShellLink instance!");
    return 1;
    }

  IPersistFile *ppf;

  // Query IShellLink for the IPersistFile interface for 
  // saving the shortcut in persistent storage.

  hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);

  if (SUCCEEDED (hres))
    { 
    // Set the path to the shortcut target.
    hres = psl->SetPath(path_to_current_exe);

    if (! SUCCEEDED (hres))
      {
      AfxMessageBox ("Create shortcut: SetPath failed!");
      }
    
    // Set the arguments of the shortcut.
    CString args = " /H=\"" + m_WhereSource + "\" /B=\"" + m_WhereBuild + "\" /G=\"" + m_GeneratorDialog.m_GeneratorChoiceString + "\" /A=\"" + (m_AdvancedValues ? "TRUE" : "FALSE") + "\"";
    
    hres = psl->SetArguments(args);

    if (! SUCCEEDED (hres))
      {
      AfxMessageBox ("Create shortcut: SetArguments failed!");
      }
    
    // Set the description of the shortcut.
    hres = psl->SetDescription("Shortcut to CMakeSetup");
    
    if (! SUCCEEDED (hres))
      {
      AfxMessageBox ("Create shortcut: SetDescription failed!");
      }
    
    // Ensure that the string consists of ANSI characters.
    WORD wszAr[MAX_PATH]; 
    LPWSTR wsz = (LPWSTR)wszAr;
    MultiByteToWideChar(CP_ACP, 0, link_name, -1, (LPWSTR)(wsz), MAX_PATH);

    // Save the shortcut via the IPersistFile::Save member function.
    hres = ppf->Save(wsz, TRUE);

    if (! SUCCEEDED (hres))
      {
      AfxMessageBox ("Create shortcut: Save failed!");
      }
    
    // Release the pointer to IPersistFile.
    ppf->Release ();
    }
  // Release the pointer to IShellLink.
  psl->Release ();

  return 0;
}

void CMakeSetupDialog::OnHelpButton() 
{
  CMakeHelp dialog;
  dialog.DoModal();
}

void CMakeSetupDialog::OnDeleteButton() 
{
  std::string message = "Are you sure you want to delete the CMakeCache.txt file for:\n";
  message += m_WhereBuild;
  if(::MessageBox(0, message.c_str(), "Delete Cache?", 
                  MB_YESNO|MB_TASKMODAL) == IDNO)
    {
    return;
    }
  m_GeneratorPicked = false;

  if(m_WhereBuild != "" && this->m_CMakeInstance)
    {
    this->m_CMakeInstance->GetCacheManager()->DeleteCache(m_WhereBuild);
    }
  
  // Make sure we are working from the cache on disk
  this->LoadCacheFromDiskToGUI(); 
  
  m_OKButton.EnableWindow(false);
}

void CMakeSetupDialog::ShowAdvancedValues()
{
  m_CacheEntriesList.ShowAdvanced();
}

void CMakeSetupDialog::RemoveAdvancedValues()
{
  m_CacheEntriesList.HideAdvanced();
}


void CMakeSetupDialog::OnAdvancedValues() 
{
  this->UpdateData();
  if(m_AdvancedValues)
    {
    this->ShowAdvancedValues();
    }
  else
    {
    this->RemoveAdvancedValues();
    }
}

void CMakeSetupDialog::OnDoubleclickedAdvancedValues() 
{
  this->OnAdvancedValues();
}

// Handle param or single dropped file.
void CMakeSetupDialog::ChangeDirectoriesFromFile(const char* arg)
{
  // Check if the argument refers to a CMakeCache.txt or
  // CMakeLists.txt file.
  std::string listPath;
  std::string cachePath;
  bool argIsFile = false;
  if(cmSystemTools::FileIsDirectory(arg))
    {
    std::string path = cmSystemTools::CollapseFullPath(arg);
    cmSystemTools::ConvertToUnixSlashes(path);
    std::string cacheFile = path;
    cacheFile += "/CMakeCache.txt";
    std::string listFile = path;
    listFile += "/CMakeLists.txt";
    if(cmSystemTools::FileExists(cacheFile.c_str()))
      {
      cachePath = path;
      }
    if(cmSystemTools::FileExists(listFile.c_str()))
      {
      listPath = path;
      }
    }
  else if(cmSystemTools::FileExists(arg))
    {
    argIsFile = true;
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath.c_str());
    name = cmSystemTools::LowerCase(name);
    if(name == "cmakecache.txt")
      {
      cachePath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    else if(name == "cmakelists.txt")
      {
      listPath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    }
  
  // If there is a CMakeCache.txt file, use its settings.
  if(cachePath.length() > 0)
    {
    cmCacheManager* cachem = m_CMakeInstance->GetCacheManager();
    cmCacheManager::CacheIterator it = cachem->NewIterator();
    if(cachem->LoadCache(cachePath.c_str()) && it.Find("CMAKE_HOME_DIRECTORY"))
      {
      std::string path = ConvertToWindowsPath(cachePath.c_str());
      m_WhereBuild = path.c_str();
      
      path = ConvertToWindowsPath(it.GetValue());
      m_WhereSource = path.c_str();
      
      m_GeneratorDialog.m_GeneratorChoiceString = _T("");
      return;
      }
    }
  
  // If there is a CMakeLists.txt file, use it as the source tree.
  if(listPath.length() > 0)
    {
    std::string path = ConvertToWindowsPath(listPath.c_str());
    m_WhereSource = path.c_str();
    
    if(argIsFile)
      {
      // Source CMakeLists.txt file given.  It was probably dropped
      // onto the window or executable.  Default to an in-source
      // build.
      m_WhereBuild = path.c_str();
      }
    else
      {
      // Source directory given on command line.  Use current working
      // directory as build tree.
      std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
      path = ConvertToWindowsPath(cwd.c_str());
      m_WhereBuild = path.c_str();
      }
    }
}


// The framework calls this member function when the user releases the
// left mouse button over a window that has registered itself as the 
// recipient of dropped files. 

void CMakeSetupDialog::OnDropFiles(HDROP hDropInfo)
{
  UINT nb_files = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
  if (nb_files > 0)
    {
    UINT buffer_size = DragQueryFile(hDropInfo, 0, NULL, 0);
    char *buffer = new char [buffer_size + 1];
    DragQueryFile(hDropInfo, 0, buffer, buffer_size + 1);

    this->ChangeDirectoriesFromFile(buffer);
    delete [] buffer;

    this->m_WhereSourceControl.SetWindowText(this->m_WhereSource);
    this->m_WhereBuildControl.SetWindowText(this->m_WhereBuild);

    this->UpdateData(FALSE);

    this->OnChangeWhereSource();
    this->OnChangeWhereBuild();
    }

  DragFinish(hDropInfo);
}

BOOL CMakeSetupDialog::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
  CDialog::OnSetCursor(pWnd, nHitTest, message);
  if(m_Cursor == LoadCursor(NULL, IDC_WAIT))
    {
    ::SetCursor(m_Cursor);
    }
   return true;
}
