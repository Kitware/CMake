/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmWXMainFrame.h"

#include "cmCacheManager.h"
#include "cmWXCacheProperty.h"
#include "cmWXCommandLineInfo.h"
#include "cmake.h"

cmMainFrame::cmMainFrame(const wxString& title, const wxSize& size)
  : wxFrame((wxFrame*)NULL, cmMainFrame::ID_MainFrame, title, wxDefaultPosition, size)
{

  cmSystemTools::SetErrorCallback(cmMainFrame::MessageCallback, this);
  this->m_Clean            = false;
  this->m_BuildPathChanged = false;
  this->m_WhereSource      = "";
  this->m_WhereBuild       = "";
  
  this->m_CMakeInstance    = 0;
  this->m_Update           = false;
  this->m_Valid            = false;
  this->m_EntryRemoved     = false;
  this->m_CursorChanged    = false;

  this->m_CacheEntries     = new cmMainFrame::CacheMapType;

  this->CreateStatusBar(1);
  this->SetStatusText("Welcome to CMakeSetup");

  
  this->m_MainPanel = new wxPanel(this, -1);

  this->SetBackgroundColour(this->m_MainPanel->GetBackgroundColour());
  
  this->m_TopMostSizer = new wxBoxSizer(wxVERTICAL);
  
  this->m_TopMostSizer->Add( this->m_MainPanel, 1, wxGROW | wxALL, 5 );

  wxFlexGridSizer* msizer = new wxFlexGridSizer(1, 5, 5);
  msizer->AddGrowableRow(2);
  msizer->AddGrowableCol(0);
  this->m_MainSizer = msizer;

  wxFlexGridSizer* tgrid = new wxFlexGridSizer(7, 2, 2);
  tgrid->AddGrowableCol(2);
  this->m_TopGrid = tgrid;
  

  this->m_TextSource = new wxStaticText(this->m_MainPanel, -1, "Where is the source code:");
  this->m_PathSource = new wxComboBox(this->m_MainPanel, -1, "PathSource");
  this->m_BrowseSource = new wxButton(this->m_MainPanel, -1, "Browse...");

  this->m_GeneratorFrame = new wxBoxSizer(wxHORIZONTAL);
  this->m_BuildFor = new wxStaticText(this->m_MainPanel, -1, "Build For:");
  this->m_GeneratorMenu = new wxComboBox(this->m_MainPanel, -1, "Generator", 
                                         wxDefaultPosition, wxDefaultSize,
                                         0, 0, wxCB_READONLY);

  this->m_GeneratorFrame->Add(this->m_BuildFor, 0, wxALIGN_LEFT);
  this->m_GeneratorFrame->Add(5,5,0);
  this->m_GeneratorFrame->Add(this->m_GeneratorMenu, 1, wxGROW | wxALL );

  this->m_TextBinary = new wxStaticText(this->m_MainPanel, -1, 
                                        "Where to build the binaries:");
  this->m_PathBinary = new wxComboBox(this->m_MainPanel, -1, "PathBinary");
  this->m_BrowseBinary = new wxButton(this->m_MainPanel, -1, "Browse...");
  this->m_ShowAdvancedValues = new wxCheckBox(this->m_MainPanel, -1, 
                                              "Show Advanced Values");

  this->m_TopGrid->Add(this->m_TextSource, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  this->m_TopGrid->Add(5, 5, 0);
  this->m_TopGrid->Add(this->m_PathSource, 1, wxGROW | wxLEFT | wxRIGHT );
  this->m_TopGrid->Add(5, 5, 0);
  this->m_TopGrid->Add(this->m_BrowseSource, 1, 0);
  this->m_TopGrid->Add(5, 5, 0);
  this->m_TopGrid->Add(this->m_GeneratorFrame, 1, 0);
  this->m_TopGrid->Add(this->m_TextBinary, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  this->m_TopGrid->Add(5, 5, 0);
  this->m_TopGrid->Add(this->m_PathBinary, 1, wxGROW | wxLEFT | wxRIGHT );
  this->m_TopGrid->Add(5, 5, 0);
  this->m_TopGrid->Add(this->m_BrowseBinary, 1, 0);
  this->m_TopGrid->Add(5, 5, 0);
  this->m_TopGrid->Add(this->m_ShowAdvancedValues, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  this->m_MainSizer->Add(5, 5, 0);
  this->m_MainSizer->Add(this->m_TopGrid, 1, wxGROW | wxLEFT | wxRIGHT, 10 );

  this->m_CacheValuesBox = new wxStaticBox(this->m_MainPanel, -1, "Cache Values");
  //this->m_CacheValuesBox->SetBackgroundColour(*wxWHITE);
  this->m_CacheValuesFrame = new wxStaticBoxSizer(this->m_CacheValuesBox,
                                                  wxVERTICAL);
  this->m_CacheValuesScroll = new wxScrolledWindow(this->m_MainPanel, -1, 
                                                   wxDefaultPosition, wxDefaultSize, 
                                                   wxVSCROLL);
  this->m_CacheValuesScroll->SetBackgroundColour(*wxWHITE);
  this->m_CacheValuesFrame->Add(this->m_CacheValuesScroll, 1, wxEXPAND | wxALL, 0 );
  this->m_CacheValuesPanel = new wxPanel(this->m_CacheValuesScroll, -1);
  this->m_CacheValuesPanel->SetBackgroundColour(wxColor(150, 150, 150));
  wxFlexGridSizer* csizer = new wxFlexGridSizer(2, 1, 1);
  csizer->AddGrowableCol(0);
  csizer->AddGrowableCol(1);
  this->m_CacheValuesSizer = csizer;
  this->m_CacheValuesPanel->SetAutoLayout( TRUE );
  this->m_CacheValuesPanel->SetSizer(this->m_CacheValuesSizer);
  //this->m_CacheValuesPanel->SetBackgroundColour(wxColour(10,10,10));
  this->m_CacheValuesSizer->Fit(this->m_CacheValuesPanel);
  this->m_CacheValuesSizer->SetSizeHints(this->m_CacheValuesPanel);
  
  //this->m_CacheValuesScroll->SetScrollbars(0, 20, 0, 50);
  //wxSize framesize = this->m_CacheValuesBox->GetSize();
  //this->m_CacheValuesBox->SetSize(framesize.GetWidth(), 100);
  this->SetSizeHints(580, 340);
  this->m_MainSizer->Add(this->m_CacheValuesFrame, 1, wxGROW | wxALL );
  wxString helpTextData = "";
  helpTextData.append
    ("Right click on a cache value for additional options "
     "(delete, ignore, and help).\n"
     "Press Configure to update and display new values in red.\n"
     "Press OK to generate selected build files and exit.");
  this->m_HelpText = new wxStaticText(this->m_MainPanel, -1, helpTextData, 
                                      wxDefaultPosition,
                                      wxDefaultSize,
                                      wxTE_MULTILINE);

  this->m_MainSizer->Add(5, 5, 0);
  this->m_MainSizer->Add(this->m_HelpText, 0, wxALIGN_CENTER_HORIZONTAL, 10);

  this->m_BottomButtonsFrame = new wxBoxSizer(wxHORIZONTAL);
  
  //this->m_VersionText = new wxStaticText(this->m_MainPanel, -1, 
  //"Version 1.5 - development");
  this->m_ConfigureButton = new wxButton(this->m_MainPanel, -1, "Configure");
  this->m_OKButton = new wxButton(this->m_MainPanel, -1, "OK");
  this->m_CancelButton = new wxButton(this->m_MainPanel, -1, "Cancel");
  this->m_HelpButton = new wxButton(this->m_MainPanel, -1, "Help");


  this->m_BottomButtonsFrame->Add(m_ConfigureButton, 0, wxALIGN_LEFT, 10);
  this->m_BottomButtonsFrame->Add(5, 5, 0);
  this->m_BottomButtonsFrame->Add(m_OKButton, 0, wxALIGN_LEFT, 10);
  this->m_BottomButtonsFrame->Add(5, 5, 0);
  this->m_BottomButtonsFrame->Add(m_CancelButton, 0, wxALIGN_LEFT, 10);
  this->m_BottomButtonsFrame->Add(5, 5, 0);
  this->m_BottomButtonsFrame->Add(m_HelpButton, 0, wxALIGN_LEFT, 10);

  this->m_MainSizer->Add(5, 5, 0);
  this->m_MainSizer->Add(m_BottomButtonsFrame, 0, wxALIGN_CENTER_HORIZONTAL, 10);
  this->m_MainSizer->Add(5, 5, 0);

  this->m_MainPanel->SetAutoLayout( TRUE );
  this->m_MainPanel->SetSizer(this->m_MainSizer);
  this->m_MainSizer->Fit(this->m_MainPanel);
  this->m_MainSizer->SetSizeHints(this->m_MainPanel);

  this->SetAutoLayout( TRUE );
  this->SetSizer(this->m_TopMostSizer);
  this->m_TopMostSizer->Fit(this);
  this->m_TopMostSizer->SetSizeHints(this);

  // Setup statusbar callbacks
  
  this->SetupStatusBarBinding(this->m_ConfigureButton);
  this->SetupStatusBarBinding(this->m_OKButton);
  this->SetupStatusBarBinding(this->m_CancelButton);
  this->SetupStatusBarBinding(this->m_HelpButton);
  this->SetupStatusBarBinding(this->m_PathSource);
  this->SetupStatusBarBinding(this->m_BrowseSource);
  this->SetupStatusBarBinding(this->m_PathBinary);
  this->SetupStatusBarBinding(this->m_BrowseBinary);
  this->SetupStatusBarBinding(this->m_GeneratorMenu);
  this->SetupStatusBarBinding(this->m_ShowAdvancedValues);

  // Setup other callbacks
  this->ConnectEvent( this->m_CancelButton, wxEVT_COMMAND_BUTTON_CLICKED,
                      (wxObjectEventFunction) &cmMainFrame::OnCancel );  
  this->ConnectEvent( this->m_OKButton, wxEVT_COMMAND_BUTTON_CLICKED,
                      (wxObjectEventFunction) &cmMainFrame::OnOk );  
  this->ConnectEvent( this->m_HelpButton, wxEVT_COMMAND_BUTTON_CLICKED,
                      (wxObjectEventFunction) &cmMainFrame::OnHelp );  
  this->ConnectEvent( this->m_ConfigureButton, wxEVT_COMMAND_BUTTON_CLICKED,
                      (wxObjectEventFunction) &cmMainFrame::OnConfigure );  
  this->ConnectEvent( this, wxEVT_SIZE,
                      (wxObjectEventFunction) &cmMainFrame::OnResize );  
  this->ConnectEvent( this->m_ShowAdvancedValues, wxEVT_COMMAND_CHECKBOX_CLICKED,
                      (wxObjectEventFunction) &cmMainFrame::OnShowAdvancedValues );
  this->ConnectEvent( this->m_BrowseSource, wxEVT_COMMAND_BUTTON_CLICKED,
                      (wxObjectEventFunction) &cmMainFrame::OnBrowseSource );
  this->ConnectEvent( this->m_BrowseBinary, wxEVT_COMMAND_BUTTON_CLICKED,
                      (wxObjectEventFunction) &cmMainFrame::OnBrowseBinary );
  this->ConnectEvent( this->m_PathSource, wxEVT_COMMAND_COMBOBOX_SELECTED,
                      (wxObjectEventFunction) &cmMainFrame::OnSourceSelected );
  this->ConnectEvent( this->m_PathSource, wxEVT_COMMAND_TEXT_UPDATED,
                      (wxObjectEventFunction) &cmMainFrame::OnSourceSelected );
  this->ConnectEvent( this->m_PathBinary, wxEVT_COMMAND_COMBOBOX_SELECTED,
                      (wxObjectEventFunction) &cmMainFrame::OnBinarySelected );
  this->ConnectEvent( this->m_PathBinary, wxEVT_COMMAND_TEXT_UPDATED,
                      (wxObjectEventFunction) &cmMainFrame::OnBinarySelected );
  this->ConnectEvent( this->m_GeneratorMenu, wxEVT_COMMAND_COMBOBOX_SELECTED,
                      (wxObjectEventFunction) &cmMainFrame::OnGeneratorSelected );
  this->ConnectEvent( this->m_GeneratorMenu, wxEVT_COMMAND_TEXT_UPDATED,
                      (wxObjectEventFunction) &cmMainFrame::OnGeneratorSelected );


  this->ConnectEvent( this, wxEVT_TIMER,
                      (wxObjectEventFunction) &cmMainFrame::OnExitTimer );

  this->Connect(cmCacheProperty::Menu_Popup_Ignore, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction) &cmMainFrame::OnPopupMenuIgnore);
  this->Connect(cmCacheProperty::Menu_Popup_Delete, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction) &cmMainFrame::OnPopupMenuDelete);
  this->Connect(cmCacheProperty::Menu_Popup_Help, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction) &cmMainFrame::OnPopupMenuHelp);

}

