// pcbuilderdialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CMakeSetup.h"
#include "CMakeSetupDialog.h"
#include "../cmDSWMakefile.h"
#include "../cmMSProjectGenerator.h"
#include "../cmCacheManager.h"
#include "../cmMakefile.h"
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
  m_WhereBuild = startPath;
  this->LoadFromRegistry();
  m_BuildPathChanged = false;
}

void CMakeSetupDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMakeSetupDialog)
	DDX_Control(pDX, IDC_LIST2, m_CacheEntriesList);
  DDX_Text(pDX, IDC_WhereSource, m_WhereSource);
  DDX_Text(pDX, IDC_WhereBuild, m_WhereBuild);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMakeSetupDialog, CDialog)
  //{{AFX_MSG_MAP(CMakeSetupDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_BUTTON2, OnBrowseWhereSource)
  ON_BN_CLICKED(IDC_BUTTON3, OnBrowseWhereBuild)
  ON_BN_CLICKED(IDC_BuildProjects, OnBuildProjects)
  ON_EN_CHANGE(IDC_WhereBuild, OnChangeWhereBuild)
  ON_EN_CHANGE(IDC_WhereSource, OnChangeWhereSource)
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
  this->LoadCacheFromDiskToGUI();
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


void CMakeSetupDialog::OnBrowseWhereSource() 
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

  bool bSuccess = (SHGetPathFromIDList(pidl, szPathName) ? true : false);
  if(bSuccess)
    {
    result = szPathName;
    }
  
  return bSuccess;
}


void CMakeSetupDialog::OnBrowseWhereBuild() 
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
  ::SetCursor(LoadCursor(NULL, IDC_WAIT));
  // get all the info from the screen
  this->UpdateData();
  if(!m_BuildPathChanged)
    {
    // if the build path has not changed save the 
    // current GUI values to the cache
    this->SaveCacheFromGUI();
    }
  // Make sure we are working from the cache on disk
  this->LoadCacheFromDiskToGUI();
  // Create a makefile object
  cmMakefile makefile;
  makefile.SetMakefileGenerator(new cmMSProjectGenerator);
  makefile.SetHomeDirectory(m_WhereSource);
  makefile.SetStartOutputDirectory(m_WhereBuild);
  makefile.SetHomeOutputDirectory(m_WhereBuild);
  makefile.SetStartDirectory(m_WhereSource);
  makefile.MakeStartDirectoriesCurrent();
  CString makefileIn = m_WhereSource;
  makefileIn += "/CMakeLists.txt";
  makefile.ReadListFile(makefileIn);
  // Generate the project files
  makefile.GenerateMakefile();
  // Save the cache
  cmCacheManager::GetInstance()->SaveCache(&makefile);
  // update the GUI with any new values in the caused by the
  // generation process
  this->LoadCacheFromDiskToGUI();
  // save source and build paths to registry
  this->SaveToRegistry();
  // path is not up-to-date
  m_BuildPathChanged = false;
  ::SetCursor(LoadCursor(NULL, IDC_ARROW));
}


 // copy from the cache manager to the cache edit list box
void CMakeSetupDialog::FillCacheGUIFromCacheManager()
{
  // Clear the current GUI
  m_CacheEntriesList.RemoveAll();
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
                                         CPropertyList::CHECKBOX,"");
          }
        else
          {
          m_CacheEntriesList.AddProperty(key,
                                         "OFF",
                                         CPropertyList::CHECKBOX,"");
          }
        break;
      case cmCacheManager::PATH:
        m_CacheEntriesList.AddProperty(key, value.m_Value.c_str(),
                                       CPropertyList::PATH,"");
        break;
      case cmCacheManager::FILEPATH:
        m_CacheEntriesList.AddProperty(key, value.m_Value.c_str(),
                                       CPropertyList::FILE,"");
        break;
      case cmCacheManager::STRING:
        m_CacheEntriesList.AddProperty(key, value.m_Value.c_str(),
                                       CPropertyList::EDIT,"");
        break;
      case cmCacheManager::INTERNAL:
        break;
      }
    }
  this->UpdateData(FALSE);
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
    // check to see if the editor has removed the cache entry
    if(item->m_Removed)
      {
      cmCacheManager::GetInstance()->RemoveCacheEntry((*i)->m_propName);
      }
    else
      {
	  cmCacheManager::CacheEntry *entry = 
      cmCacheManager::GetInstance()->GetCacheEntry((const char*)item->m_propName);
      if (entry)
        {
        entry->m_Value = item->m_curValue;
        }
      }
    }
}

  
  

void CMakeSetupDialog::OnChangeWhereBuild() 
{
  this->UpdateData();
  std::string cachefile = m_WhereBuild;
  cachefile += "/CMakeCache.txt";
  if(cmSystemTools::FileExists(cachefile.c_str()))
    {
    m_CacheEntriesList.ShowWindow(SW_SHOW);
    this->LoadCacheFromDiskToGUI();
    m_BuildPathChanged = true;
    }
  else
    {
    m_CacheEntriesList.RemoveAll();
    }
  
}

void CMakeSetupDialog::OnChangeWhereSource() 
{
  this->UpdateData();
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
