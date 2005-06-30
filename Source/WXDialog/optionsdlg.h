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

#ifndef _OPTIONSDLG_H_
#define _OPTIONSDLG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "optionsdlg.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/notebook.h"
////@end includes

#include <wx/config.h>
#include "config.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10004
#define SYMBOL_CMOPTIONSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CMOPTIONSDLG_TITLE _("CMakeSetup Options ...")
#define SYMBOL_CMOPTIONSDLG_IDNAME ID_DIALOG
#define SYMBOL_CMOPTIONSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CMOPTIONSDLG_POSITION wxDefaultPosition
#define ID_NOTEBOOK 10006
#define ID_PANEL 10007
#define ID_CHECKBOX_CLOSECMAKE 10008
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
 * CMOptionsDlg class declaration
 */

class CMOptionsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CMOptionsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMOptionsDlg( );
    CMOptionsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CMOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CMOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CMOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CMOPTIONSDLG_SIZE, long style = SYMBOL_CMOPTIONSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CMOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CMOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CMOPTIONSDLG_SIZE, long style = SYMBOL_CMOPTIONSDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMOptionsDlg event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_CLOSECMAKE
    void OnButtonOK( wxCommandEvent& event );

////@end CMOptionsDlg event handler declarations

////@begin CMOptionsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMOptionsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    // write values from config to GUI controls
    void SetConfig(wxConfig *cfg);

    // write GUI controls back to config
    void GetConfig(wxConfig *cfg);

private:

////@begin CMOptionsDlg member variables
    wxCheckBox* m_closeAfterGenerate;
////@end CMOptionsDlg member variables
};

#endif
    // _OPTIONSDLG_H_
