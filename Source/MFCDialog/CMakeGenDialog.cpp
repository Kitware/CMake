// CMakeGenDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CMakeSetup.h"
#include "CMakeGenDialog.h"
#include "../cmake.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCMakeGenDialog dialog


CCMakeGenDialog::CCMakeGenDialog(CWnd* pParent /*=NULL*/)
  : CDialog(CCMakeGenDialog::IDD, pParent)
{
  //{{AFX_DATA_INIT(CCMakeGenDialog)
  //}}AFX_DATA_INIT
}


void CCMakeGenDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CCMakeGenDialog)
  DDX_Control(pDX, IDC_BuildForLabel, m_BuildForLabel);
  DDX_Control(pDX, IDC_Generator, m_GeneratorChoice);
  DDX_CBStringExact(pDX, IDC_Generator, m_GeneratorChoiceString);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCMakeGenDialog, CDialog)
  //{{AFX_MSG_MAP(CCMakeGenDialog)
  // NOTE: the ClassWizard will add message map macros here
  ON_CBN_EDITCHANGE(IDC_Generator, OnEditchangeGenerator)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMakeGenDialog message handler
  
  void CCMakeGenDialog::OnEditchangeGenerator() 
{
        // TODO: Add your control notification handler code here
        
}


BOOL CCMakeGenDialog::OnInitDialog()
{
  CDialog::OnInitDialog();
  std::vector<std::string> names;
  this->m_CMakeInstance->GetRegisteredGenerators(names);
  for(std::vector<std::string>::iterator i = names.begin();
      i != names.end(); ++i)
    {
    m_GeneratorChoice.AddString(i->c_str());
    }

  // we want to pick the best generator for their system first we check to
  // see if they have run cmake before, if so we use that generator
  std::string mp;
  bool done = false;
  
  // is the last generator set? If so use it
  mp = "[HKEY_CURRENT_USER\\Software\\Kitware"
    "\\CMakeSetup\\Settings\\StartPath;LastGenerator]";
  cmSystemTools::ExpandRegistryValues(mp);
  if(mp != "/registry")
    {
    m_GeneratorChoiceString = mp.c_str();
    done = true;
    }
  struct regToGen
  {
    const char* Registry;
    const char* GeneratorName;
  };
  regToGen installedGenerators[] = {
    // VS 9
    { "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft"
      "\\VisualStudio\\9.0\\Setup;Dbghelp_path]", "Visual Studio 9 2008"},
    // VS 8
    { "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft"
      "\\VisualStudio\\8.0\\Setup;Dbghelp_path]", "Visual Studio 8 2005"},
    // VS 7.1
    {"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft"
     "\\VisualStudio\\7.1;InstallDir]", "Visual Studio 7 .NET 2003"},
    // VS 7
    {"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft"
     "\\VisualStudio\\7.0;InstallDir]", "Visual Studio 7"},
    {0,0}
  };
  for(regToGen* ptr = installedGenerators; ptr->Registry != 0 && !done; ptr++)
    {
    mp = ptr->Registry;
    cmSystemTools::ExpandRegistryValues(mp);
    if(mp != "/registry")
      {
      m_GeneratorChoiceString = ptr->GeneratorName;
      done = true;
      }
    }
  
  // if still not done just guess on VS 6
  if (!done)
    {
    m_GeneratorChoiceString = "Visual Studio 6";
    }

  this->UpdateData(FALSE);
  
  return TRUE;  // return TRUE  unless you set the focus to a control
}

  