cmMainFrame::~cmMainFrame()
{
  cmMainFrame::CacheMapType* items = this->m_CacheEntries;
  for(cmMainFrame::CacheMapType::iterator i = items->begin();
      i != items->end(); ++i)
    {
    cmCacheProperty* item = i->second;
    delete item;
    }
  delete this->m_CacheEntries;

  delete this->m_CMakeInstance;

}

void cmMainFrame::MessageCallback(const char* m, const char* title, bool& nomore,
                                  void* clientData)
{

  if ( clientData )
    {
    cmMainFrame* self = static_cast<cmMainFrame*>( clientData );
    self->DisplayMessage(m, title, nomore);
    }
  else
    {
    std::string message = "The following error happen without frame being set:\n\n";
    message += m;
    message += "\n\n(Press  Cancel to suppress any further messages.)";
    if(::wxMessageBox(message.c_str(), title, 
                      wxICON_WARNING | wxOK | wxCANCEL ) == wxCANCEL)
      {
      nomore = true;
      }    
    }

}

void cmMainFrame::DisplayMessage(const char* m, const char* title, bool& nomore)
{
  this->CursorNormal(false);
  std::string message = m;
  message += "\n\n(Press  Cancel to suppress any further messages.)";
  if(::wxMessageBox(message.c_str(), title, 
                    wxICON_WARNING | wxOK | wxCANCEL ) == wxCANCEL)
    {
    nomore = true;
    }
  if ( this->m_CursorChanged )
    {
    this->CursorBusy(false);
    }
}

