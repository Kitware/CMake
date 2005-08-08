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
#pragma implementation "CMakeSetupFrame.h"
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

#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>

#include "CMakeSetupFrame.h"
#include "PropertyList.h"
#include "app_resources.h"
#include "CMakeIcon.xpm"
#include "aboutdlg.h"

// cmake includes
#include "../cmListFileCache.h"
#include "../cmCacheManager.h"
#include "../cmGlobalGenerator.h"
#include "../cmDynamicLoader.h"

////@begin XPM images
////@end XPM images

/*!
 * CMakeSetupFrm type definition
 */

IMPLEMENT_CLASS( CMakeSetupFrm, wxFrame )

/*!
 * CMakeSetupFrm event table definition
 */

BEGIN_EVENT_TABLE( CMakeSetupFrm, wxFrame )

////@begin CMakeSetupFrm event table entries
    EVT_CLOSE( CMakeSetupFrm::OnCloseWindow )

    EVT_SPLITTER_SASH_POS_CHANGING( ID_SPLITTERWINDOW, CMakeSetupFrm::OnSplitterPosChanging )
    EVT_SPLITTER_DCLICK( ID_SPLITTERWINDOW, CMakeSetupFrm::OnSplitterwindowSashDClick )

    EVT_BUTTON( ID_BROWSE_PROJECT, CMakeSetupFrm::OnButtonBrowseProject )

    EVT_TEXT( ID_SOURCE_BUILD_PATH, CMakeSetupFrm::OnSourceBuildPathUpdated )
    EVT_TEXT_ENTER( ID_SOURCE_BUILD_PATH, CMakeSetupFrm::OnSourceBuildPathEnter )

    EVT_BUTTON( ID_BROWSE_BUILD, CMakeSetupFrm::OnButtonBrowseBuild )

    EVT_COMBOBOX( ID_SEARCHQUERY, CMakeSetupFrm::OnSearchquerySelected )
    EVT_TEXT( ID_SEARCHQUERY, CMakeSetupFrm::OnSearchqueryUpdated )

    EVT_CHECKBOX( ID_SHOW_ADVANCED, CMakeSetupFrm::OnShowAdvancedValues )

    EVT_GRID_CELL_CHANGE( CMakeSetupFrm::OnCellChange )
    EVT_GRID_SELECT_CELL( CMakeSetupFrm::OnGridSelectCell )
    EVT_MOTION( CMakeSetupFrm::OnPropertyMotion )

    EVT_BUTTON( ID_DO_CONFIGURE, CMakeSetupFrm::OnButtonConfigure )

    EVT_BUTTON( ID_DO_OK, CMakeSetupFrm::OnButtonOk )

    EVT_BUTTON( ID_DO_CANCEL, CMakeSetupFrm::OnButtonCancel )

    EVT_BUTTON( ID_DO_DELETE_CACHE, CMakeSetupFrm::OnButtonDeleteCache )

    EVT_BUTTON( ID_CLEAR_LOG, CMakeSetupFrm::OnClearLogClick )

    EVT_BUTTON( ID_BROWSE_GRID, CMakeSetupFrm::OnBrowseGridClick )

    EVT_MENU( ID_MENU_RELOAD_CACHE, CMakeSetupFrm::OnMenuReloadCacheClick )

    EVT_MENU( ID_MENU_DELETE_CACHE, CMakeSetupFrm::OnMenuDeleteCacheClick )

    EVT_MENU( ID_MENU_QUIT, CMakeSetupFrm::OnMenuQuitClick )

    EVT_MENU( ID_MENU_CONFIGURE, CMakeSetupFrm::OnMenuConfigureClick )

    EVT_MENU( ID_MENU_EXITGENERATE, CMakeSetupFrm::OnMenuGenerateClick )

    EVT_MENU( ID_MENU_TOGGLE_ADVANCED, CMakeSetupFrm::OnMenuToggleAdvancedClick )

    EVT_MENU( ID_CMAKE_OPTIONS, CMakeSetupFrm::OnOptionsClick )

    EVT_MENU( ID_ABOUTDLG, CMakeSetupFrm::OnAboutClick )

////@end CMakeSetupFrm event table entries

    EVT_MENU_RANGE(CM_RECENT_BUILD_ITEM, CM_RECENT_BUILD_ITEM + CM_MAX_RECENT_PATHS, CMakeSetupFrm::OnRecentFileMenu)   

    EVT_TEXT_ENTER(ID_SEARCHQUERY, CMakeSetupFrm::OnAddQuery )

END_EVENT_TABLE()

/** Callback function for CMake generator, to tell user how
    far the generation actually is */
void updateProgress(const char *msg, float prog, void *cd)
{
    // TODO: Make some kind of progress counter 
    
    CMakeSetupFrm *fm = (CMakeSetupFrm *)cd;

    if(fm)
    {
        if(prog < 0)
            fm->LogMessage(0, msg);
        else
        {
            fm->UpdateProgress(prog);
            fm->IssueUpdate();
        }
    }
}

/** Callback function for CMake generator, to tell user about stuff. This should be
    logged in the m_log window */
void MFCMessageCallback(const char* m, const char* title, bool& nomore, void *clientdata)
{ 
    CMakeSetupFrm *fm = (CMakeSetupFrm *)clientdata;

    if(fm)
    {
        wxString what = m, msg;
        if(what.StartsWith("CMake Error: "))
            fm->LogMessage(-1, m);
        else
            fm->LogMessage(1, m);
    }
}

// Convert to Win32 path (slashes). This calls the system tools one and then
// removes the spaces. It is not in system tools because we don't want any
// generators accidentally use it
std::string ConvertToWindowsPath(const char* path)
{
  // Convert to output path.
  // Remove the "" around it (if any) since it's an output path for
  // the shell. If another shell-oriented feature is not designed 
  // for a GUI use, then we are in trouble.
  // save the value of the force to unix path option
  bool saveForce = cmSystemTools::GetForceUnixPaths();
  // make sure we get windows paths no matter what for the GUI
  cmSystemTools::SetForceUnixPaths(false);
  std::string s = cmSystemTools::ConvertToOutputPath(path);
  // now restore the force unix path to its previous value
  cmSystemTools::SetForceUnixPaths(saveForce);
  if (s.size())
    {
    std::string::iterator i = s.begin();
    if (*i == '\"')
      {
      s.erase(i, i + 1);
      }
    i = s.begin() + s.length() - 1;
    if (*i == '\"')
      {
      s.erase(i, i + 1);
      }
    }
  return s;
}


bool DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    size_t nFiles = filenames.GetCount();

    // only one item allowed
    if(nFiles > 1)
        return false;

    if(nFiles == 1)
    {
        // only one dir allowed
        if(!wxDirExists(filenames[0]))
            return false;

        // strip the seperator
        wxFileName name;
        name.AssignDir(filenames[0]);       
        
        // issue a 'drop' by changing text ctrl
        m_pOwner->SetValue(name.GetFullPath());

        return true;
    }

    return false;
}

/*!
 * CMakeSetupFrm constructors
 */

CMakeSetupFrm::CMakeSetupFrm( )
    : m_cmake(0)
{
}

