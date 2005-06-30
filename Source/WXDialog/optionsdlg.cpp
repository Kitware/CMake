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
#pragma implementation "optionsdlg.h"
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

#include "optionsdlg.h"

////@begin XPM images
////@end XPM images

/*!
 * CMOptionsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMOptionsDlg, wxDialog )

/*!
 * CMOptionsDlg event table definition
 */

BEGIN_EVENT_TABLE( CMOptionsDlg, wxDialog )

////@begin CMOptionsDlg event table entries
    EVT_CHECKBOX( ID_CHECKBOX_CLOSECMAKE, CMOptionsDlg::OnButtonOK )

////@end CMOptionsDlg event table entries

END_EVENT_TABLE()

/*!
 * CMOptionsDlg constructors
 */

CMOptionsDlg::CMOptionsDlg( )
{
}

CMOptionsDlg::CMOptionsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CMOptionsDlg creator
 */

bool CMOptionsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMOptionsDlg member initialisation
    m_closeAfterGenerate = NULL;
////@end CMOptionsDlg member initialisation

////@begin CMOptionsDlg creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CMOptionsDlg creation
    return TRUE;
}

/*!
 * Control creation for CMOptionsDlg
 */

void CMOptionsDlg::CreateControls()
{    
////@begin CMOptionsDlg content construction
    CMOptionsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxNotebook* itemNotebook3 = new wxNotebook( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_TOP );

    wxPanel* itemPanel4 = new wxPanel( itemNotebook3, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    itemBoxSizer5->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_closeAfterGenerate = new wxCheckBox( itemPanel4, ID_CHECKBOX_CLOSECMAKE, _("Close down CMakeSetup after generation of project"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_closeAfterGenerate->SetValue(FALSE);
    itemBoxSizer5->Add(m_closeAfterGenerate, 0, wxALIGN_LEFT|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("General"));

    itemBoxSizer2->Add(itemNotebook3, 1, wxGROW|wxALL|wxFIXED_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CMOptionsDlg content construction
}

/*!
 * Should we show tooltips?
 */

bool CMOptionsDlg::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMOptionsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMOptionsDlg bitmap retrieval
    return wxNullBitmap;
////@end CMOptionsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMOptionsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMOptionsDlg icon retrieval
    return wxNullIcon;
////@end CMOptionsDlg icon retrieval
}

void CMOptionsDlg::SetConfig(wxConfig *cfg)
{
    bool boolval;
    
    // close after generation
    cfg->Read(CM_CLOSEAFTERGEN, &boolval, CM_CLOSEAFTERGEN_DEF);
    m_closeAfterGenerate->SetValue(boolval);
}

void CMOptionsDlg::GetConfig(wxConfig *cfg)
{
    // close after generation
    cfg->Write(CM_CLOSEAFTERGEN, m_closeAfterGenerate->GetValue());
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_CLOSECMAKE
 */

void CMOptionsDlg::OnButtonOK( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_CLOSECMAKE in CMOptionsDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_CLOSECMAKE in CMOptionsDlg. 
}






