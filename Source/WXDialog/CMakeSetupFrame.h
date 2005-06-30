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

#ifndef _CMAKESETUPFRAME_H_
#define _CMAKESETUPFRAME_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "CMakeSetupFrame.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/splitter.h"
#include "wx/grid.h"
#include "wx/statline.h"
#include "wx/statusbr.h"
////@end includes

#include <wx/config.h>
#include <wx/dataobj.h>
#include <wx/dnd.h>


#include "../cmake.h"
#include "progressdlg.h"
#include "optionsdlg.h"
#include "CommandLineInfo.h"
#include "config.h"

// this ID should be taken as base to make sure they are unique
// NOTE: DialogBlocks starts at 10100 so 10000 to 10099 are free
#define CM_NEXT_USABLEID      CM_RECENT_BUILD_ITEM + CM_MAX_RECENT_PATHS

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSplitterWindow;
class wxPropertyList;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_FRAME 10100
#define SYMBOL_CMAKESETUPFRM_STYLE wxDEFAULT_FRAME_STYLE|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX|wxSUNKEN_BORDER
#define SYMBOL_CMAKESETUPFRM_TITLE _("CMake Setup v2.0.x")
#define SYMBOL_CMAKESETUPFRM_IDNAME ID_FRAME
#define SYMBOL_CMAKESETUPFRM_SIZE wxSize(600, 550)
#define SYMBOL_CMAKESETUPFRM_POSITION wxDefaultPosition
#define ID_SPLITTERWINDOW 10101
#define ID_MAINPANEL 10102
#define ID_PROJECT_PATH 10103
#define ID_BROWSE_PROJECT 10104
#define ID_SOURCE_BUILD_PATH 10105
#define ID_BROWSE_BUILD 10106
#define ID_CHOOSE_GENERATOR 10107
#define ID_SEARCHQUERY 10109
#define ID_SHOW_ADVANCED 10108
#define ID_OPTIONS 10110
#define ID_LOGPANEL 10111
#define ID_LOG_AREA 10112
#define ID_DESCRIPTION 10113
#define ID_DO_CONFIGURE 10114
#define ID_DO_OK 10115
#define ID_DO_CANCEL 10116
#define ID_DO_DELETE_CACHE 10117
#define ID_CLEAR_LOG 10118
#define ID_BROWSE_GRID 10119
#define ID_STATUSBAR 10120
#define ID_MENU_RELOAD_CACHE 10122
#define ID_MENU_DELETE_CACHE 10123
#define ID_MENU_QUIT 10125
#define ID_MENU_CONFIGURE 10126
#define ID_MENU_EXITGENERATE 10127
#define ID_MENU_TOGGLE_ADVANCED 10128
#define ID_CMAKE_OPTIONS 10124
#define ID_ABOUTDLG 10121
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

class DnDFile : public wxFileDropTarget
{
public:
    DnDFile(wxTextCtrl *pOwner) 
        : m_pOwner(pOwner)
    {
    };

    virtual bool OnDropFiles(wxCoord x, wxCoord y,
                             const wxArrayString& filenames);

private:
    wxTextCtrl *m_pOwner;
};


/*!
 * CMakeSetupFrm class declaration
 */

class CMakeSetupFrm: public wxFrame
{    
    DECLARE_CLASS( CMakeSetupFrm )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMakeSetupFrm( );
    CMakeSetupFrm( wxWindow* parent, wxWindowID id = SYMBOL_CMAKESETUPFRM_IDNAME, const wxString& caption = SYMBOL_CMAKESETUPFRM_TITLE, const wxPoint& pos = SYMBOL_CMAKESETUPFRM_POSITION, const wxSize& size = SYMBOL_CMAKESETUPFRM_SIZE, long style = SYMBOL_CMAKESETUPFRM_STYLE );

    // Destructor
    virtual ~CMakeSetupFrm();

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMAKESETUPFRM_IDNAME, const wxString& caption = SYMBOL_CMAKESETUPFRM_TITLE, const wxPoint& pos = SYMBOL_CMAKESETUPFRM_POSITION, const wxSize& size = SYMBOL_CMAKESETUPFRM_SIZE, long style = SYMBOL_CMAKESETUPFRM_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /** Logs a message. For the kind parameter use; 
        1  - neutral message
        0  - warning message (blue text)
        -1 - error message (red text)
    */
    void LogMessage(int logkind, const char *msg);

    void UpdateProgress(float progress) {
        if(m_progressDlg)
            m_progressDlg->SetProgress(progress);

        // also show in the tiny field
        wxStatusBar *bar = GetStatusBar();
        if(bar)
        {
            wxString str;
            str.Printf("%2.1f %%", (progress * 100));
            bar->SetStatusText(str, 1);
        }
    };

    void IssueUpdate();

