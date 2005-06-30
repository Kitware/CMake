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
#pragma implementation "progressdlg.h"
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

#include "progressdlg.h"

////@begin XPM images
////@end XPM images

/*!
 * CMProgressDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMProgressDialog, wxDialog )

/*!
 * CMProgressDialog event table definition
 */

BEGIN_EVENT_TABLE( CMProgressDialog, wxDialog )

////@begin CMProgressDialog event table entries
    EVT_BUTTON( ID_CMAKE_BUTTON, CMProgressDialog::OnCmakeCancelClick )

////@end CMProgressDialog event table entries

END_EVENT_TABLE()

/*!
 * CMProgressDialog constructors
 */

CMProgressDialog::CMProgressDialog( )
{
}

CMProgressDialog::CMProgressDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CMProgressDialog creator
 */

bool CMProgressDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMProgressDialog member initialisation
    m_textMessage = NULL;
    m_progress = NULL;
////@end CMProgressDialog member initialisation

////@begin CMProgressDialog creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CMProgressDialog creation
    
    m_cancelPressed = false;
    m_cancelling = false;

    return TRUE;
}

/*!
 * Control creation for CMProgressDialog
 */

void CMProgressDialog::CreateControls()
{    
////@begin CMProgressDialog content construction
    CMProgressDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_textMessage = new wxStaticText( itemDialog1, wxID_STATIC, _("Please wait while CMake configures ..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_textMessage, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_progress = new wxGauge( itemDialog1, ID_CMAKE_PROGRESS, 100, wxDefaultPosition, wxSize(250, 20), wxGA_HORIZONTAL );
    m_progress->SetValue(0);
    itemBoxSizer5->Add(m_progress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_CMAKE_BUTTON, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

////@end CMProgressDialog content construction
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CMAKE_BUTTON
 */

void CMProgressDialog::OnCmakeCancelClick( wxCommandEvent& event )
{
    m_cancelPressed = true;
}

/*!
 * Should we show tooltips?
 */

bool CMProgressDialog::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMProgressDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMProgressDialog bitmap retrieval
    return wxNullBitmap;
////@end CMProgressDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMProgressDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMProgressDialog icon retrieval
    return wxNullIcon;
////@end CMProgressDialog icon retrieval
}



