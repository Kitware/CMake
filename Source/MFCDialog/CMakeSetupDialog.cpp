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
#include "../cmCacheManager.h"
#include "../cmake.h"
#include "../cmGlobalGenerator.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Convert to Win32 path (slashes). But it's not in cmSystemTools, so 
// the 2 billions people that are using the CMake API can not mistake
// it with cmMakeMyCoffeeButNoSugarPlease().

std::string ConvertToWindowsPath(const char* path)
{
  // Convert to output path.
  // Remove the "" around it (if any) since it's an output path for
  // the shell. If another shell-oriented feature is not designed 
  // for a GUI use, then we are in trouble.

  std::string s = cmSystemTools::ConvertToOutputPath(path);
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
  return s;
}

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
  char tmp[1024];
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
}

CMakeSetupDialog::CMakeSetupDialog(const CMakeCommandLineInfo& cmdInfo,
                                   CWnd* pParent /*=NULL*/)
  : CDialog(CMakeSetupDialog::IDD, pParent)
{
  cmSystemTools::SetRunCommandHideConsole(true);
  cmSystemTools::SetErrorCallback(MFCMessageCallback);
  m_RegistryKey  = "Software\\Kitware\\CMakeSetup\\Settings\\StartPath";
  m_CacheEntriesList.m_CMakeSetupDialog = this;
  
  //{{AFX_DATA_INIT(CMakeSetupDialog)
	//}}AFX_DATA_INIT

  // Get the parameters from the command line info
  // If an unknown parameter is found, try to interpret it too, since it
  // is likely to be a file dropped on the shortcut :)
  if (cmdInfo.m_LastUnknownParameter.IsEmpty())
    {
    this->m_WhereSource = cmdInfo.m_WhereSource;
    this->m_WhereBuild = cmdInfo.m_WhereBuild;
    this->m_GeneratorChoiceString = cmdInfo.m_GeneratorChoiceString;
    this->m_AdvancedValues = cmdInfo.m_AdvancedValues;
    }
  else
    {
    this->m_WhereSource = _T("");
    this->m_WhereBuild = _T("");
    this->m_AdvancedValues = FALSE;
    this->m_GeneratorChoiceString = _T("");
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
  m_CMakeInstance = new cmake;
  m_CMakeInstance->SetProgressCallback(updateProgress, (void *)this);
  
}

void CMakeSetupDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMakeSetupDialog)
	DDX_Control(pDX, IDC_AdvancedValues, m_AdvancedValuesControl);
	DDX_Control(pDX, IDC_BuildForLabel, m_BuildForLabel);
	DDX_Control(pDX, IDC_BROWSE_SOURCE, m_BrowseSource);
	DDX_Control(pDX, IDC_BROWSE_BUILD, m_BrowseBuild);
	DDX_Control(pDX, IDC_HELP_BUTTON, m_HelpButton);
	DDX_Control(pDX, IDC_Generator, m_GeneratorChoice);
	DDX_Control(pDX, IDC_OK, m_OKButton);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_CBStringExact(pDX, IDC_WhereSource, m_WhereSource);
	DDX_CBStringExact(pDX, IDC_WhereBuild, m_WhereBuild);
	DDX_Control(pDX, IDC_FRAME, m_ListFrame);
	DDX_Control(pDX, IDC_WhereSource, m_WhereSourceControl);
	DDX_Control(pDX, IDC_WhereBuild, m_WhereBuildControl);
	DDX_Control(pDX, IDC_LIST2, m_CacheEntriesList);
	DDX_Control(pDX, IDC_MouseHelpCaption, m_MouseHelp);
	DDX_Control(pDX, IDC_CMAKE_VERSION, m_VersionDisplay);
	DDX_Control(pDX, IDC_BuildProjects, m_Configure);
	DDX_CBStringExact(pDX, IDC_Generator, m_GeneratorChoiceString);
	DDX_Check(pDX, IDC_AdvancedValues, m_AdvancedValues);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMakeSetupDialog, CDialog)
  //{{AFX_MSG_MAP(CMakeSetupDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_DROPFILES()
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
  ON_CBN_EDITCHANGE(IDC_Generator, OnEditchangeGenerator)
  ON_BN_CLICKED(IDC_HELP_BUTTON, OnHelpButton)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(IDC_AdvancedValues, OnAdvancedValues)
  ON_BN_DOUBLECLICKED(IDC_AdvancedValues, OnDoubleclickedAdvancedValues)
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
  SetIcon(m_hIcon, TRUE);			// Set big icon
  SetIcon(m_hIcon, FALSE);		// Set small icon
  // Load source and build dirs from registry
  this->LoadFromRegistry();
  std::vector<std::string> names;
  this->m_CMakeInstance->GetRegisteredGenerators(names);
  for(std::vector<std::string>::iterator i = names.begin();
      i != names.end(); ++i)
    {
    m_GeneratorChoice.AddString(i->c_str());
    }
  if (m_GeneratorChoiceString == _T("")) 
    {
    m_GeneratorChoiceString = "Visual Studio 6";
    }

  // try to load the cmake cache from disk
  this->LoadCacheFromDiskToGUI();
  m_WhereBuildControl.LimitText(2048);
  m_WhereSourceControl.LimitText(2048);
  m_GeneratorChoice.LimitText(2048);
    
  // Set the version number
  char tmp[1024];
  sprintf(tmp,"Version %d.%d - %s", cmake::GetMajorVersion(),
          cmake::GetMinorVersion(), cmake::GetReleaseVersion());
  SetDlgItemText(IDC_CMAKE_VERSION, tmp);
  SetDlgItemText(IDC_PROGRESS, "");
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
    // save some values
    CString regvalue;
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
  ::SetCursor(LoadCursor(NULL, IDC_WAIT));  

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
      m_CMakeInstance->CreateGlobalGenerator(m_GeneratorChoiceString));
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
  ::SetCursor(LoadCursor(NULL, IDC_ARROW));
  m_OKButton.EnableWindow(true);
  cmSystemTools::ResetErrorOccuredFlag();
}


