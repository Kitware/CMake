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

#ifndef _ABOUTDLG_H_
#define _ABOUTDLG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "aboutdlg.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/html/htmlwin.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxHtmlWindow;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SOME_ABOUTDLG 10003
#define SYMBOL_CMABOUTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CMABOUTDLG_TITLE _("About ...")
#define SYMBOL_CMABOUTDLG_IDNAME ID_SOME_ABOUTDLG
#define SYMBOL_CMABOUTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CMABOUTDLG_POSITION wxDefaultPosition
#define ID_HTMLWINDOW 10000
#define ID_ABOUT_DLG_OK 10005
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CMAboutDlg class declaration
 */

class CMAboutDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CMAboutDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMAboutDlg( );
    CMAboutDlg( wxWindow* parent, wxWindowID id = SYMBOL_CMABOUTDLG_IDNAME, const wxString& caption = SYMBOL_CMABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_CMABOUTDLG_POSITION, const wxSize& size = SYMBOL_CMABOUTDLG_SIZE, long style = SYMBOL_CMABOUTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMABOUTDLG_IDNAME, const wxString& caption = SYMBOL_CMABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_CMABOUTDLG_POSITION, const wxSize& size = SYMBOL_CMABOUTDLG_SIZE, long style = SYMBOL_CMABOUTDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    void SetAboutText(const wxString &cmversion, const wxString &cmsversion, const wxArrayString &generators);

////@begin CMAboutDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ABOUT_DLG_OK
    void OnAboutDlgClick( wxCommandEvent& event );

////@end CMAboutDlg event handler declarations

////@begin CMAboutDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMAboutDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CMAboutDlg member variables
    wxStaticBitmap* m_cmIcon;
    wxHtmlWindow* m_html;
////@end CMAboutDlg member variables
};

#endif
    // _ABOUTDLG_H_
