// CMakeSetupdialog.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CMakeSetup.h"
#include "CMakeSetupDialog.h"
#include "CMakeCommandLineInfo.h" 
#include "../cmDocumentation.h"
#include "../cmake.h"
#include "../cmSystemTools.h"


//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  CMakeSetup - CMake Windows GUI.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  CMakeSetup [options]\n"
   "  CMakeSetup [options] <path-to-source>\n"
   "  CMakeSetup [options] <path-to-existing-build>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"CMakeSetup\" executable is the CMake Windows GUI.  Project "
   "configuration settings may be specified interactively.  "
   "Brief instructions are provided at the bottom of the "
   "window when the program is running.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][3] =
{
  {"-A[on|off]", "Enable/disable display of advanced cache values.",
   "There are two categories of CMake cache values: non-advanced and "
   "advanced.  Most users will not need to change the advanced options.  "
   "The CMakeSetup GUI contains a checkbox to enable/disable display of "
   "advanced options.  This command line flag changes its default setting."},
  {0,0,0}
};

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
  Enable3dControls();                   // Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic();     // Call this when linking to MFC statically
#endif
#endif
  CMakeCommandLineInfo cmdInfo;
  ParseCommandLine(cmdInfo);
  cmSystemTools::FindExecutableDirectory(cmdInfo.GetArgV()[0]);

  // Check for documentation options.  If there are no arguments skip
  // the check because the GUI should be displayed instead of showing
  // usage in this case.
  cmDocumentation doc;
  if(cmdInfo.GetArgC() > 1 &&
     doc.CheckOptions(cmdInfo.GetArgC(), cmdInfo.GetArgV()))
    {
    // Construct and print requested documentation.
    cmake hcm;
    hcm.AddCMakePaths();
    doc.SetCMakeRoot(hcm.GetCacheDefinition("CMAKE_ROOT"));
    std::vector<cmDocumentationEntry> commands;
    std::vector<cmDocumentationEntry> compatCommands;
    std::map<std::string,cmDocumentationSection *> propDocs;

    std::vector<cmDocumentationEntry> generators;
    hcm.GetCommandDocumentation(commands, true, false);
    hcm.GetCommandDocumentation(compatCommands, false, true);
    hcm.GetGeneratorDocumentation(generators);
    hcm.GetPropertiesDocumentation(propDocs);
    doc.SetName("cmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.SetSection("Description",cmDocumentationDescription);
    doc.AppendSection("Generators",generators);
    doc.PrependSection("Options",cmDocumentationOptions);
    doc.SetSection("Commands",commands);
    doc.SetSection("Compatilbility Commands", compatCommands);
    doc.SetSections(propDocs);

    return (doc.PrintRequestedDocumentation(std::cout)? 0:1);
    }
  
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

  // Since the dialog has been closed, return FALSE so that we exit the
  //  application, rather than start the application's message pump.
  return FALSE;
}