void cmMainFrame::ConnectEvent(wxWindow* win, wxEventType et, wxObjectEventFunction func)
{
//
  this->Connect((win?win->GetId():-1), et, func);
//
}

void cmMainFrame::ConnectEventTo(wxWindow* win, wxEventType et, wxObjectEventFunction func)
{
//
  win->Connect(-1, et, func);
//
}

void cmMainFrame::OnStatusBar(wxEvent& event)
{
  wxControl* eobject = static_cast<wxControl*>(event.GetEventObject());
  if ( eobject && eobject->GetClientData() )
    {
    cmMainFrame* self = static_cast<cmMainFrame*>(eobject->GetClientData());
    if ( eobject == self->m_OKButton )
      {
      self->SetStatusText("Press OK to generate selected build files and exit.");
      }
    else if ( eobject == self->m_CancelButton )
      {
      self->SetStatusText("Press Cancel to lose the changes and exit.");
      }
    else if ( eobject == self->m_ConfigureButton )
      {
      self->SetStatusText("Press Configure to update and display new values in red.");
      }
    else if ( eobject == self->m_HelpButton )
      {
      self->SetStatusText("Press Help to display help.");
      }
    else if ( eobject == self->m_ShowAdvancedValues )
      {
      self->SetStatusText("Toggle between regular and advanced cache values.");
      }
    else if ( eobject == self->m_GeneratorMenu )
      {
      self->SetStatusText("Set the generator to generate the build files.");
      }
    else if ( eobject == self->m_PathSource )
      {
      self->SetStatusText("Enter the path to the source files.");
      }
    else if ( eobject == self->m_BrowseSource )
      {
      self->SetStatusText("Browse the path to the source files.");
      }
    else if ( eobject == self->m_PathBinary )
      {
      self->SetStatusText("Enter the path to the build files.");
      }
    else if ( eobject == self->m_BrowseBinary )
      {
      self->SetStatusText("Browse the path to the build files.");
      }
    else
      {
      self->SetStatusText("CMakeSetup");
      }
    }
}

void cmMainFrame::OnCacheStatusBar(wxEvent& event)
{
  wxControl* eobject = static_cast<wxControl*>(event.GetEventObject());
  if ( eobject && eobject->GetClientData() )
    {
    cmCacheProperty* cprop = static_cast<cmCacheProperty*>(eobject->GetClientData());
    cprop->GetMainFrame()->SetStatusText(cprop->GetHelp().c_str(), 0);
    }
}

void cmMainFrame::OnPopupMenu(wxMouseEvent& event)
{

  // 
  wxControl* eobject = static_cast<wxControl*>(event.GetEventObject());
  if ( eobject && eobject->GetClientData() )
    {
    wxMenu menu;
    menu.Append(cmCacheProperty::Menu_Popup_Ignore, "Ignore Cache Entry");
    menu.Append(cmCacheProperty::Menu_Popup_Delete, "Delete Cache Entry");
    menu.Append(cmCacheProperty::Menu_Popup_Help, "Help for Cache Entry");
    cmCacheProperty* cprop = static_cast<cmCacheProperty*>(eobject->GetClientData());
    cmMainFrame* self = cprop->GetMainFrame();
    menu.SetClientData(eobject->GetClientData());
    this->PopupMenu(&menu, event.GetPosition());
    if ( self->m_EntryRemoved )
      {
      self->UpdateCacheValuesDisplay();
      self->m_EntryRemoved = false;
      }
    }
}

void cmMainFrame::OnPopupMenuIgnore(wxEvent& event)
{
//
  this->OnPopupMenuEntry(event, 0);
//
}

