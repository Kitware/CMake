// MakeHelp.cpp : implementation file
//

#include "stdafx.h"
#include "CMakeSetup.h"
#include "MakeHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMakeHelp dialog


CMakeHelp::CMakeHelp(CWnd* pParent /*=NULL*/)
	: CDialog(CMakeHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMakeHelp)
	m_HelpMessage = _T("CMake is used to configure and generate build files for software projects.   The basic steps for configuring a project are as follows:\r\n\r\n1. Select the source directory for the project.  This should contain the CMakeLists.txt files for the project.\r\n\r\n2. Select the build directory for the project.   This is the directory where the project will be built.  It can be the same or a different directory than the source directory.   For easy clean up, a separate build directory is recommended.  CMake will create the directory if it does not exist.\r\n\r\n3. Once the source and binary directories are selected, it is time to press the Configure button.  This will cause CMake to read all of the input files and discover all the variables used by the project.   The first time a variable is displayed it will be in Red.   Users should inspect red variables making sure the values are correct.   For some projects the Configure process can be iterative, so continue to press the Configure button until there are no longer red entries.\r\n\r\n4. Once there are no longer red entries, you should click the OK button.  This will write the build files to the build directory and exit CMake.");
	//}}AFX_DATA_INIT
}


void CMakeHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMakeHelp)
	DDX_Text(pDX, IDC_EDIT1, m_HelpMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMakeHelp, CDialog)
	//{{AFX_MSG_MAP(CMakeHelp)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMakeHelp message handlers
