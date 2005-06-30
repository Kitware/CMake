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
#pragma implementation "aboutdlg.h"
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

#include "cmake_logo.xpm"
#include "aboutdlg.h"

////@begin XPM images
////@end XPM images

/*!
 * CMAboutDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMAboutDlg, wxDialog )

/*!
 * CMAboutDlg event table definition
 */

BEGIN_EVENT_TABLE( CMAboutDlg, wxDialog )

////@begin CMAboutDlg event table entries
    EVT_BUTTON( ID_ABOUT_DLG_OK, CMAboutDlg::OnAboutDlgClick )

////@end CMAboutDlg event table entries

END_EVENT_TABLE()

/*!
 * CMAboutDlg constructors
 */

CMAboutDlg::CMAboutDlg( )
{
}

CMAboutDlg::CMAboutDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CMAboutDlg creator
 */

bool CMAboutDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMAboutDlg member initialisation
    m_cmIcon = NULL;
    m_html = NULL;
////@end CMAboutDlg member initialisation

////@begin CMAboutDlg creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CMAboutDlg creation

    // set logo on the dialog
    wxBitmap bmp(cmake_logo);
    m_cmIcon->SetBitmap(bmp);

    return TRUE;
}

/*!
 * Control creation for CMAboutDlg
 */

void CMAboutDlg::CreateControls()
{    
////@begin CMAboutDlg content construction
    CMAboutDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxBitmap m_cmIconBitmap(wxNullBitmap);
    m_cmIcon = new wxStaticBitmap( itemDialog1, wxID_STATIC, m_cmIconBitmap, wxDefaultPosition, wxSize(32, 32), 0 );
    itemFlexGridSizer3->Add(m_cmIcon, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxLEFT|wxRIGHT|wxTOP, 5);

    m_html = new wxHtmlWindow( itemDialog1, ID_HTMLWINDOW, wxDefaultPosition, wxSize(500, 300), wxHW_SCROLLBAR_AUTO|wxNO_BORDER|wxHSCROLL|wxVSCROLL );
    itemFlexGridSizer3->Add(m_html, 1, wxGROW|wxGROW|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, ID_ABOUT_DLG_OK, _("&Ok"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CMAboutDlg content construction
}

/*!
 * Should we show tooltips?
 */

bool CMAboutDlg::ShowToolTips()
{
    return TRUE;
}

void CMAboutDlg::SetAboutText(const wxString &cmversion, const wxString &cmsversion, const wxArrayString &generators)
{
    // set HTML text in window

    wxString text = _("<html><head><title>CMakeSetup - GUI Build System for CMake</title></head>"
                      "<body><font color=\"#000080\" size=\"+1\"><em><strong>CMakeSetup - GUI Build System for CMake</strong></em></font>"
                      "<p><font face=\"Arial, Helvetica, sans-serif\">Welcome to <b>CMake</b>! The cross-platform, open-source make system."
                      "<b>CMake</b> is used to control the software compilation process using simple platform and compiler independent configuration files. "
                      "<b>CMake</b> generates native makefiles and workspaces that can be used in the compiler environment of your choice. "
                      "Please go to <i><b>http://www.cmake.org</b></i> to learn more about CMake.<br><br>"
                      "CMakeSetup.exe is enhanced and ported by Jorgen Bodde using <i>@WXV@</i>. The original CMakeSetup.exe is "
                      "written by Bill Hoffman, Ken Martin, Brad King and Andy Cedilnik.<br><br>"
                      "Current CMakeSetup version is: <b>@B@</b><br>"
                      "Current CMake build system is: <b>@V@</b><br><br>"
                      "Current generators are supported:<br>"
                      "@G@</font></p></body></html>");

    // compile list of generators in a bulleted list
    wxString gens = _("<ul>");
    for(size_t i = 0; i < generators.Count(); i++)
        gens << "<li><u>" << generators[i] << "</u></li>";
    gens << _("</ul>");

    // replace stuff
    text.Replace(_("@V@"), cmversion, true);
    text.Replace(_("@B@"), cmsversion, true);
    text.Replace(_("@G@"), gens, true);
    text.Replace(_("@WXV@"), wxVERSION_STRING, true);

    m_html->SetPage(text);

    // set color of HTML window to bg window
    m_html->SetBackgroundColour(GetBackgroundColour());
}

/*!
 * Get bitmap resources
 */

wxBitmap CMAboutDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMAboutDlg bitmap retrieval
    return wxNullBitmap;
////@end CMAboutDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMAboutDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMAboutDlg icon retrieval
    return wxNullIcon;
////@end CMAboutDlg icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ABOUT_DLG_OK
 */

void CMAboutDlg::OnAboutDlgClick( wxCommandEvent& event )
{
    Close();
}