void cmMainFrame::OnPopupMenuDelete(wxEvent& event)
{
//
  this->OnPopupMenuEntry(event, 1);
//
}

void cmMainFrame::OnPopupMenuHelp(wxEvent& event)
{
//
  this->OnPopupMenuEntry(event, 2);
//
}

void cmMainFrame::OnPopupMenuEntry(wxEvent& event, int idx)
{

  wxMenu* eobject = static_cast<wxMenu*>(event.GetEventObject());
  if ( eobject && eobject->GetClientData() )
    {
    cmCacheProperty* cprop = static_cast<cmCacheProperty*>(eobject->GetClientData());
    switch ( idx )
      {
      case 0: this->IgnoreCacheEntry(cprop->GetName().c_str());
        break;
      case 1: this->RemoveCacheEntry(cprop);
        break;
      case 2: this->HelpCacheEntry(cprop->GetName().c_str(), cprop->GetHelp().c_str());
        break;
      }
    }
}

void cmMainFrame::OnOk(wxCommandEvent&)
{

  // enable error messages each time configure is pressed
  cmSystemTools::EnableMessages();
  this->ClearDirty();
  this->RunCMake(true);
  cmMainFrame::Close(TRUE);  

}

void cmMainFrame::OnCancel(wxCommandEvent&)
{
  if ( this->IsDirty() )
    {
    // Display dialog
    if ( wxMessageBox( "You have changed options but not rebuild, "
                       "are you sure you want to exit?",
                       "Confirm Exit", wxICON_WARNING | wxYES_NO ) != wxYES )
      {
      return;
      }
    }
  cmMainFrame::Close(TRUE);

}

void cmMainFrame::OnConfigure(wxCommandEvent&)
{
  // enable error messages each time configure is pressed
  cmSystemTools::EnableMessages();
  this->m_Update = true;
  this->RunCMake(false);
}

void cmMainFrame::OnHelp(wxCommandEvent&)
{

  std::string message = 
    "CMake is used to configure and generate build files for software projects. The basic steps for configuring a\n"
    "project are as follows:\n\n"
    "1. Select the source directory for the project.  This should contain the CMakeLists.txt files for the project.\n\n"
    "2. Select the build directory for the project.   This is the directory where the project will be built.  It can\n"
    "be the same or a different directory than the source directory.   For easy clean up, a separate build directory\n"
    "is recommended.  CMake will create the directory if it does not exist.\n\n"
    "3. Once the source and binary directories are selected, it is time to press the Configure button.  This will cause\n"
    "CMake to read all of the input files and discover all the variables used by the project.   The first time a\n"
    "variable is displayed it will be in Red.   Users should inspect red variables making sure the values are correct.\n"
    "For some projects the Configure process can be iterative, so continue to press the Configure button until there\n"
    " are no longer red entries.\n\n"
    "4. Once there are no longer red entries, you should click the OK button.  This will write the build files to the\n"
    "build directory and exit CMake.";
  ::wxMessageBox(message.c_str(), "CMake Help", 
                 wxICON_INFORMATION | wxOK );

}

void cmMainFrame::OnPropertyChanged(wxEvent& event)
{

  wxControl* eobject = static_cast<wxControl*>(event.GetEventObject());
  if ( eobject && eobject->GetClientData() )
    {
    cmCacheProperty* property = static_cast<cmCacheProperty*>( 
      eobject->GetClientData() );
    property->OnPropertyChanged(event);
    }
}

void cmMainFrame::OnResize(wxSizeEvent& event)
{
  
  this->wxFrame::OnSize(event);
  // Expand inner pannel when window resizes
  this->ResizeInternal();
}

void cmMainFrame::OnExitTimer(wxEvent& event)
{
  this->Close();
  this->Refresh();
}

void cmMainFrame::ResizeInternal()
{
  // Expand inner pannel when window resizes
  int x, y;
  this->m_CacheValuesScroll->GetClientSize(&x, &y);
  wxSize size1 = this->m_CacheValuesPanel->GetSize();
  this->m_CacheValuesPanel->SetSize(wxSize(x, size1.GetHeight()));
  //this->m_CacheValuesSizer->SetDimension(0,0,x,size1.GetHeight());
}

void cmMainFrame::OnBrowseSource(wxCommandEvent&)
{
  std::string path = this->m_PathSource->GetValue().c_str();
  if ( path == "PathSource" )
    {
    path = cmSystemTools::CollapseFullPath(this->m_PathToExecutable.c_str());
    }
  wxDirDialog dialog ( this, _T("Select path"), path.c_str() );

  if (dialog.ShowModal() == wxID_OK)
    {
    this->SetSourceDir(dialog.GetPath());
    }
}

void cmMainFrame::OnBrowseBinary(wxCommandEvent&)
{
  std::string path = this->m_PathBinary->GetValue().c_str();
  if ( path == "PathBinary" )
    {
    path = this->m_PathSource->GetValue().c_str();
    if ( path == "PathSource" )
      {
      path = cmSystemTools::CollapseFullPath(this->m_PathToExecutable.c_str());
      }
    }
  wxDirDialog dialog ( this, _T("Select path"), path.c_str() );

  if (dialog.ShowModal() == wxID_OK)
    {
    if ( this->SetBinaryDir(dialog.GetPath()) )
      {
      this->m_Update = true;
      this->ChangeWhereBuild();
      }
    }
}