    /** Initialise all crap in the frame, like listing the make generators,
        selecting the best one to use, and loading the cache for the first time
        when the build paths were set */
    void DoInitFrame(cmCommandLineInfo &cm, const wxString &fn);

////@begin CMakeSetupFrm event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_FRAME
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGING event handler for ID_SPLITTERWINDOW
    void OnSplitterPosChanging( wxSplitterEvent& event );

    /// wxEVT_COMMAND_SPLITTER_DOUBLECLICKED event handler for ID_SPLITTERWINDOW
    void OnSplitterwindowSashDClick( wxSplitterEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BROWSE_PROJECT
    void OnButtonBrowseProject( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SOURCE_BUILD_PATH
    void OnSourceBuildPathUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_SOURCE_BUILD_PATH
    void OnSourceBuildPathEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BROWSE_BUILD
    void OnButtonBrowseBuild( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_SEARCHQUERY
    void OnSearchquerySelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SEARCHQUERY
    void OnSearchqueryUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SHOW_ADVANCED
    void OnShowAdvancedValues( wxCommandEvent& event );

    /// wxEVT_GRID_CELL_CHANGE event handler for ID_OPTIONS
    void OnCellChange( wxGridEvent& event );

    /// wxEVT_GRID_SELECT_CELL event handler for ID_OPTIONS
    void OnGridSelectCell( wxGridEvent& event );

    /// wxEVT_MOTION event handler for ID_OPTIONS
    void OnPropertyMotion( wxMouseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_CONFIGURE
    void OnButtonConfigure( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_OK
    void OnButtonOk( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_CANCEL
    void OnButtonCancel( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DO_DELETE_CACHE
    void OnButtonDeleteCache( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_LOG
    void OnClearLogClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BROWSE_GRID
    void OnBrowseGridClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_RELOAD_CACHE
    void OnMenuReloadCacheClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_DELETE_CACHE
    void OnMenuDeleteCacheClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_QUIT
    void OnMenuQuitClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_CONFIGURE
    void OnMenuConfigureClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_EXITGENERATE
    void OnMenuGenerateClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_MENU_TOGGLE_ADVANCED
    void OnMenuToggleAdvancedClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_CMAKE_OPTIONS
    void OnOptionsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_ABOUTDLG
    void OnAboutClick( wxCommandEvent& event );

////@end CMakeSetupFrm event handler declarations

    void OnRecentFileMenu( wxCommandEvent &event );

    void OnAddQuery ( wxCommandEvent &event );

////@begin CMakeSetupFrm member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMakeSetupFrm member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

        /** Load cache for the CMakeLists to the GUI */
    void LoadCacheFromDiskToGUI();

    /** Display the grid with the loaded cache. */
    void FillCacheGUIFromCacheManager();

    int RunCMake(bool generateProjectFiles);

    /** Save cache to disk from the GUI */
    void SaveCacheFromGUI();

    void FillCacheManagerFromCacheGUI();

    /** Update the cache and mark all the new items as old */
    bool PerformCacheRun();

    /** Shows the property matched by the row in the description area
        or nothing when the row is not correct */
    void ShowPropertyDescription(int row);

    /** Disable or enable controls based upon the internal state of the 
        program */
    void UpdateWindowState();

    void AppendPathToRecentList(const wxString &p);

    /** Used to synchonise any options that have immediate effect on the GUI
        form, like clearing a search list, resetting a spitter perhaps, etc */
    void SyncFormOptions(CMOptionsDlg *dlg);

    void DoCancelButton();

    void DoReloadCache();

    void DoDeleteCache();

    void DoConfigure();

    void DoGenerate();

    void OnExitTimer(wxTimerEvent &event);

////@begin CMakeSetupFrm member variables
    wxSplitterWindow* m_splitter;
    wxTextCtrl* m_cmProjectPath;
    wxButton* m_BrowseProjectPathButton;
    wxTextCtrl* m_cmBuildPath;
    wxButton* m_BrowseSourcePathButton;
    wxComboBox* m_cmGeneratorChoice;
    wxComboBox* m_cmSearchQuery;
    wxCheckBox* m_cmShowAdvanced;
    wxPropertyList* m_cmOptions;
    wxTextCtrl* m_cmLog;
    wxTextCtrl* m_cmDescription;
    wxButton* m_ConfigureButton;
    wxButton* m_OkButton;
    wxButton* m_CancelButton;
    wxButton* m_DeleteCacheButton;
    wxButton* m_ClearLogButton;
    wxButton* m_cmBrowseCell;
////@end CMakeSetupFrm member variables

    // this is the cmake instance with which we will communicate
    // to generate our stuff, and get the info from.
    cmake *m_cmake;

    // the config class for project build dir and source path
    wxTimer *m_ExitTimer;
    wxConfig *m_config;
    wxString m_PathToExecutable;
    wxArrayString m_recentPaths;
    CMProgressDialog *m_progressDlg;
    bool m_RunningConfigure;
    bool m_noRefresh;
    bool m_quitAfterGenerating;
};

#endif
    // _CMAKESETUPFRAME_H_