CMakeSetupFrm::CMakeSetupFrm( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_cmake(0)
{
    Create( parent, id, caption, pos, size, style );
}

/*!
 * CMakeSetupFrm creator
 */

bool CMakeSetupFrm::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMakeSetupFrm member initialisation
    m_splitter = NULL;
    m_cmProjectPath = NULL;
    m_BrowseProjectPathButton = NULL;
    m_cmBuildPath = NULL;
    m_BrowseSourcePathButton = NULL;
    m_cmGeneratorChoice = NULL;
    m_cmSearchQuery = NULL;
    m_cmShowAdvanced = NULL;
    m_cmOptions = NULL;
    m_cmLog = NULL;
    m_cmDescription = NULL;
    m_ConfigureButton = NULL;
    m_OkButton = NULL;
    m_CancelButton = NULL;
    m_DeleteCacheButton = NULL;
    m_ClearLogButton = NULL;
    m_cmBrowseCell = NULL;
////@end CMakeSetupFrm member initialisation

    wxFrame::Create( parent, id, caption, pos, size, style );

    // make sure the developer does not assign more then 100
    // would be rediculous but also overlap other id's
    wxASSERT(CM_MAX_RECENT_PATHS < 100);

    m_ExitTimer = 0;

    m_progressDlg = 0;
    m_noRefresh = false;
    m_quitAfterGenerating = false;

    m_config = new wxConfig("CMakeSetup");

    wxIcon icon(CMakeIcon_xpm);
    SetIcon(icon);

    CreateControls();
    
    //SetIcon(GetIconResource(wxT("cmake_icon.xpm")));
    //SetIcon(wxIcon("NGDialog.ico", wxBITMAP_TYPE_ICO_RESOURCE));
    Centre();

    // is it needed to hide console?
    m_RunningConfigure = false;
    cmSystemTools::SetRunCommandHideConsole(true);
    cmSystemTools::SetErrorCallback(MFCMessageCallback, (void *)this);

    // create our cmake instance
    m_cmake = new cmake;
    m_cmake->SetProgressCallback(updateProgress, (void *)this);

    return TRUE;
}

CMakeSetupFrm::~CMakeSetupFrm()
{
    wxString str;

    // write configs back to disk
    m_config->Write(CM_LASTPROJECT_PATH, m_cmProjectPath->GetValue());
    m_config->Write(CM_LASTBUILD_PATH, m_cmBuildPath->GetValue());

    // clear the config first
    for(size_t i = 0 ; i < CM_MAX_RECENT_PATHS; i++)
    {
        str.Printf("%s%i", _(CM_RECENT_BUILD_PATH), i);
        m_config->Write(str, _(""));
    }

    // write the last CM_MAX_RECENT_PATHS items back to config
    int i = (m_recentPaths.Count() >= CM_MAX_RECENT_PATHS ? CM_MAX_RECENT_PATHS : m_recentPaths.Count());
    while(i > 0)
    {
        str.Printf("%s%i", _(CM_RECENT_BUILD_PATH), i);
        m_config->Write(str, m_recentPaths[i - 1]);
        i--;
    }
    
    // write recent query list to config
    for(int j = 0; j < m_cmSearchQuery->GetCount(); j++)
    {
        // allow max to be written
        if(j < CM_MAX_SEARCH_QUERIES)
        {
            str.Printf("%s%i", _(CM_SEARCH_QUERY), j);
            m_config->Write(str, m_cmSearchQuery->GetString(j));
        }
        else
            break;
    }

    // set window pos + size in settings
    if(!IsIconized() && !IsMaximized())
    {
        int xsize, ysize;
        GetSize(&xsize, &ysize);
        if(xsize > 0 && ysize > 0) 
        {
            m_config->Write(CM_XSIZE, (long)xsize);
            m_config->Write(CM_YSIZE, (long)ysize);
        }

        if(m_splitter->GetSashPosition() > 0)
            m_config->Write(CM_SPLITTERPOS, (long)m_splitter->GetSashPosition());

        GetPosition(&xsize, &ysize);
        if(xsize != 0 && ysize != 0) 
        {
            m_config->Write(CM_XPOS, (long)xsize);
            m_config->Write(CM_YPOS, (long)ysize);
        }
    }

    // write changes (will be done before deletion)
    delete m_config;
    
    // delete timer
    if(m_ExitTimer)
        delete m_ExitTimer;

    // delete our cmake instance again
    if(m_cmake)
        delete m_cmake;
}

void CMakeSetupFrm::UpdateWindowState()
{
    bool dogenerate = !m_RunningConfigure && !m_cmOptions->IsCacheDirty() && 
                       (m_cmOptions->GetCount() != 0);
    
    // when configure is running, disable a lot of stuff
    m_cmProjectPath->Enable(!m_RunningConfigure);
    m_BrowseProjectPathButton->Enable(!m_RunningConfigure);
    m_cmBuildPath->Enable(!m_RunningConfigure);
    m_BrowseSourcePathButton->Enable(!m_RunningConfigure);
    m_cmGeneratorChoice->Enable(!m_RunningConfigure);
    m_cmShowAdvanced->Enable(!m_RunningConfigure);
    m_cmOptions->Enable(!m_RunningConfigure);
    m_ConfigureButton->Enable(!m_RunningConfigure);
    m_OkButton->Enable(dogenerate);
    m_CancelButton->Enable(m_RunningConfigure);
    m_DeleteCacheButton->Enable(!m_RunningConfigure);
    m_ClearLogButton->Enable(!m_RunningConfigure);
    if(m_RunningConfigure)
        m_cmBrowseCell->Enable(false);

    // when cache loaded (items available show other control)
    m_cmGeneratorChoice->Enable(m_cmOptions->GetCount() == 0 && !m_RunningConfigure);
    m_cmSearchQuery->Enable(!m_RunningConfigure);
    m_cmBrowseCell->Enable(!m_RunningConfigure && m_cmOptions->IsSelectedItemBrowsable());

    // disable the menus when configuring
    if(GetMenuBar())
    {
        // disable configure button when there is nothing, and generate and exit
        // only when it is allowed to generate
        GetMenuBar()->Enable(ID_MENU_EXITGENERATE, dogenerate);
        GetMenuBar()->Enable(ID_MENU_CONFIGURE, !m_RunningConfigure);

        for(size_t i = 0; i < GetMenuBar()->GetMenuCount(); i++)
            GetMenuBar()->EnableTop(i, !m_RunningConfigure); 
    }
}

void CMakeSetupFrm::LogMessage(int logkind, const char *msg)
{
    // put CR first but prevent a CR at the end
#ifndef __LINUX__   
    if(m_cmLog->IsModified())
        (*m_cmLog) << wxT("\n");
#else
    // Linux requires a different approach
    if(!m_cmLog->GetValue().IsEmpty())
        (*m_cmLog) << wxT("\n");
#endif        

    // log error, warning, or message
    wxTextAttr defattr = m_cmLog->GetDefaultStyle();
    
    switch(logkind)
    {
    // user message
    case 1:
        {
            wxTextAttr colattr(*wxBLUE);
            m_cmLog->SetDefaultStyle(colattr);
            (*m_cmLog) << msg;
            m_cmLog->SetDefaultStyle(defattr);
        }
        break;

    // progress
    case 0:
        (*m_cmLog) << msg;
        break;

    // error
    case -1:
        {
            wxTextAttr colattr(*wxRED);
            m_cmLog->SetDefaultStyle(colattr);
            (*m_cmLog) << msg;
            m_cmLog->SetDefaultStyle(defattr);
        }
        break;
    }
        
    IssueUpdate();
}