void cmMainFrame::Initialize(cmCommandLineInfo* cmdInfo)
{

  this->m_PathToExecutable = cmdInfo->GetPathToExecutable();
  this->LoadFromRegistry();
  this->m_CMakeInstance = new cmake; // force a register of generators
  std::vector<std::string> names;
  this->m_CMakeInstance->GetRegisteredGenerators(names);
  for(std::vector<std::string>::iterator i = names.begin();
      i != names.end(); ++i)
    {
    this->m_GeneratorMenu->Append(i->c_str());
    }

  //{{AFX_DATA_INIT(CMakeSetupDialog)
  // Get the parameters from the command line info
  // If an unknown parameter is found, try to interpret it too, since it
  // is likely to be a file dropped on the shortcut :)
  if (cmdInfo->m_LastUnknownParameter.empty())
    {
    if ( cmdInfo->m_WhereSource.size() > 0 )
      {
      this->SetSourceDir( cmdInfo->m_WhereSource.c_str() );
      }
    if ( cmdInfo->m_WhereBuild.size() > 0 )
      {
      this->SetBinaryDir( cmdInfo->m_WhereBuild.c_str() );
      }
    if ( this->m_GeneratorMenu->GetSelection() >= 0 &&
         this->m_GeneratorMenu->GetValue().size() > 0 )
      {
      this->SetGenerator(this->m_GeneratorMenu->GetValue().c_str());
      }
    this->m_ShowAdvancedValues->SetValue(cmdInfo->m_AdvancedValues);
    }
  else
    {
    this->m_ShowAdvancedValues->SetValue(FALSE);
    this->ChangeDirectoriesFromFile(cmdInfo->m_LastUnknownParameter.c_str());
    }
  /*
  this->UpdateSourceBuildMenus();
  */
  this->LoadCacheFromDiskToGUI();

  if ( cmdInfo->m_ExitAfterLoad )
    {
    this->m_ExitTimer = new wxTimer(this, this->GetId());
    this->m_ExitTimer->Start(3000);
    }
}

//! Set the current generator
void cmMainFrame::SetGenerator(const char* generator)
{
  if ( strlen(generator) > 0 )
    {
    int pos = this->m_GeneratorMenu->FindString(generator);    
    if ( pos >= 0 )
      {
      this->m_GeneratorMenu->SetSelection(pos);
      }
    }

}

//! Load cache file from m_WhereBuild and display in GUI editor
void cmMainFrame::LoadCacheFromDiskToGUI()
{
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  if(this->GetBinaryDir().size() > 0 )
    {
    cachem->LoadCache(this->m_WhereBuild.c_str());
    this->UpdateCacheValuesDisplay();
    cmCacheManager::CacheIterator it = cachem->NewIterator();
    if(it.Find("CMAKE_GENERATOR"))
      {
      std::string curGen = it.GetValue();
      if(this->m_GeneratorMenu->GetSelection() < 0 || 
         std::string(this->m_GeneratorMenu->GetValue().c_str()) != curGen)
        {
        this->SetGenerator(curGen.c_str());
        }
      }
    }

}


// copy from the cache manager to the cache edit list box
void cmMainFrame::FillCacheGUIFromCacheManager()
{
 
  //size_t size = 0;
  //size_t size = this->m_CacheEntriesList.GetItems().size();
  //bool reverseOrder = false;
  // if there are already entries in the cache, then
  // put the new ones in the top, so they show up first
  /*
    if(size)
    {
    reverseOrder = true;
    }
  */

  this->UpdateCacheValuesDisplay();

}

void cmMainFrame::OnGeneratorSelected(wxEvent&)
{


}

void cmMainFrame::OnShowAdvancedValues(wxCommandEvent&) 
{
  this->m_Update = false;
  this->UpdateCacheValuesDisplay();
}

// Handle param or single dropped file.
// If the dropped file is a build directory or any file in a 
// build directory, set the source dir from the cache file,
// otherwise set the source and build dirs to this file (or dir).

void cmMainFrame::ChangeDirectoriesFromFile(const char* buffer)
{
  // Get the path to this file

  std::string path = buffer;
  if (!cmSystemTools::FileIsDirectory(path.c_str()))
    {
    path = cmSystemTools::GetFilenamePath(path);
    }
  else
    {
    cmSystemTools::ConvertToUnixSlashes(path);
    }

  // Check if it's a build dir and grab the cache

  std::string cache_file = path;
  cache_file += "/CMakeCache.txt";

  cmCacheManager *cache = this->m_CMakeInstance->GetCacheManager();
  cmCacheManager::CacheIterator it = cache->GetCacheIterator("CMAKE_HOME_DIRECTORY");
  if (cmSystemTools::FileExists(cache_file.c_str()) &&
      cache->LoadCache(path.c_str()) &&
      !it.IsAtEnd())
    {
    path = cmSystemTools::ConvertToOutputPath(path.c_str());
    this->SetBinaryDir(path.c_str());

    path = cmSystemTools::ConvertToOutputPath(it.GetValue());
    this->SetSourceDir(path.c_str());
    }
  else
    {
    path = cmSystemTools::ConvertToOutputPath(path.c_str());
    this->SetSourceDir(path.c_str());
    this->SetBinaryDir(path.c_str());
    }

}

void cmMainFrame::UpdateSourceBuildMenus()
{


}

void cmMainFrame::RunCMake(bool generateProjectFiles)
{
  if(!cmSystemTools::FileExists(this->GetBinaryDir().c_str()))
    {
    std::string message =
      "Build directory does not exist, should I create it?\n\n"
      "Directory: ";
    message += this->GetBinaryDir();
    if(wxMessageBox(message.c_str(), "Create Directory", wxICON_WARNING | wxOK | wxCANCEL) == wxOK)
      {
      cmSystemTools::MakeDirectory(this->GetBinaryDir().c_str());
      }
    else
      {
      (void)wxMessageBox("Build Project aborted, nothing done.", "CMake Aborted", 
                         wxICON_INFORMATION | wxOK);

      return;
      }
    }
  // set the wait cursor
  this->CursorBusy(true);
  

  // get all the info from the dialog
  //this->UpdateData();
  // always save the current gui values to disk
  this->SaveCacheFromGUI();
  // Make sure we are working from the cache on disk
  this->LoadCacheFromDiskToGUI(); 


  // 
  this->m_Valid = true;
  this->m_OKButton->Enable(false);

  // setup the cmake instance
  if (generateProjectFiles)
    {
    if(this->m_CMakeInstance->Generate() != 0)
      {
      cmSystemTools::Error(
        "Error in generation process, project files may be invalid");
      }
    }
  else
    {
    this->m_CMakeInstance->SetHomeDirectory(this->GetSourceDir().c_str());
    this->m_CMakeInstance->SetStartDirectory(this->GetSourceDir().c_str());
    this->m_CMakeInstance->SetHomeOutputDirectory(this->GetBinaryDir().c_str());
    this->m_CMakeInstance->SetStartOutputDirectory(this->GetBinaryDir().c_str());
    this->m_CMakeInstance->SetGlobalGenerator(
     this-> m_CMakeInstance->CreateGlobalGenerator(this->m_GeneratorMenu->GetValue()));
    this->m_CMakeInstance->SetCMakeCommand(this->m_PathToExecutable.c_str());
    this->m_CMakeInstance->LoadCache();
    if(this->m_CMakeInstance->Configure() != 0)
      {
      cmSystemTools::Error(
        "Error in configuration process, project files may be invalid");
      }
    // update the GUI with any new values in the caused by the
    // generation process
    this->LoadCacheFromDiskToGUI();
    }

  // save source and build paths to registry
  this->SaveToRegistry();
  // path is up-to-date now
  this->m_BuildPathChanged = false;
  // put the cursor back
  this->CursorNormal(true);
  cmSystemTools::ResetErrorOccuredFlag();

}

