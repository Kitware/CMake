// CMakeSetupdialog.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CMakeSetup.h"
#include "CMakeSetupDialog.h"
#include "CMakeCommandLineInfo.h" 
#include "../cmListFileCache.h"
#include "../cmMakefileGenerator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMakeSetup

BEGIN_MESSAGE_MAP(CMakeSetup, CWinApp)
  //{{AFX_MSG_MAP(CMakeSetup)
  // NOTE - the ClassWizard will add and remove mapping macros here.
  //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG
  ON_COMMAND(ID_HELP, CWinApp::OnHelp)
  END_MESSAGE_MAP();


/////////////////////////////////////////////////////////////////////////////
// CMakeSetup construction
CMakeSetup::CMakeSetup()
{
  // TODO: add construction code here,
  // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMakeSetup object

CMakeSetup theApp;

/////////////////////////////////////////////////////////////////////////////
// CMakeSetup initialization

BOOL CMakeSetup::InitInstance()
{
  AfxEnableControlContainer();

  // Standard initialization
  // If you are not using these features and wish to reduce the size
  //  of your final executable, you should remove from the following
  //  the specific initialization routines you do not need.
#if _MFC_VER <= 0x421
#ifdef _AFXDLL
  Enable3dControls();			// Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif
  CMakeCommandLineInfo cmdInfo;
  ParseCommandLine(cmdInfo);

  CMakeSetupDialog dlg(cmdInfo);

  m_pMainWnd = &dlg;
  INT_PTR nResponse = dlg.DoModal();
  if (nResponse == IDOK)
    {
    // TODO: Place code here to handle when the dialog is
    //  dismissed with OK
    }
  else if (nResponse == IDCANCEL)
    {
    // TODO: Place code here to handle when the dialog is
    //  dismissed with Cancel
    }

  // clean up globals 
  cmListFileCache::GetInstance()->ClearCache(); 
  cmMakefileGenerator::UnRegisterGenerators();
  // Since the dialog has been closed, return FALSE so that we exit the
  //  application, rather than start the application's message pump.
  return FALSE;
}
