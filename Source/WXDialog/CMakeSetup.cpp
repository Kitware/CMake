/*=========================================================================

  Program:   WXDialog - wxWidgets X-platform GUI Front-End for CMake
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Author:    Jorgen Bodde

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "CMakeSetup.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <wx/filename.h>

#include "cmSystemTools.h"
#include "CommandLineInfo.h"

#include "CMakeSetup.h"
#include "CMakeSetupFrame.h"

////@begin XPM images
////@end XPM images

/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( CMakeSetupApp )
////@end implement app

/*!
 * CMakeSetupApp type definition
 */

IMPLEMENT_CLASS( CMakeSetupApp, wxApp )

/*!
 * CMakeSetupApp event table definition
 */

BEGIN_EVENT_TABLE( CMakeSetupApp, wxApp )

////@begin CMakeSetupApp event table entries
////@end CMakeSetupApp event table entries

END_EVENT_TABLE()

/*!
 * Constructor for CMakeSetupApp
 */

CMakeSetupApp::CMakeSetupApp()
{
////@begin CMakeSetupApp member initialisation
////@end CMakeSetupApp member initialisation
}

/*!
 * Initialisation for CMakeSetupApp
 */

bool CMakeSetupApp::OnInit()
{    
    cmSystemTools::DisableRunCommandOutput();

    // parse command line params
    cmCommandLineInfo cm;
    cm.SetValidArguments("ABGHQG");
    cm.ParseCommandLine(wxApp::argc, wxApp::argv);

    // set vendor name and app for config
    SetVendorName("Kitware");
    SetAppName("CMakeSetup");
        
    CMakeSetupFrm *MyFrame = new CMakeSetupFrm(NULL);
    
    // alternative app path way, somehow otherwise it does not work
    wxFileName fname(argv[0]);        
    MyFrame->DoInitFrame(cm, fname.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));
    MyFrame->Show(TRUE);

    return true;
}

/*!
 * Cleanup for CMakeSetupApp
 */
int CMakeSetupApp::OnExit()
{    
////@begin CMakeSetupApp cleanup
    return wxApp::OnExit();
////@end CMakeSetupApp cleanup
}