//! Save GUI values to cmCacheManager and then save to disk.
void cmMainFrame::SaveCacheFromGUI()
{
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  this->FillCacheManagerFromCacheGUI();
  if(this->GetBinaryDir() != "")
    {
    cachem->SaveCache(this->GetBinaryDir().c_str());
    }

}

// copy from the list box to the cache manager
void cmMainFrame::FillCacheManagerFromCacheGUI()
{
 
  cmMainFrame::CacheMapType *items = this->m_CacheEntries;
  for(cmMainFrame::CacheMapType::iterator i = items->begin();
      i != items->end(); ++i)
    {
    cmCacheProperty* item = i->second;

    cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
    cmCacheManager::CacheIterator it = cachem->GetCacheIterator(item->GetName().c_str());
    if (!it.IsAtEnd())
      {
      // if value is enclosed in single quotes ('foo') then remove them
      // they were used to enforce the fact that it had 'invisible' 
      // trailing stuff
      std::string str = item->GetValue();
      if (str.size() >= 2 &&
          str[0] == '\'' && 
          str[str.size() - 1] == '\'') 
        {
        it.SetValue(str.substr(1,str.size() - 2).c_str());
        }
      else
        {
        it.SetValue(str.c_str());
        }
      }
    }

}

void cmMainFrame::UpdateCacheValuesDisplay()
{
 
  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();

  this->m_CacheValuesScroll->Scroll(0,0);
  
  if ( this->m_Update )
    {
    // all the current values are not new any more
    cmMainFrame::CacheMapType* items = this->m_CacheEntries;
    for(cmMainFrame::CacheMapType::iterator i = items->begin();
        i != items->end(); ++i)
      {
      cmCacheProperty* item = i->second;
      item->SetNewFlag( false );
      }
    }

  // redraw the list
  this->m_CacheValuesPanel->SetBackgroundColour(*wxWHITE);
  cmMainFrame::CacheMapType* items = this->m_CacheEntries;
  for(cmMainFrame::CacheMapType::iterator i = items->begin();
      i != items->end(); ++i)
    {
    cmCacheProperty* item = i->second;
    item->Remove(this->m_CacheValuesSizer, this->m_CacheValuesPanel);
    }

  //this->m_CacheValuesPanel->SetSize(5,5);
  //this->m_CacheValuesPanel->Fit();

  bool showadvancedvalues = this->m_ShowAdvancedValues->GetValue();
  
  int x, y;
  this->m_CacheValuesPanel->GetSize(&x, &y);

  for(cmCacheManager::CacheIterator i = cachem->NewIterator();
      !i.IsAtEnd(); i.Next())
    {
    const char* key = i.GetName();
    cmMainFrame::CacheMapType::iterator cprop = this->m_CacheEntries->find(key);
    cmCacheProperty *property = 0;
    if ( cprop != this->m_CacheEntries->end() )
      {
      property = cprop->second;
      }
    
    std::string value = i.GetValue();
    
    // if value has trailing space or tab, enclose it in single quotes
    // to enforce the fact that it has 'invisible' trailing stuff
    if (value.size() && 
        (value[value.size() - 1] == ' ' || 
         value[value.size() - 1] == '\t'))
      {
      value = '\'' + value +  '\'';
      }

    if ( i.GetType() != cmCacheManager::BOOL && 
         i.GetType() != cmCacheManager::FILEPATH && 
         i.GetType() != cmCacheManager::PATH && 
         i.GetType() != cmCacheManager::STRING ||
         !showadvancedvalues && i.GetPropertyAsBool("ADVANCED") )
      {
      continue;
      }         

    if ( !property )
      {
      property = new cmCacheProperty(this, key);
      property->SetHelp( i.GetProperty("HELPSTRING") );
      (*this->m_CacheEntries)[key] = property;
      }

    if(i.GetPropertyAsBool("ADVANCED"))
      {
      property->MarkAdvanced();
      }
    if ( !property->IsRemoved() )
      {
      property->SetValue(value);
      switch(i.GetType() )
        {
        case cmCacheManager::BOOL:
          if(cmSystemTools::IsOn(value.c_str()))
            {
            property->SetValue("ON");
            }
          else
            {
            property->SetValue("OFF");
            }
          property->SetItemType( cmCacheProperty::CHECKBOX );
          break;
        case cmCacheManager::PATH:
          property->SetItemType( cmCacheProperty::PATH );
          break;
        case cmCacheManager::FILEPATH:
          property->SetItemType( cmCacheProperty::FILE );
          break;
        case cmCacheManager::STRING:
          property->SetItemType( cmCacheProperty::EDIT );
          break;
        default:
          property->MarkRemoved();
        }
      }    
    }

  if(this->m_CacheEntries->size() > 0 && !cmSystemTools::GetErrorOccuredFlag())
    {
    bool enable = true;
    cmMainFrame::CacheMapType* items = this->m_CacheEntries;
    for(cmMainFrame::CacheMapType::iterator i = items->begin();
        i != items->end(); i++)
      {
      cmCacheProperty* item = i->second;
      if(item && item->GetNewFlag() && !item->IsRemoved())
        {
        // if one new value then disable to OK button
        enable = false;
        this->m_Valid = false;
        break;
        }
      }
    this->m_OKButton->Enable(this->m_Valid);
    this->m_CacheValuesPanel->SetBackgroundColour(wxColor(150, 150, 150));
    }
  else
    {
    this->m_CacheValuesPanel->SetBackgroundColour(*wxWHITE);
    }

  int max = 0;
  int count = 0;
  wxSize size1 = this->m_CacheValuesPanel->GetSize();
  size1.SetHeight(1);
  this->m_CacheValuesPanel->SetSize(size1);
  int height = 0;
  // redraw the list
  cmMainFrame::CacheMapType::iterator nexti;
  for(cmMainFrame::CacheMapType::iterator i = items->begin();
      i != items->end(); i = nexti)
    {
    cmCacheProperty* item = i->second;
    nexti = i;
    nexti++;
    if ( item->IsRemoved() )
      {
      delete item;
      items->erase(i);
      }
    }
  
  for(cmMainFrame::CacheMapType::iterator i = items->begin();
      i != items->end(); i++)
    {
    cmCacheProperty* item = i->second;
    if((showadvancedvalues || !item->IsAdvanced()) && !item->GetNewFlag() )
      {
      int nm = item->Display(this->m_CacheValuesSizer, this->m_CacheValuesPanel);
      height += nm + 1;
      count ++;
      if ( nm > max )
        {
        max = nm;
        }
      }
    }
  for(cmMainFrame::CacheMapType::reverse_iterator i = items->rbegin();
      i != items->rend(); i++)
    {
    cmCacheProperty* item = i->second;
    if((showadvancedvalues || !item->IsAdvanced()) && item->GetNewFlag())
      {
      int nm = item->Display(this->m_CacheValuesSizer, this->m_CacheValuesPanel);
      height += nm + 1;
      count ++;
      if ( nm > max )
        {
        max = nm;
        }
      }
    }
  if ( count > 0 )
    {
    this->m_CacheValuesPanel->SetBackgroundColour(wxColor(150, 150, 150));
    }
  else
    {
    this->m_CacheValuesPanel->SetBackgroundColour(*wxWHITE);
    }
  this->m_CacheValuesSizer->Layout();

  //max += 1;

  // Fix size
  int sx, sy;
  int total = max * count + 4;
  wxSize sizersize = this->m_CacheValuesSizer->GetSize();
  
  this->m_CacheValuesScroll->GetClientSize(&sx, &sy);
  wxSize size2 = this->m_CacheValuesPanel->GetSize();
  sy = size2.GetHeight();
  this->m_CacheValuesPanel->SetSize(wxSize(sx, height));
  this->m_CacheValuesSizer->Layout();
  this->m_CacheValuesScroll->SetScrollbars(0, 2, 0, height/2);
  
  this->ResizeInternal();

}

