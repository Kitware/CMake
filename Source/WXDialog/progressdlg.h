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

#ifndef _PROGRESSDLG_H_
#define _PROGRESSDLG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "progressdlg.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_PROGRESSDLG 10000
#define SYMBOL_CMPROGRESSDIALOG_STYLE wxRAISED_BORDER
#define SYMBOL_CMPROGRESSDIALOG_TITLE _("Progress Dialog")
#define SYMBOL_CMPROGRESSDIALOG_IDNAME ID_PROGRESSDLG
#define SYMBOL_CMPROGRESSDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CMPROGRESSDIALOG_POSITION wxDefaultPosition
#define ID_CMAKE_PROGRESS 10001
#define ID_CMAKE_BUTTON 10002
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
 * CMProgressDialog class declaration
 */

class CMProgressDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CMProgressDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMProgressDialog( );
    CMProgressDialog( wxWindow* parent, wxWindowID id = SYMBOL_CMPROGRESSDIALOG_IDNAME, const wxString& caption = SYMBOL_CMPROGRESSDIALOG_TITLE, const wxPoint& pos = SYMBOL_CMPROGRESSDIALOG_POSITION, const wxSize& size = SYMBOL_CMPROGRESSDIALOG_SIZE, long style = SYMBOL_CMPROGRESSDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMPROGRESSDIALOG_IDNAME, const wxString& caption = SYMBOL_CMPROGRESSDIALOG_TITLE, const wxPoint& pos = SYMBOL_CMPROGRESSDIALOG_POSITION, const wxSize& size = SYMBOL_CMPROGRESSDIALOG_SIZE, long style = SYMBOL_CMPROGRESSDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    void SetProgress(float progress) {
        m_progress->SetValue((int)(progress * 100));
    };

////@begin CMProgressDialog event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CMAKE_BUTTON
    void OnCmakeCancelClick( wxCommandEvent& event );

////@end CMProgressDialog event handler declarations

////@begin CMProgressDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMProgressDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    bool CancelPressed() const {
        return m_cancelPressed;
    };

    void CancelAcknowledged() {
        m_cancelling = true;
    };

    bool IsCancelling() const {
        return m_cancelling;
    };

    void ResetCancel() {
        m_cancelling = false;
        m_cancelPressed = false;
    };

////@begin CMProgressDialog member variables
    wxStaticText* m_textMessage;
    wxGauge* m_progress;
////@end CMProgressDialog member variables

private:
    bool m_cancelPressed;
    bool m_cancelling;

};

#endif
    // _PROGRESSDLG_H_