// Callback for build projects button
void CMakeSetupDialog::OnConfigure() 
{
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
  if (cmSystemTools::FileExists(cache_file.c_str()) &&
      cachem->LoadCache(path.c_str()) &&
      it.Find("CMAKE_HOME_DIRECTORY"))
    {
    path = ConvertToWindowsPath(it.GetValue());
    this->m_WhereSource = path.c_str();
    this->m_WhereSourceControl.SetWindowText(this->m_WhereSource);
    this->OnChangeWhereSource();
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
  size_t size = m_CacheEntriesList.GetItems().size();
  bool reverseOrder = false;
  // if there are already entries in the cache, then
  // put the new ones in the top, so they show up first
  if(size)
    {
    reverseOrder = true;
    }
  
  // all the current values are not new any more
  std::set<CPropertyItem*> items = m_CacheEntriesList.GetItems();
  for(std::set<CPropertyItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    {
    CPropertyItem* item = *i;
    item->m_NewValue = false;
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

    if(!m_AdvancedValues)
      {
      if(i.GetPropertyAsBool("ADVANCED"))
        {
        m_CacheEntriesList.RemoveProperty(key);
        continue;
        }
      }
    switch(i.GetType() )
      {
      case cmCacheManager::BOOL:
        if(cmSystemTools::IsOn(value.c_str()))
          {
          m_CacheEntriesList.AddProperty(key,
                                         "ON",
                                         i.GetProperty("HELPSTRING"),
                                         CPropertyList::COMBO,"ON|OFF",
                                         reverseOrder 
            );
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         i.GetProperty("HELPSTRING"),
                                         CPropertyList::COMBO,"ON|OFF",
                                         reverseOrder
            );
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.c_str(),
                                       i.GetProperty("HELPSTRING"),
                                       CPropertyList::PATH,"",
                                       reverseOrder
          );
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, 
                                       value.c_str(),
                                       i.GetProperty("HELPSTRING"),
                                       CPropertyList::FILE,"",
                                       reverseOrder
          );
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key,
                                       value.c_str(),
                                       i.GetProperty("HELPSTRING"),
                                       CPropertyList::EDIT,"",
                                       reverseOrder
          );
        break;
      case cmCacheManager::INTERNAL:
	m_CacheEntriesList.RemoveProperty(key);
        break;
      }
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
      if(item->m_NewValue)
        {
        // if one new value then disable to OK button
        enable = false;
        break;
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
    cachem->LoadCache(m_WhereBuild);
    this->FillCacheGUIFromCacheManager();
    cmCacheManager::CacheIterator it = 
      cachem->GetCacheIterator("CMAKE_GENERATOR");
    if(!it.IsAtEnd())
      {
      std::string curGen = it.GetValue();
      if(m_GeneratorChoiceString != curGen.c_str())
        {
        m_GeneratorChoiceString = curGen.c_str();
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
                                 SWP_NOSIZE | SWP_NOZORDER);
    m_BuildForLabel.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_BuildForLabel.SetWindowPos(&wndTop, cRect.left + deltax, 
                                 cRect.top, 
                                 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);
    m_GeneratorChoice.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_GeneratorChoice.SetWindowPos(&wndTop, cRect.left + deltax, 
                                   cRect.top, 
                                   0, 0,
                                   SWP_NOSIZE | SWP_NOZORDER);
    m_BrowseSource.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_BrowseSource.SetWindowPos(&wndTop, cRect.left + deltax, 
                                cRect.top, 
                                0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);
    m_BrowseBuild.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_BrowseBuild.SetWindowPos(&wndTop, cRect.left + deltax, 
                                 cRect.top, 
                                 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);
    
    m_WhereSourceControl.GetWindowRect(&cRect);
    m_WhereSourceControl.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                                      cRect.Width() + deltax, 
                                      cRect.Height() + deltay, 
                                      SWP_NOMOVE | SWP_NOZORDER);
    m_WhereBuildControl.GetWindowRect(&cRect);
    m_WhereBuildControl.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                                      cRect.Width() + deltax, 
                                      cRect.Height() + deltay, 
                                      SWP_NOMOVE | SWP_NOZORDER);
    m_ListFrame.GetWindowRect(&cRect);
    m_ListFrame.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                             cRect.Width() + deltax, 
                             cRect.Height() + deltay, 
                             SWP_NOMOVE | SWP_NOZORDER);
    m_CacheEntriesList.GetWindowRect(&cRect);
    m_CacheEntriesList.SetWindowPos(&wndTop, cRect.left, cRect.top, 
                             cRect.Width() + deltax, 
                             cRect.Height() + deltay, 
                             SWP_NOMOVE | SWP_NOZORDER);
    m_VersionDisplay.SetWindowPos(&wndTop, 5, cy-23, 0, 0,
                                  SWP_NOSIZE | SWP_NOZORDER);

    deltax = int(deltax + m_deltaXRemainder);
    m_deltaXRemainder = float(deltax%2);
    m_MouseHelp.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_MouseHelp.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                             cRect.top + deltay, 
                             0, 0,
                             SWP_NOSIZE | SWP_NOZORDER);

    m_Configure.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_Configure.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                                 cRect.top + deltay, 
                                 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);
    m_CancelButton.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_CancelButton.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                                cRect.top + deltay, 
                                0, 0,
                                SWP_NOSIZE | SWP_NOZORDER);
    m_OKButton.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_OKButton.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                            cRect.top + deltay, 
                            0, 0,
                            SWP_NOSIZE | SWP_NOZORDER);
    m_HelpButton.GetWindowRect(&cRect);
    this->ScreenToClient(&cRect);
    m_HelpButton.SetWindowPos(&wndTop, cRect.left + deltax/2, 
                              cRect.top + deltay, 
                              0, 0,
                              SWP_NOSIZE | SWP_NOZORDER);
    }
  
}