void cmMainFrame::RemoveAdvancedValues()
{

  cmCacheManager *cachem = this->m_CMakeInstance->GetCacheManager();
  cmMainFrame::CacheMapType* items = this->m_CacheEntries;
  
  for(cmCacheManager::CacheIterator i = cachem->NewIterator();
      !i.IsAtEnd(); i.Next())
    {
    const char* key = i.GetName();
    //const cmCacheManager::CacheEntry& value = i.GetEntry();
    if(i.GetPropertyAsBool("ADVANCED"))
      {
      cmCacheProperty* property = (*items)[key];
      property->Remove(this->m_CacheValuesSizer, this->m_CacheValuesPanel);
      }
    }

}

void cmMainFrame::ChangeWhereBuild()
{

  std::string path(this->m_PathBinary->GetValue().c_str());
  cmSystemTools::ConvertToUnixSlashes(path);
  std::string cache_file = path;
  cache_file += "/CMakeCache.txt";

  if ( !this->m_CMakeInstance )
    {

    return;
    }

  cmCacheManager *cache = this->m_CMakeInstance->GetCacheManager();
  cmCacheManager::CacheIterator it = cache->NewIterator();

  if (cmSystemTools::FileExists(cache_file.c_str()) &&
      cache->LoadCache(path.c_str()) &&
      it.Find("CMAKE_HOME_DIRECTORY"))
    {
    path = cmSystemTools::ConvertToOutputPath(it.GetValue());
      
    if ( this->SetSourceDir(path.c_str()) )
      {
      this->ChangeWhereSource();
      }
    }

  this->ClearCache();
  this->LoadCacheFromDiskToGUI();
  this->m_BuildPathChanged = true;  

}

void cmMainFrame::ChangeWhereSource()
{

  

}

bool cmMainFrame::SetSourceDir(const char* dir)
{
  if ( this->m_WhereSource == dir || strlen(dir) == 0 )
    {
    return false;
    }
  if ( this->m_PathSource->FindString(dir) < 0 )
    {
    this->m_PathSource->Append(dir);
    }
  this->m_PathSource->SetValue(dir);
  this->m_WhereSource = dir;
  this->m_Valid = false;

  return true;
}

bool cmMainFrame::SetBinaryDir(const char* dir)
{
  if ( this->m_WhereBuild == dir || strlen(dir) == 0 )
    {

    return false;
    }
  if ( this->m_PathBinary->FindString(dir) < 0 )
    {
    this->m_PathBinary->Append(dir);
    }
  this->m_PathBinary->SetValue(dir);
  this->m_WhereBuild = dir;
  this->m_Valid = false;

  return true;
}

void cmMainFrame::ClearCache()
{
  cmMainFrame::CacheMapType* items = this->m_CacheEntries;
  for(cmMainFrame::CacheMapType::iterator i = items->begin();
      i != items->end(); ++i)
    {
    cmCacheProperty* item = i->second;
    item->Remove(this->m_CacheValuesSizer, this->m_CacheValuesPanel);
    }

  items->erase(items->begin(), items->end());

}

void cmMainFrame::OnBinarySelected(wxCommandEvent&)
{
  if ( this->BuildDirectoryChanged() )
    {
    this->m_WhereBuild = this->GetBinaryDir();
    this->ChangeWhereBuild();
    }

}