void CMakeSetupFrm::IssueUpdate()
{
    //::wxSafeYield(m_CancelButton, true);
    ::wxYield();

    // when we pressed cancel on the progress dialog
    // stop all activities
    if(m_progressDlg)
    {
        if(m_progressDlg->CancelPressed() && !m_progressDlg->IsCancelling())
        {           
            m_progressDlg->CancelAcknowledged();

            // send a button event to cancel the progress
            wxCommandEvent event( wxEVT_COMMAND_BUTTON_CLICKED, ID_DO_CANCEL);
            wxPostEvent(this, event);
        }
    }
}

/*!
 * Control creation for CMakeSetupFrm
 */

void CMakeSetupFrm::CreateControls()
{    
////@begin CMakeSetupFrm content construction
    CMakeSetupFrm* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu37 = new wxMenu;
    itemMenu37->Append(ID_MENU_RELOAD_CACHE, _("&Reload Cache\tCtrl+R"), _("Reload the cache from disk"), wxITEM_NORMAL);
    itemMenu37->Append(ID_MENU_DELETE_CACHE, _("&Delete Cache\tCtrl+D"), _("Delete the cache on disk of the current path"), wxITEM_NORMAL);
    itemMenu37->AppendSeparator();
    itemMenu37->Append(ID_MENU_QUIT, _("E&xit\tAlt+F4"), _("Quit CMake Setup"), wxITEM_NORMAL);
    menuBar->Append(itemMenu37, _("&File"));
    wxMenu* itemMenu42 = new wxMenu;
    itemMenu42->Append(ID_MENU_CONFIGURE, _("&Configure\tCtrl+N"), _T(""), wxITEM_NORMAL);
    itemMenu42->Append(ID_MENU_EXITGENERATE, _("&Generate and Exit\tCtrl+G"), _T(""), wxITEM_NORMAL);
    itemMenu42->Append(ID_MENU_TOGGLE_ADVANCED, _("Toggle &Advanced\tCtrl+A"), _T(""), wxITEM_NORMAL);
    itemMenu42->AppendSeparator();
    itemMenu42->Append(ID_CMAKE_OPTIONS, _("&Options\tCtrl+O"), _T(""), wxITEM_NORMAL);
    menuBar->Append(itemMenu42, _("&Tools"));
    wxMenu* itemMenu48 = new wxMenu;
    itemMenu48->Append(ID_ABOUTDLG, _("&About ..."), _("Shows the about dialog ..."), wxITEM_NORMAL);
    menuBar->Append(itemMenu48, _("&Help"));
    itemFrame1->SetMenuBar(menuBar);

    m_splitter = new wxSplitterWindow( itemFrame1, ID_SPLITTERWINDOW, wxDefaultPosition, wxSize(100, 100), wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER );

    wxPanel* itemPanel3 = new wxPanel( m_splitter, ID_MAINPANEL, wxDefaultPosition, wxSize(600, 400), wxNO_BORDER|wxTAB_TRAVERSAL );
    itemPanel3->SetExtraStyle(itemPanel3->GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxGROW|wxTOP|wxBOTTOM, 5);
    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 3, 0, 0);
    itemFlexGridSizer6->AddGrowableRow(1);
    itemFlexGridSizer6->AddGrowableCol(1);
    itemBoxSizer5->Add(itemFlexGridSizer6, 1, wxALIGN_TOP|wxLEFT, 5);
    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel3, wxID_STATIC, _("CMake project path"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_cmProjectPath = new wxTextCtrl( itemPanel3, ID_PROJECT_PATH, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer6->Add(m_cmProjectPath, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    m_BrowseProjectPathButton = new wxButton( itemPanel3, ID_BROWSE_PROJECT, _("Browse"), wxDefaultPosition, wxSize(55, -1), 0 );
    itemFlexGridSizer6->Add(m_BrowseProjectPathButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel3, wxID_STATIC, _("Project build path"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_cmBuildPath = new wxTextCtrl( itemPanel3, ID_SOURCE_BUILD_PATH, _T(""), wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer6->Add(m_cmBuildPath, 1, wxGROW|wxALIGN_TOP|wxTOP|wxBOTTOM, 5);

    m_BrowseSourcePathButton = new wxButton( itemPanel3, ID_BROWSE_BUILD, _("Browse"), wxDefaultPosition, wxSize(55, -1), 0 );
    itemFlexGridSizer6->Add(m_BrowseSourcePathButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer5->Add(itemBoxSizer13, 0, wxGROW|wxLEFT|wxRIGHT, 5);
    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer13->Add(itemFlexGridSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);
    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel3, wxID_STATIC, _("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxString* m_cmGeneratorChoiceStrings = NULL;
    m_cmGeneratorChoice = new wxComboBox( itemPanel3, ID_CHOOSE_GENERATOR, _T(""), wxDefaultPosition, wxSize(170, -1), 0, m_cmGeneratorChoiceStrings, wxCB_READONLY );
    itemFlexGridSizer14->Add(m_cmGeneratorChoice, 1, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxTOP|wxBOTTOM, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemPanel3, wxID_STATIC, _("Search"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer14->Add(itemStaticText17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxString* m_cmSearchQueryStrings = NULL;
    m_cmSearchQuery = new wxComboBox( itemPanel3, ID_SEARCHQUERY, _T(""), wxDefaultPosition, wxSize(170, -1), 0, m_cmSearchQueryStrings, wxWANTS_CHARS );
    itemFlexGridSizer14->Add(m_cmSearchQuery, 1, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxTOP|wxBOTTOM, 5);

    m_cmShowAdvanced = new wxCheckBox( itemPanel3, ID_SHOW_ADVANCED, _("Show advanced values"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_cmShowAdvanced->SetValue(FALSE);
    itemBoxSizer13->Add(m_cmShowAdvanced, 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5);

    m_cmOptions = new wxPropertyList( itemPanel3, ID_OPTIONS, wxDefaultPosition, wxSize(200, 150), wxSTATIC_BORDER|wxWANTS_CHARS|wxVSCROLL );
    m_cmOptions->SetDefaultColSize(250);
    m_cmOptions->SetDefaultRowSize(25);
    m_cmOptions->SetColLabelSize(20);
    m_cmOptions->SetRowLabelSize(0);
    m_cmOptions->CreateGrid(10, 2, wxGrid::wxGridSelectRows);
    itemBoxSizer4->Add(m_cmOptions, 1, wxGROW|wxALL, 5);

    wxPanel* itemPanel21 = new wxPanel( m_splitter, ID_LOGPANEL, wxDefaultPosition, wxSize(-1, 100), wxNO_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemPanel21->SetSizer(itemBoxSizer22);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer22->Add(itemBoxSizer23, 1, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);
    m_cmLog = new wxTextCtrl( itemPanel21, ID_LOG_AREA, _("Select your project path (where CMakeLists.txt is) and then select the build path (where the projects should be saved), or select a previous build path.\n\nRight click on a cache value for additional options (delete and ignore). Press configure to update and display new values in red, press OK to generate the projects and exit."), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2|wxSTATIC_BORDER );
    itemBoxSizer23->Add(m_cmLog, 1, wxGROW|wxRIGHT, 5);

    m_cmDescription = new wxTextCtrl( itemPanel21, ID_DESCRIPTION, _T(""), wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2|wxSTATIC_BORDER );
    itemBoxSizer23->Add(m_cmDescription, 0, wxGROW|wxLEFT, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer22->Add(itemBoxSizer26, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_ConfigureButton = new wxButton( itemPanel21, ID_DO_CONFIGURE, _("Co&nfigure"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ConfigureButton->SetDefault();
    itemBoxSizer26->Add(m_ConfigureButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OkButton = new wxButton( itemPanel21, ID_DO_OK, _("&Generate!"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_OkButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CancelButton = new wxButton( itemPanel21, ID_DO_CANCEL, _("C&ancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#if defined(__WXMSW__)
    wxStaticLine* itemStaticLine30 = new wxStaticLine( itemPanel21, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer26->Add(itemStaticLine30, 0, wxGROW|wxALL, 5);
#endif

    m_DeleteCacheButton = new wxButton( itemPanel21, ID_DO_DELETE_CACHE, _("&Delete Cache"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_DeleteCacheButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ClearLogButton = new wxButton( itemPanel21, ID_CLEAR_LOG, _("Clear &Log"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_ClearLogButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#if defined(__WXMSW__)
    wxStaticLine* itemStaticLine33 = new wxStaticLine( itemPanel21, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer26->Add(itemStaticLine33, 0, wxGROW|wxALL, 5);
#endif

    m_cmBrowseCell = new wxButton( itemPanel21, ID_BROWSE_GRID, _("&Browse"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_cmBrowseCell, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_splitter->SplitHorizontally(itemPanel3, itemPanel21, 300);

    wxStatusBar* itemStatusBar35 = new wxStatusBar( itemFrame1, ID_STATUSBAR, wxST_SIZEGRIP|wxNO_BORDER );
    itemStatusBar35->SetFieldsCount(2);
    itemFrame1->SetStatusBar(itemStatusBar35);

////@end CMakeSetupFrm content construction
}

void CMakeSetupFrm::DoInitFrame(cmCommandLineInfo &cm, const wxString &fn)
{ 
    // path to where cmake.exe is
    // m_PathToExecutable = cm.GetPathToExecutable().c_str();
    m_PathToExecutable = fn;

    // adjust size of last bar, to display % progress
    wxStatusBar *bar = GetStatusBar();
    if(bar)
    {
        wxASSERT(bar->GetFieldsCount() > 1);
        
        // fill all with -1. Why this way? because the count of the status bars
        // can change. All of the widths must be accounted for and initialised
        int *widths = new int[bar->GetFieldsCount()];
        for(int i = 0; i < bar->GetFieldsCount(); i++)
            widths[i] = -1;

        // the % field
        widths[1] = 75;
        bar->SetStatusWidths(bar->GetFieldsCount(), widths);
        delete widths;
    }

    wxString name, generator;
    std::vector<std::string> names;
  
    m_RunningConfigure = false;

    // set grid labels
    m_cmOptions->SetColLabelValue(0, wxT("Cache Name"));
    m_cmOptions->SetColLabelValue(1, wxT("Cache Value"));
    m_cmOptions->SetProjectGenerated(false);

    // set drop target
    m_cmOptions->SetDropTarget(new DnDFile(m_cmBuildPath));

    m_cmake->GetRegisteredGenerators(names);
    for(std::vector<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        name = i->c_str();
        m_cmGeneratorChoice->Append(name);
    }
    
    // sync advanced option with grid
    m_cmOptions->SetShowAdvanced(m_cmShowAdvanced->GetValue());

    // if we have a command line query that a generator 
    // needs to be chosen instead of the default, take it
    bool foundGivenGenerator = false;
    if(!cm.m_GeneratorChoiceString.IsEmpty())
    {
        // set proper discovered generator
        foundGivenGenerator = m_cmGeneratorChoice->SetStringSelection(cm.m_GeneratorChoiceString);  
    }

    // if none selected, we will see if VS8, VS7 or VS6 is present
    if(!foundGivenGenerator || m_cmGeneratorChoice->GetValue().IsEmpty())
    {
        std::string mp;
        mp = "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup;Dbghelp_path]";
        cmSystemTools::ExpandRegistryValues(mp);
        if(mp != "/registry")
            generator = wxT("Visual Studio 8 2005");
        else
        {
            mp = "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\7.1;InstallDir]";
            cmSystemTools::ExpandRegistryValues(mp);
            if (mp != "/registry")
                generator = wxT("Visual Studio 7 .NET 2003");
            else
            {
                mp = "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\7.0;InstallDir]";
                cmSystemTools::ExpandRegistryValues(mp);
                if (mp != "/registry")
                    generator = wxT("Visual Studio 7");
                else
                    generator = wxT("Visual Studio 6");         
            }
        }
    }

    // set proper discovered generator
    m_cmGeneratorChoice->SetStringSelection(generator);
    
    wxString str;
    //str.Printf("CMake %d.%d - %s", cmake::GetMajorVersion(), cmake::GetMinorVersion(), cmake::GetReleaseVersion());
    str.Printf("CMakeSetup v%i.%i%s", CMAKEGUI_MAJORVER, CMAKEGUI_MINORVER, CMAKEGUI_ADDVER);

    SetTitle(str);
    wxString path;
    
    // get last 5 used projects
    for(size_t i = 0; i < CM_MAX_RECENT_PATHS; i++)
    {
        path.Printf("%s%i", _(CM_RECENT_BUILD_PATH), i);
        if(m_config->Read(path, &str))
            AppendPathToRecentList(str);
    }

    // get query items
    for(size_t i = 0; i < CM_MAX_SEARCH_QUERIES; i++)
    {
        path.Printf("%s%i", _(CM_SEARCH_QUERY), i);
        if(m_config->Read(path, &str))
            m_cmSearchQuery->Append(str);
    }


    // make sure the call to update grid is not executed
    m_noRefresh = true;
    m_cmSearchQuery->SetValue(_(""));
    m_noRefresh = false;

    // Get the parameters from the command line info
    // If an unknown parameter is found, try to interpret it too, since it
    // is likely to be a file dropped on the shortcut :)
    bool sourceDirLoaded = false,
         buildDirLoaded = false;
    
    if(cm.m_LastUnknownParameter.empty())
    {
        if(cm.m_WhereSource.size() > 0 )
        {
            m_cmProjectPath->SetValue(cm.m_WhereSource.c_str());
            sourceDirLoaded = true;
        }   
    
        if (cm.m_WhereBuild.size() > 0 )
        {
            m_cmBuildPath->SetValue(cm.m_WhereBuild.c_str());
            buildDirLoaded = true;
        }
            
        m_cmShowAdvanced->SetValue(cm.m_AdvancedValues);
    }
    else
    {
        m_cmShowAdvanced->SetValue(false);
        
        // TODO: Interpret directory from dropped shortcut
        //this->ChangeDirectoriesFromFile(cmdInfo->m_LastUnknownParameter.c_str());
    }

    if (cm.m_ExitAfterLoad)
    {
        int id = GetId();
        m_ExitTimer = new wxTimer(this, id);
        m_ExitTimer->Start(3000);

        Connect( id, wxEVT_TIMER,(wxObjectEventFunction) &CMakeSetupFrm::OnExitTimer ); 

    } 

    // retrieve settings, this needs to be done here
    // because writing to the m_cmBuildPath triggers a cache reload
    if(!sourceDirLoaded && m_config->Read(CM_LASTPROJECT_PATH, &str))
        m_cmProjectPath->SetValue(str);

    if(!buildDirLoaded)
    {
        m_cmOptions->RemoveAll();
        if(m_config->Read(CM_LASTBUILD_PATH, &str))
            m_cmBuildPath->SetValue(str);
    }

    // set window size from settings
    long xsize, ysize, splitpos;
    if(m_config->Read(CM_XSIZE, &xsize) && m_config->Read(CM_YSIZE, &ysize) &&
       m_config->Read(CM_SPLITTERPOS, &splitpos))
    {
        SetSize(xsize, ysize);
        m_splitter->SetSashPosition(splitpos);
    }

    if(m_config->Read(CM_XPOS, &xsize) && m_config->Read(CM_YPOS, &ysize))
        SetSize(xsize, ysize, -1, -1, wxSIZE_USE_EXISTING);

    UpdateWindowState();
}

void CMakeSetupFrm::LoadCacheFromDiskToGUI()
{
    wxString builddir = m_cmBuildPath->GetValue();  
    
    cmCacheManager *cachem = m_cmake->GetCacheManager();
    if(cachem && !builddir.Trim().IsEmpty())
    {
        if(cachem->LoadCache(builddir.c_str()))
            AppendPathToRecentList(builddir);

        // represent this cache in the grid, but not before we
        // wiped all of the old items
        FillCacheGUIFromCacheManager();
    
        // set the generator string to the one used in the cache
        cmCacheManager::CacheIterator it = cachem->GetCacheIterator("CMAKE_GENERATOR");
        if(!it.IsAtEnd())
        {
            wxString curGen = it.GetValue();
            m_cmGeneratorChoice->SetStringSelection(curGen);
        }
    }
}

void CMakeSetupFrm::AppendPathToRecentList(const wxString &p)
{
    wxFileName path;
    wxString str;

    if(p.IsEmpty())
        return;
    
    // cheap way to get rid of trailing seperators
    path.AssignDir(p);
    str = path.GetPath();

    // append the item, or add it to end to make sure
    // it is remembered between sessions
    for(size_t i = 0; i < m_recentPaths.Count(); i++)
    {
        if(m_recentPaths[i].IsSameAs(str, false))
        {
            m_recentPaths.RemoveAt(i);
            
            // only re-add when item is still valid
            if(::wxDirExists(str))
                m_recentPaths.Add(str);
            else
                return;  // no add when the item is not existing

            return;
        }
    }
 
    if(GetMenuBar())
    {
        // get file menu
        int lastUsedID = 0;
        wxMenu *mnu = GetMenuBar()->GetMenu(0);
        wxASSERT(mnu != 0);
        
        if(::wxDirExists(str))
        {
            // add to array
            if(m_recentPaths.Count() == 0)
                mnu->AppendSeparator();

            lastUsedID = CM_RECENT_BUILD_ITEM + m_recentPaths.Count();
            m_recentPaths.Add(str);

            // when we have more in list then we can display, prune and 
            // remove some menu items until we have room (and available ID's again)
            if(m_recentPaths.Count() > CM_MAX_RECENT_PATHS)
            {
                // prune the list
                while(m_recentPaths.Count() > CM_MAX_RECENT_PATHS)
                    m_recentPaths.RemoveAt(0);  

                // now determine count, and remove until we have room
                int index = mnu->GetMenuItemCount() - 1;
                int count = 0;
                wxASSERT(index > 0);
                
                wxMenuItem *item;
                do
                {
                    item = mnu->FindItemByPosition(index);
                    if(item)
                    {
                        if(item->IsSeparator())
                        {
                            // next index is valid item
                            index ++;
                            break;
                        }
                        else
                            count ++;
                    }

                    index --;
                }
                while(index >= 0 && item);

                // ok, if count > CM_MAX_RECENT_PATHS then we are going to
                // delete some items on the index position
                if(count >= CM_MAX_RECENT_PATHS)
                {
                    // delete items that are exceeding
                    while(count >= CM_MAX_RECENT_PATHS)
                    {
                        lastUsedID = mnu->FindItemByPosition(index)->GetId();
                        mnu->Delete(lastUsedID);
                        count --;
                    }
                }
            }

            // append item
            mnu->Append(lastUsedID, str);
        }
    }
}

bool CMakeSetupFrm::PerformCacheRun()
{
    bool enable = false;
    cmCacheManager *cachem = m_cmake->GetCacheManager();
    cmCacheManager::CacheIterator it = cachem->NewIterator();

    // remove all items that are no longer present
    size_t j = 0;
    while(j < m_cmOptions->GetCount())
    {
        // check to see if it is still in the CMake cache
        // if it is still in the cache then it is no longer new
        wxPropertyItem *item = m_cmOptions->GetItem(j);
        if ( !it.Find((const char*)item->GetPropName().c_str()) )
            m_cmOptions->RemoveProperty(item);
        else
        {
            // ok we found it, mark as old
            item->SetNewValue(false);
            int row = m_cmOptions->FindProperty(item);
            if(row != -1) 
                m_cmOptions->UpdatePropertyItem(item, row);
            j++;
        }
    }

    if(cachem->GetSize() > 0 && !cmSystemTools::GetErrorOccuredFlag())
    {
        bool enable = true;     
        for(size_t i = 0; i < m_cmOptions->GetCount(); i++)
        {
            wxPropertyItem* item = m_cmOptions->GetItem(i);
            if(item->GetAdvanced())
            {
                if(item->GetNewValue() && m_cmOptions->GetShowAdvanced())
                {
                    // if one new value then disable to OK button
                    enable = false;
                    break;
                }
            }
            else
            {
                if(item->GetNewValue())
                {
                    // if one new value then disable to OK button
                    enable = false;
                    break;
                }
            }
        }
    }

    return enable;  
}

void CMakeSetupFrm::FillCacheGUIFromCacheManager()
{
    cmCacheManager *cachem = m_cmake->GetCacheManager();
    cmCacheManager::CacheIterator it = cachem->NewIterator();
  
    // remove all items that are no longer present
    size_t j = 0;
    while(j < m_cmOptions->GetCount())
    {
        // check to see if it is still in the CMake cache
        // if it is still in the cache then it is no longer new
        wxPropertyItem *item = m_cmOptions->GetItem(j);
        if ( !it.Find((const char*)item->GetPropName().c_str()) )
            m_cmOptions->RemoveProperty(item);
        else
            j++;
    }

    // if there are already entries in the cache, then
    // put the new ones in the top, so they show up first
    bool reverseOrder = false;
    for(cmCacheManager::CacheIterator i = cachem->NewIterator(); !i.IsAtEnd(); i.Next())
    {
        const char* key = i.GetName();

        // if value has trailing space or tab, enclose it in single quotes
        // to enforce the fact that it has 'invisible' trailing stuff
        std::string value = i.GetValue();
        if (value.size() && (value[value.size() - 1] == ' ' ||  value[value.size() - 1] == '\t'))
            value = '\'' + value +  '\'';

        bool advanced = i.GetPropertyAsBool("ADVANCED");
        switch(i.GetType() )
        {
        case cmCacheManager::BOOL:
            {
                wxString OnOff;
                
                if(cmSystemTools::IsOn(value.c_str()))
                    OnOff = wxT("ON");
                else
                    OnOff = wxT("OFF");

                m_cmOptions->AddProperty(key,
                                         OnOff.c_str(),
                                         i.GetProperty("HELPSTRING"),
                                         wxPropertyList::CHECKBOX, "ON|OFF",
                                         reverseOrder,
                                         advanced );
            }
            break;

        case cmCacheManager::PATH:
            m_cmOptions->AddProperty(key, 
                                     value.c_str(),
                                     i.GetProperty("HELPSTRING"),
                                     wxPropertyList::PATH,"",
                                     reverseOrder, advanced);
            break;
        
        case cmCacheManager::FILEPATH:
            m_cmOptions->AddProperty(key, 
                                     value.c_str(),
                                     i.GetProperty("HELPSTRING"),
                                     wxPropertyList::FILE,"",
                                     reverseOrder, advanced);
            break;
      
        case cmCacheManager::STRING:
            m_cmOptions->AddProperty(key,
                                     value.c_str(),
                                     i.GetProperty("HELPSTRING"),
                                     wxPropertyList::EDIT,"",
                                     reverseOrder, advanced);
            break;
      
        case cmCacheManager::INTERNAL:
            {
                wxPropertyItem *pItem = m_cmOptions->FindPropertyByName(key);
                if(pItem)
                    m_cmOptions->RemoveProperty(pItem);
            }
            break;          
        }
    }
}

void CMakeSetupFrm::OnExitTimer(wxTimerEvent &event)
{
    Close();
} 

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BROWSE_PROJECT
 */

void CMakeSetupFrm::OnButtonBrowseProject( wxCommandEvent& event )
{
    const wxString& dir = wxDirSelector("Select project directory", m_cmProjectPath->GetValue());
    if(!dir.IsEmpty())
        m_cmProjectPath->SetValue(dir);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BROWSE_BUILD
 */

void CMakeSetupFrm::OnButtonBrowseBuild( wxCommandEvent& event )
{
    const wxString& dir = wxDirSelector("Select build directory", m_cmBuildPath->GetValue());
    if(!dir.IsEmpty())
        m_cmBuildPath->SetValue(dir);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SHOW_ADVANCED
 */

void CMakeSetupFrm::OnShowAdvancedValues( wxCommandEvent& event )
{
    if(m_cmShowAdvanced->GetValue())
        m_cmOptions->ShowAdvanced();
    else
        m_cmOptions->HideAdvanced();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_CONFIGURE
 */

void CMakeSetupFrm::OnButtonConfigure( wxCommandEvent& event )
{
    DoConfigure();
}

void CMakeSetupFrm::DoConfigure()
{
    // enable error messages each time configure is pressed
    cmSystemTools::EnableMessages();
    m_cmOptions->HideControls();

    cmSystemTools::ResetErrorOccuredFlag(); 
    
    // instantiate a dialog for the progress meter

    PerformCacheRun();
    RunCMake(false);
}

int CMakeSetupFrm::RunCMake(bool generateProjectFiles)
{
    int value = -1;
    
    // clear log
    m_cmLog->Clear();
    m_cmLog->DiscardEdits();

    wxString builddir = m_cmBuildPath->GetValue(), 
             sourcedir = m_cmProjectPath->GetValue(),
             err = wxT("Error in configuration process, project files may be invalid");


    // sanity check for people pressing OK on empty dirs
    if(builddir.Trim().IsEmpty() || sourcedir.Trim().IsEmpty())
    {
        wxMessageBox(wxT("Please enter a valid source directory and build directory"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return -1;
    }

    // check if the directory exists, if not, create it
    if(!cmSystemTools::FileExists(builddir.c_str()))
    {
        wxString str;
        str << wxT("Build directory does not exist, should I create it?\n\nDirectory: ") << builddir;
    
        int answer = wxMessageBox(str, wxT("Create directory"), wxYES_NO, this);
        if (answer == wxYES)
        {
            if(!cmSystemTools::MakeDirectory(builddir.c_str()))
            {
                // could not create, tell and abort
                wxMessageBox(wxT("Could not create directory"), wxT("Error"), wxOK | wxICON_ERROR, this);
                return -1;
            }
        }
        else
        {
            // we abort because the user did not want to make the directory
            wxMessageBox(wxT("Build Project aborted, nothing done."), wxT("Aborted"), 
                         wxOK | wxICON_EXCLAMATION, this);
            return -1;
        }
    }

    /** show progress dialog that informs the user with a progress bar */ 
    if(m_progressDlg)
        m_progressDlg->Destroy();

    m_progressDlg = new CMProgressDialog(this);
    m_progressDlg->Show();

    // set the wait cursor
    m_RunningConfigure = true;
    UpdateWindowState();

    // always save the current gui values to disk
    SaveCacheFromGUI();
  
    // Make sure we are working from the cache on disk
    LoadCacheFromDiskToGUI(); 

    // setup the cmake instance
    if (generateProjectFiles)
    {
        if(m_cmake->Generate() != 0)
        {
            wxMessageBox(err, wxT("Error"), wxOK | wxICON_ERROR, this);
            cmSystemTools::Error(err.c_str());
            value = -1;
        }
        else
        {
            value = 0;
            m_cmOptions->SetProjectGenerated(true); // clear cache dirty when generated
        }
    }
    else
    {
        // set paths 
        m_cmake->SetHomeDirectory(m_cmProjectPath->GetValue().c_str());
        m_cmake->SetStartDirectory(m_cmProjectPath->GetValue().c_str());
        m_cmake->SetHomeOutputDirectory(m_cmBuildPath->GetValue().c_str());
        m_cmake->SetStartOutputDirectory(m_cmBuildPath->GetValue().c_str());
        
        m_cmake->SetGlobalGenerator(m_cmake->CreateGlobalGenerator(m_cmGeneratorChoice->GetValue().c_str()));
        m_cmake->SetCMakeCommand(m_PathToExecutable.c_str());
        m_cmake->LoadCache();
        if(m_cmake->Configure() != 0)
        {
            wxMessageBox(err, wxT("Error"), wxOK | wxICON_ERROR, this);
            cmSystemTools::Error(err.c_str());
        }
        
        // update the GUI with any new values in the caused by the
        // generation process
        LoadCacheFromDiskToGUI();
    }

    m_RunningConfigure = false;
    
    if(!value)
        cmSystemTools::ResetErrorOccuredFlag();

    m_progressDlg->Destroy();
    m_progressDlg = 0;

    // reset the statusbar progress 
    wxStatusBar *bar = GetStatusBar();
    if(bar)
        bar->SetStatusText(wxEmptyString, 1);

    UpdateWindowState();
    return value;
}

//! Save GUI values to cmCacheManager and then save to disk.
void CMakeSetupFrm::SaveCacheFromGUI()
{
    cmCacheManager *cachem = m_cmake->GetCacheManager();
    FillCacheManagerFromCacheGUI();
  
    // write the cache to disk
    if(!m_cmBuildPath->GetValue().Trim().IsEmpty())
        cachem->SaveCache(m_cmBuildPath->GetValue().c_str());
}

void CMakeSetupFrm::FillCacheManagerFromCacheGUI()
{ 
    cmCacheManager *cachem = m_cmake->GetCacheManager();
    
    cmCacheManager::CacheIterator it = cachem->NewIterator();
    for(size_t i = 0; i < m_cmOptions->GetCount(); i++)
    {
        wxPropertyItem* item = m_cmOptions->GetItem(i); 
        if ( it.Find((const char*)item->GetPropName().c_str()) )
        {
            // if value is enclosed in single quotes ('foo') then remove them
            // they were used to enforce the fact that it had 'invisible' 
            // trailing stuff
            if (item->GetCurValue().Len() >= 2 &&
                item->GetCurValue().GetChar(0) == '\'' && 
                item->GetCurValue().GetChar(item->GetCurValue().Len() - 1) == '\'') 
            {
                it.SetValue(item->GetCurValue().Mid(1, item->GetCurValue().Len() - 2).c_str());
            }
            else
                it.SetValue(item->GetCurValue().c_str());
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_OK
 */

void CMakeSetupFrm::OnButtonOk( wxCommandEvent& event )
{
    DoGenerate();
}

void CMakeSetupFrm::DoGenerate()
{
    cmSystemTools::EnableMessages();
    
    cmSystemTools::ResetErrorOccuredFlag(); 
    
    m_cmOptions->HideControls();
    PerformCacheRun();
  
    if(!RunCMake(true))
    {
        // issue a close when this is done (this is issued by menu "Generate and Exit"
        if(m_quitAfterGenerating)
            Close();
        else if(!wxGetKeyState(WXK_SHIFT))
        {
            bool close;
            m_config->Read(CM_CLOSEAFTERGEN, &close, CM_CLOSEAFTERGEN_DEF);
            
            if(!close)
                wxMessageBox(wxT("Building of project files succesful!"), wxT("Success!"), wxOK|wxICON_INFORMATION);
            else
                Close();
        }
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_CANCEL
 */

void CMakeSetupFrm::OnButtonCancel( wxCommandEvent& event )
{
    DoCancelButton();
}

void CMakeSetupFrm::DoCancelButton()
{
    if(m_RunningConfigure)
    {
        int result = wxMessageBox(wxT("You are in the middle of a Configure.\n"
                                      "If you Cancel now the configure information will be lost.\n"
                                      "Are you sure you want to Cancel?"), wxT("Warning"), wxYES_NO|wxICON_WARNING);
        if(result == wxYES)
            cmSystemTools::SetFatalErrorOccured();
        else
            if(m_progressDlg)
                m_progressDlg->ResetCancel();
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_DELETE_CACHE
 */

void CMakeSetupFrm::OnButtonDeleteCache( wxCommandEvent& event )
{
    DoDeleteCache();
}

void CMakeSetupFrm::DoDeleteCache()
{
    bool deletecache = true;
    if(m_cmOptions->IsCacheDirty() || (m_cmOptions->GetCount() > 0 && !m_cmOptions->IsGenerated()))
    {
        int result = ::wxMessageBox(_("You have changed options, are you sure you want to delete all items?\n"), 
                                    _("Warning"), wxYES_NO|wxICON_QUESTION);
        
        // when user wants to wait, wait.. else quit
        if(result == wxNO)
            deletecache = false;
            
    }

    if(deletecache)
    {
        // indicate that we haven't generated a project yet
        m_cmOptions->SetProjectGenerated(false);
        
        if(!m_cmBuildPath->GetValue().Trim().IsEmpty() && m_cmake != 0)
            m_cmake->GetCacheManager()->DeleteCache(m_cmBuildPath->GetValue().Trim());
      
        LoadCacheFromDiskToGUI();   
        UpdateWindowState();
    }
}

/*!
 * Should we show tooltips?
 */

bool CMakeSetupFrm::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMakeSetupFrm::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMakeSetupFrm bitmap retrieval
    return wxNullBitmap;
////@end CMakeSetupFrm bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMakeSetupFrm::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMakeSetupFrm icon retrieval
    if (name == wxT("cmake_icon.xpm"))
    {
        wxIcon icon(_T("cmake_icon.xpm"), wxBITMAP_TYPE_XPM);
        return icon;
    }
    return wxNullIcon;
////@end CMakeSetupFrm icon retrieval
}

/*!
 * wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGING event handler for ID_SPLITTERWINDOW
 */

void CMakeSetupFrm::OnSplitterPosChanging( wxSplitterEvent& event )
{
    int width, height;

    GetSize(&width, &height);

    if((height > 100))
    {
        if(event.GetSashPosition() < 170)
            event.SetSashPosition(170);
        else
        {
            if(event.GetSashPosition() > (height - 180))
                event.SetSashPosition(height - 180);
        }
    }
    else
        event.Veto();

}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_LOG
 */

void CMakeSetupFrm::OnClearLogClick( wxCommandEvent& event )
{
    // delete the log text
    m_cmLog->Clear();
    m_cmLog->DiscardEdits();
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SOURCE_BUILD_PATH
 */

void CMakeSetupFrm::OnSourceBuildPathUpdated( wxCommandEvent& event )
{
    DoReloadCache();
}

void CMakeSetupFrm::DoReloadCache()
{
    wxString buildpath = m_cmBuildPath->GetValue();
    // The build dir has changed, check if there is a cache, and 
    // grab the source dir from it

    // make sure the call to update grid is not executed
    m_noRefresh = true;
    m_cmSearchQuery->SetValue(_(""));
    m_noRefresh = false;

    std::string path = buildpath.c_str();
    cmSystemTools::ConvertToUnixSlashes(path);

    // adjust the cmake instance
    m_cmake->SetHomeOutputDirectory(buildpath.c_str());
    m_cmake->SetStartOutputDirectory(buildpath.c_str());

    std::string cache_file = path;
    cache_file += "/CMakeCache.txt";

    // fill in the project path where the source is located, this is 
    // read from the CMake cache
    cmCacheManager *cachem = m_cmake->GetCacheManager();
    cmCacheManager::CacheIterator it = cachem->NewIterator();
    if (cmSystemTools::FileExists(cache_file.c_str()) && cachem->LoadCache(path.c_str()) && 
        it.Find("CMAKE_HOME_DIRECTORY"))
    {
        path = ConvertToWindowsPath(it.GetValue());
        m_cmProjectPath->SetValue(path.c_str());
    }

    m_cmOptions->RemoveAll();
    LoadCacheFromDiskToGUI();
    UpdateWindowState();
}


/*!
 * wxEVT_COMMAND_TEXT_ENTER event handler for ID_SOURCE_BUILD_PATH
 */

void CMakeSetupFrm::OnSourceBuildPathEnter( wxCommandEvent& event )
{
    OnSourceBuildPathUpdated(event);
}

/*!
 * wxEVT_MOTION event handler for ID_OPTIONS
 */

void CMakeSetupFrm::OnPropertyMotion( wxMouseEvent& event )
{
    ShowPropertyDescription(m_cmOptions->YToRow(event.GetY()));
    event.Skip();
}


/*!
 * wxEVT_GRID_SELECT_CELL event handler for ID_OPTIONS
 */

void CMakeSetupFrm::OnGridSelectCell( wxGridEvent& event )
{
    // show description 
    ShowPropertyDescription(event.GetRow());
    
    // enable or disable the browse button
    m_cmBrowseCell->Enable(m_cmOptions->IsSelectedItemBrowsable(event.GetRow()));
    event.Skip();
}

void CMakeSetupFrm::ShowPropertyDescription(int row)
{
    if(row == wxNOT_FOUND || row < 0)
        m_cmDescription->SetValue(wxEmptyString);
    else
    {
        wxPropertyItem *pItem = m_cmOptions->GetPropertyItemFromRow(row);
        if(pItem)
            m_cmDescription->SetValue(pItem->GetHelpString());
        else
            m_cmDescription->SetValue(wxEmptyString);
    }
}

/*!
 * wxEVT_GRID_CELL_CHANGE event handler for ID_OPTIONS
 */

void CMakeSetupFrm::OnCellChange( wxGridEvent& event )
{
    // update the button state when the cache is invalidated
    UpdateWindowState();
}

void CMakeSetupFrm::OnRecentFileMenu( wxCommandEvent &event )
{   
    if(GetMenuBar())
    {
        // get file menu
        wxMenu *mnu = GetMenuBar()->GetMenu(0);
        wxASSERT(mnu != 0);

        wxMenuItem *item = mnu->FindItem(event.GetId());
        if(item)
            m_cmBuildPath->SetValue(item->GetLabel());
    }
}
/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
 */

void CMakeSetupFrm::OnSearchquerySelected( wxCommandEvent& event )
{
    m_cmOptions->SetQuery(m_cmSearchQuery->GetValue());
}

void CMakeSetupFrm::OnAddQuery ( wxCommandEvent &event )
{
    // add current text if not yet present
    if(m_cmSearchQuery->FindString(m_cmSearchQuery->GetValue()) == wxNOT_FOUND)
    {
        m_cmSearchQuery->Append(m_cmSearchQuery->GetValue());
        
        // if too many items are present, prune
        while(m_cmSearchQuery->GetCount() > CM_MAX_SEARCH_QUERIES)
            m_cmSearchQuery->Delete(0);     
    }
}

/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SEARCHQUERY
 */

void CMakeSetupFrm::OnSearchqueryUpdated( wxCommandEvent& event )
{
    // only refresh when this event was caused by user
    if(!m_noRefresh)
        m_cmOptions->SetQuery(m_cmSearchQuery->GetValue());
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BROWSE_GRID
 */

void CMakeSetupFrm::OnBrowseGridClick( wxCommandEvent& event )
{
    m_cmOptions->BrowseSelectedItem();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_RELOAD_CACHE
 */

void CMakeSetupFrm::OnMenuReloadCacheClick( wxCommandEvent& event )
{
    bool reload = true;
    if(m_cmOptions->IsCacheDirty() || (m_cmOptions->GetCount() > 0 && !m_cmOptions->IsGenerated()))
    {
        int result = ::wxMessageBox(_("You have changed options, are you sure you want to reload?\n"), 
                                    _("Warning"), wxYES_NO|wxICON_QUESTION);
        
        // when user wants to wait, wait.. else quit
        if(result == wxNO)
            reload = false;
            
    }

    if(reload) 
        DoReloadCache();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_DELETE_CACHE
 */

void CMakeSetupFrm::OnMenuDeleteCacheClick( wxCommandEvent& event )
{
    DoDeleteCache();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_QUIT
 */

void CMakeSetupFrm::OnMenuQuitClick( wxCommandEvent& event )
{
    // the close event will veto if the user 
    // did not want to quit due to unsaved changes
    Close();
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_FRAME
 */

void CMakeSetupFrm::OnCloseWindow( wxCloseEvent& event )
{
    // ask quit if:
    //  - The cache is dirty
    //  - Or the cache is OK and has some items, and no project was generated recently (configure -> generate)
    if(m_cmOptions->IsCacheDirty() || (m_cmOptions->GetCount() > 0 && !m_cmOptions->IsGenerated()))
    {
        int result = ::wxMessageBox(_("You have changed options, but not yet generated the projects\n"
                                      "are you sure you want to quit?"), _("Warning"), wxYES_NO|wxICON_QUESTION);
        
        // when user wants to wait, wait.. else quit
        if(result == wxNO)
            event.Veto();
        else
            event.Skip();
    }
    else
        event.Skip();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_ABOUTDLG
 */

void CMakeSetupFrm::OnAboutClick( wxCommandEvent& event )
{
    CMAboutDlg *dlg = new CMAboutDlg(this);
    
    wxArrayString generators;
    std::vector<std::string> names; 
    m_cmake->GetRegisteredGenerators(names);
    for(std::vector<std::string>::iterator i = names.begin(); i != names.end(); ++i)
        generators.Add(i->c_str());

    wxString cmversion, cmsversion;
    cmversion.Printf("v%i.%i %s", cmake::GetMajorVersion(), cmake::GetMinorVersion(), cmake::GetReleaseVersion());
    cmsversion.Printf("v%i.%i%s", CMAKEGUI_MAJORVER, CMAKEGUI_MINORVER, CMAKEGUI_ADDVER);

    dlg->SetAboutText(cmversion, cmsversion, generators);

    dlg->ShowModal();
    dlg->Destroy();
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CMAKE_OPTIONS
 */

void CMakeSetupFrm::OnOptionsClick( wxCommandEvent& event )
{
    CMOptionsDlg *dlg = new CMOptionsDlg(this);

    dlg->SetConfig(m_config);
    if(dlg->ShowModal() == wxID_OK)
    {
        // store volatile settings
        dlg->GetConfig(m_config);
        
        // apply non volatile setting such as clear search query, recent menu, etc.
        SyncFormOptions(dlg);
    }

    dlg->Destroy();
}

void CMakeSetupFrm::SyncFormOptions(CMOptionsDlg *dlg)
{
    // TODO: Clear search query etc.
}
/*!
 * wxEVT_COMMAND_SPLITTER_DOUBLECLICKED event handler for ID_SPLITTERWINDOW
 */

void CMakeSetupFrm::OnSplitterwindowSashDClick( wxSplitterEvent& event )
{
    event.Veto(); 
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_CONFIGURE
 */

void CMakeSetupFrm::OnMenuConfigureClick( wxCommandEvent& event )
{
    DoConfigure();
}

/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_EXITGENERATE
 */

void CMakeSetupFrm::OnMenuGenerateClick( wxCommandEvent& event )
{
    // set flag so that a close command is issued
    // after generating the cmake cache to projects
    m_quitAfterGenerating = true;
    DoGenerate();
    m_quitAfterGenerating = false;
}


/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_TOGGLE_ADVANCED
 */

void CMakeSetupFrm::OnMenuToggleAdvancedClick( wxCommandEvent& event )
{
    // toggle the check box
    m_cmShowAdvanced->SetValue(!m_cmShowAdvanced->GetValue());
    OnShowAdvancedValues(event);
}