void CMakeSetupDialog::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
  lpMMI->ptMinTrackSize.x = 550;
  lpMMI->ptMinTrackSize.y = 272;
}

void CMakeSetupDialog::OnCancel()
{
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
  if (!(::GetKeyState(VK_SHIFT) & 0x1000))
    {
    CDialog::OnOK();
    }
}

void CMakeSetupDialog::OnEditchangeGenerator() 
{
	// TODO: Add your control notification handler code here
	
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
    CString args = " /H=\"" + m_WhereSource + "\" /B=\"" + m_WhereBuild + "\" /G=\"" + m_GeneratorChoiceString + "\" /A=\"" + (m_AdvancedValues ? "TRUE" : "FALSE") + "\"";
    
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
    WORD wsz[MAX_PATH]; 
    MultiByteToWideChar(CP_ACP, 0, link_name, -1, wsz, MAX_PATH);

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

void CMakeSetupDialog::ShowAdvancedValues()
{
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  for(cmCacheManager::CacheIterator i = cachem->NewIterator();
      !i.IsAtEnd(); i.Next())
    {
    const char* key = i.GetName();
    if(!i.GetPropertyAsBool("ADVANCED"))
      {
      continue;
      }
    switch(i.GetType() )
      {
      case cmCacheManager::BOOL:
        if(cmSystemTools::IsOn(i.GetValue()))
          {
          m_CacheEntriesList.AddProperty(key,
                                         "ON",
                                         i.GetProperty("HELPSTRING"),
                                         CPropertyList::COMBO,"ON|OFF",
                                         true 
            );
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         i.GetProperty("HELPSTRING"),
                                         CPropertyList::COMBO,"ON|OFF",
                                         true
            );
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, 
				       i.GetValue(),
				       i.GetProperty("HELPSTRING"),
                                       CPropertyList::PATH,"",
                                       true
          );
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, 
                                       i.GetValue(),
				       i.GetProperty("HELPSTRING"),
                                       CPropertyList::FILE,"",
                                       true
          );
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key,
                                       i.GetValue(),
				       i.GetProperty("HELPSTRING"),
                                       CPropertyList::EDIT,"",
                                       true
          );
        break;
      case cmCacheManager::INTERNAL:
	m_CacheEntriesList.RemoveProperty(key);
        break;
      }
    }
}