void cmMainFrame::OnSourceSelected(wxCommandEvent&)
{
  if ( this->SourceDirectoryChanged() )
    {
    this->m_WhereSource = this->GetSourceDir();
    }

}

std::string cmMainFrame::GetBinaryDir()
{
  return std::string(this->m_PathBinary->GetValue().c_str());
}

std::string cmMainFrame::GetSourceDir()
{
  return std::string(this->m_PathSource->GetValue().c_str());
}

bool cmMainFrame::SourceDirectoryChanged()
{
  return (this->m_WhereSource != this->GetSourceDir());
}

bool cmMainFrame::BuildDirectoryChanged()
{
  return (this->m_WhereBuild != this->GetBinaryDir());
}

void cmMainFrame::OnRandomEvent(wxEvent& event)
{

  if ( event.GetEventType() == wxEVT_UPDATE_UI )
    {
    }
  else
    {
    std::cout << "Random event: " << event.GetEventType() << std::endl;
    }

}

void cmMainFrame::IgnoreCacheEntry(const char* key)
{
  std::cout << "IgnoreCacheEntry " << key << std::endl;
}

void cmMainFrame::RemoveCacheEntry(cmCacheProperty* cprop)
{
  this->m_Valid = false;
  this->m_CMakeInstance->GetCacheManager()->RemoveCacheEntry(cprop->GetName().c_str());
  cprop->MarkRemoved();
  this->m_EntryRemoved = true;
}

void cmMainFrame::HelpCacheEntry(const char* key, const char* help)
{
  wxMessageBox( help, key, wxICON_INFORMATION | wxOK );
}

void cmMainFrame::LoadFromRegistry()
{
//wxConfigBase *conf = (wxConfigBase*) wxConfigBase::Get();//new wxConfig("CMakeSetup");
  wxConfig *conf = new wxConfig("CMakeSetup");
  conf->SetPath("Settings/StartPath");

  int cc;
  char keyName[1024];
  wxString regvalue;
  for ( cc = 1; cc <= 10; cc ++ )
    {
    sprintf(keyName, "WhereSource%i", cc);
    regvalue = "";
    conf->Read(keyName, &regvalue);
    if ( regvalue.size() > 0 )
      {
      if ( cc == 1 )
        {
        this->SetSourceDir(regvalue.c_str());
        }
      else
        {
        this->m_PathSource->Append(regvalue);
        }
      }
    sprintf(keyName, "WhereBuild%i", cc);
    regvalue = "";
    conf->Read(keyName, &regvalue);
    if ( regvalue.size() > 0 )
      {
      if ( cc == 1 )
        {
        this->SetBinaryDir(regvalue.c_str());        
        }
      else
        {
        this->m_PathBinary->Append(regvalue);
        }
      }
    }
  
  delete conf;

}

void cmMainFrame::SaveToRegistry()
{

  //wxConfigBase *conf = (wxConfigBase*) wxConfigBase::Get();//new wxConfig("CMakeSetup");
  wxConfig *conf = new wxConfig("CMakeSetup");
  conf->SetPath("Settings/StartPath");

  wxString regvalue;
  if ( !conf->Read("WhereSource1", &regvalue) ) 
    {
    regvalue = "";
    }
  int shiftEnd = 9;
  if(this->m_WhereSource != regvalue.c_str())
    {
    char keyName[1024];
    char keyName2[1024];
    int i;
    for (i = 2; i < 10; ++i)
      {
      regvalue = "";
      sprintf(keyName,"WhereSource%i",i);      
      conf->Read(keyName, &regvalue);
      // check for short circuit, if the new value is already in
      // the list then we stop
      if (this->m_WhereSource == regvalue.c_str())
        {
        shiftEnd = i - 1;
        }
      }
      
    for (i = shiftEnd; i; --i)
      {
      regvalue = "";
      sprintf(keyName,"WhereSource%i",i);
      sprintf(keyName2,"WhereSource%i",i+1);
        
      conf->Read(keyName, &regvalue);
      if (strlen(regvalue.c_str()))
        {
        conf->Write(keyName2, wxString(regvalue.c_str()));
        }
      }
    conf->Write("WhereSource1", wxString(this->m_WhereSource.c_str()));
    }
    
  conf->Read("WhereBuild1", &regvalue);
  if(m_WhereBuild != regvalue.c_str())
    {
    int i;
    char keyName[1024];
    char keyName2[1024];
    for (i = 2; i < 10; ++i)
      {
      regvalue = "";
      sprintf(keyName,"WhereBuild%i",i);
      conf->Read(keyName, &regvalue);
      // check for short circuit, if the new value is already in
      // the list then we stop
      if (m_WhereBuild == regvalue.c_str())
        {
        shiftEnd = i - 1;
        }
      }
    for (i = shiftEnd; i; --i)
      {
      regvalue = "";
      sprintf(keyName,"WhereBuild%i",i);
      sprintf(keyName2,"WhereBuild%i",i+1);
        
      conf->Read(keyName, &regvalue);
      if (strlen(regvalue.c_str()))
        {
        conf->Write(keyName2, wxString(regvalue.c_str()));
        }
      }
    conf->Write("WhereBuild1", wxString(this->m_WhereBuild.c_str()));
    }
  delete conf;

}

void cmMainFrame::SetupStatusBarBinding(wxWindow* win)
{
  win->SetClientData(this);
  this->ConnectEventTo(win, wxEVT_MOTION,
                       (wxObjectEventFunction) &cmMainFrame::OnStatusBar);
}

void cmMainFrame::SetStatusText(const wxString& text, int number)
{
  this->wxFrame::SetStatusText("________________________________________________");
  this->wxFrame::SetStatusText(text, number);
}

void cmMainFrame::CursorBusy(bool s)
{
  wxSetCursor(*wxHOURGLASS_CURSOR);
  if ( s )
    {
    this->m_CursorChanged = true;
    }
}

void cmMainFrame::CursorNormal(bool s)
{
  wxSetCursor(*wxSTANDARD_CURSOR);
  if ( s )
    {
    this->m_CursorChanged = false;
    }
}