void CMakeSetupDialog::RemoveAdvancedValues()
{
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  
  for(cmCacheManager::CacheIterator i = cachem->NewIterator();
      !i.IsAtEnd(); i.Next())
    {
    const char* key = i.GetName();
    if(i.GetPropertyAsBool("ADVANCED"))
      {
      m_CacheEntriesList.RemoveProperty(key);
      }
    }
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
// If the dropped file is a build directory or any file in a 
// build directory, set the source dir from the cache file,
// otherwise set the source and build dirs to this file (or dir).

void CMakeSetupDialog::ChangeDirectoriesFromFile(const char* buffer)
{
  // Get the path to this file

  std::string path = buffer;
  if (!cmSystemTools::FileIsDirectory(path.c_str()))
    {
    path = cmSystemTools::GetFilenamePath(path);
    }
  else
    {
    cmSystemTools::ConvertToUnixSlashes(path);
    }

  // Check if it's a build dir and grab the cache

  std::string cache_file = path;
  cache_file += "/CMakeCache.txt";

  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();

  cmCacheManager::CacheIterator it = 
    cachem->NewIterator();

  if (cmSystemTools::FileExists(cache_file.c_str()) &&
      cachem->LoadCache(path.c_str()) &&
      it.Find("CMAKE_HOME_DIRECTORY"))
    {
    path = ConvertToWindowsPath(path.c_str());
    this->m_WhereBuild = path.c_str();

    path = ConvertToWindowsPath(it.GetName());
    this->m_WhereSource = path.c_str();
    }
  else
    {
    path = ConvertToWindowsPath(path.c_str());
    this->m_WhereSource = this->m_WhereBuild = path.c_str();
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
