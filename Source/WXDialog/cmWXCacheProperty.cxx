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

// cmCacheProperty.cxx : implementation file
//

#include "cmWXCacheProperty.h"

#include "cmSystemTools.h"
#include "cmWXMainFrame.h"

static int GetClientHeight(wxWindow* w)
{
  wxSize size = w->GetSize();
  return size.GetHeight();
}

#define cmMAX(x, y) (((x)>(y))?(x):(y))

cmCacheProperty::cmCacheProperty(cmMainFrame* mf, const std::string& name) : m_Name(name)
{
  this->m_HelpString  = "";
  this->m_Value       = "";
  this->m_NewValue    = true;
  this->m_Removed     = false;
  this->m_ItemType    = cmCacheProperty::NOTHING;
  this->m_MainFrame   = mf;
  this->m_Advanced    = false;

  this->m_KeyWindow   = 0;
  this->m_ValueWindow = 0;

  this->m_TextControl = 0;
}

cmCacheProperty::~cmCacheProperty()
{
}

int cmCacheProperty::Display(wxSizer* s, wxPanel* win)
{
  int maxheight = 0;
  this->m_TextControl = 0;
  wxPanel* panel = new wxPanel(win, -1);  
  wxPanel* panel1 = new wxPanel(panel, -1);  
  wxBoxSizer* sizer = 0;
  wxColor bgcolor = panel->GetBackgroundColour();
  sizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* name = new wxStaticText(panel1, -1, this->m_Name.c_str());
  this->SetupMenu(name);
  this->SetupMenu(panel1);
  sizer1->Add(name, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  maxheight = cmMAX(maxheight, ::GetClientHeight(panel1));
  sizer->Add(5, 5, 0);
  sizer->Add(panel1, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

  panel1->SetAutoLayout( TRUE );
  panel1->SetSizer(sizer1);
  sizer1->Fit(panel1);
  sizer1->SetSizeHints(panel1);
  sizer1->Layout();  

  panel->SetAutoLayout( TRUE );
  panel->SetSizer(sizer);
  sizer->Fit(panel);
  sizer->SetSizeHints(panel);
  sizer->Layout();  
  wxControl* value = 0;
  if ( this->m_NewValue )
    {
    wxColor brightred = wxColor(252, 102, 100);
    panel->SetBackgroundColour(brightred);
    panel1->SetBackgroundColour(brightred);
    name->SetBackgroundColour(brightred);
    }
  else
    {
    panel->SetBackgroundColour(*wxWHITE);
    panel1->SetBackgroundColour(*wxWHITE);
    name->SetBackgroundColour(*wxWHITE);
    }
  
  this->m_KeyWindow = panel;

  panel = new wxPanel(win, -1);
  sizer = new wxBoxSizer(wxHORIZONTAL);
  panel->SetBackgroundColour(*wxWHITE);
  //panel->SetBackgroundColour(*wxGREEN)

#ifdef __APPLE__
  wxColor buttoncolor = *wxWHITE;
#else // __APPLE__
  wxColor buttoncolor = bgcolor;
#endif // __APPLE__

  switch ( this->m_ItemType )
    {
    case cmCacheProperty::CHECKBOX: 
      sizer->Add(5, 5, 0);
      value = new wxCheckBox(panel, -1, "");
      this->ConnectEvent(value, wxEVT_COMMAND_CHECKBOX_CLICKED,
                         (wxObjectEventFunction) &cmMainFrame::OnPropertyChanged);
      this->SetupMenu(value);
      if ( strcmp(this->GetValue().c_str(), "ON") == 0 )
        {
        static_cast<wxCheckBox*>(value)->SetValue(true);
        }
      break;
    case cmCacheProperty::EDIT:
      value = new wxTextCtrl(panel, -1, this->m_Value.c_str());
      maxheight = cmMAX(maxheight, ::GetClientHeight(value));
      this->ConnectEvent(value, wxEVT_COMMAND_TEXT_UPDATED,
                         (wxObjectEventFunction) &cmMainFrame::OnPropertyChanged);
      this->SetupMenu(value);
      break;
    case cmCacheProperty::FILE:
      sizer->Add(5, 5, 0);
      value = new wxStaticText(panel, -1, this->m_Value.c_str());
      maxheight = cmMAX(maxheight, ::GetClientHeight(value));
      sizer->Add(value, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL );
      this->m_TextControl = value;
      this->SetupMenu(value);
      value = new wxButton(panel, -1, "...", wxDefaultPosition, 
                           wxSize(20, maxheight - 4));
      maxheight = cmMAX(maxheight, ::GetClientHeight(value));
      value->SetBackgroundColour(buttoncolor);
      sizer->Add(value, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL );
      this->ConnectEvent(value, wxEVT_COMMAND_BUTTON_CLICKED,
                         (wxObjectEventFunction) &cmMainFrame::OnPropertyChanged);
      this->SetupMenu(value);
      value = 0;
      break;
    case cmCacheProperty::PATH:
      sizer->Add(5, 5, 0);
      value = new wxStaticText(panel, -1, this->m_Value.c_str());
      maxheight = cmMAX(maxheight, ::GetClientHeight(value));
      sizer->Add(value, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL );
      this->m_TextControl = value;
      this->SetupMenu(value);
      value = new wxButton(panel, -1, "...", wxDefaultPosition, 
                           wxSize(20, maxheight - 4));
      maxheight = cmMAX(maxheight, ::GetClientHeight(value));
      value->SetBackgroundColour(buttoncolor);
      sizer->Add(value, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL );
      this->ConnectEvent(value, wxEVT_COMMAND_BUTTON_CLICKED,
                         (wxObjectEventFunction) &cmMainFrame::OnPropertyChanged);
      this->SetupMenu(value);
      value = 0;
      break;
    default:
      value = new wxStaticText(panel, -1, this->m_Value.c_str());
      maxheight = cmMAX(maxheight, ::GetClientHeight(value));
      this->m_TextControl = value;
      break;
    }
  //panel->Fit();
  this->m_ValueWindow = panel;
  //panel->Fit();
  //win->Fit();
  if ( value )
    {
    sizer->Add(value, 1, wxALIGN_LEFT | wxGROW | wxALL | wxALIGN_CENTER_VERTICAL);
    }

  //s->Layout();

  panel->SetAutoLayout( TRUE );
  panel->SetSizer(sizer);
  sizer->Fit(panel);
  sizer->SetSizeHints(panel);
  sizer->Layout();


  if ( this->m_NewValue )
    {
    s->Prepend(this->m_ValueWindow, 1, wxGROW | wxLEFT | wxRIGHT );
    s->Prepend(this->m_KeyWindow, 1, wxGROW | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL );
    }
  else
    {
    s->Add(this->m_KeyWindow, 1, wxGROW | wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL );
    s->Add(this->m_ValueWindow, 1, wxGROW | wxLEFT | wxRIGHT );
    }

  this->SetupMenu(this->m_KeyWindow);
  this->SetupMenu(this->m_ValueWindow);
  
  int x1, x2, xm, s1, s2, sm;
  win->GetSize(&xm, &sm);
  this->m_KeyWindow->GetSize(&x1, &s1);
  this->m_ValueWindow->GetSize(&x2, &s2);
  int m = s1;
  if ( s2 > m )
    {
    m = s2;
    }
  this->m_KeyWindow->SetSize(x1, m);
  this->m_ValueWindow->SetSize(x2, m);
  //std::cout << "Size of panels: " << sm << "," << s1 << ", " << s2 << " max: " << maxheight<< std::endl;
  return maxheight;
}

void cmCacheProperty::Remove(wxSizer* sizer, wxPanel*)
{
  if ( this->m_KeyWindow )
    {
    sizer->Remove(this->m_KeyWindow);
    this->m_KeyWindow->Destroy();
    }
  if ( this->m_ValueWindow )
    {
    sizer->Remove(this->m_ValueWindow);
    this->m_ValueWindow->Destroy();
    }
  this->m_KeyWindow = 0;
  this->m_ValueWindow = 0;
  //sizer->Layout();
  //win->Fit();
}

void cmCacheProperty::ConnectEvent(wxWindow* win, wxEventType et, wxObjectEventFunction func)
{
  if ( !this->m_MainFrame )
    {
    return;
    }
  win->SetClientData(this);
  this->m_MainFrame->Connect(win->GetId(), et, func);
}

void cmCacheProperty::ConnectEventTo(wxWindow* win, wxEventType et, 
                                     wxObjectEventFunction func)
{
  if ( !this->m_MainFrame )
    {
    return;
    }
  win->SetClientData(this);
  this->m_MainFrame->ConnectEventTo(win, et, func);
}

void cmCacheProperty::OnPropertyChanged(wxEvent& event)
{
  if ( event.GetEventType() == wxEVT_RIGHT_DOWN )
    {
    }
  else
    {
    switch ( this->m_ItemType )
      {
      case cmCacheProperty::EDIT: this->OnEntryChanged(event); break;
      case cmCacheProperty::FILE: this->OnFileBrowseButton(event); break;
      case cmCacheProperty::CHECKBOX: this->OnCheckboxButton(event); break;
      case cmCacheProperty::PATH: this->OnPathBrowseButton(event); break;
      }
    }
}

void cmCacheProperty::OnFileBrowseButton(wxEvent&)
{
  std::string path = cmSystemTools::GetFilenamePath(this->m_Value);
  std::string file = cmSystemTools::GetFilenameName(this->m_Value);

  if ( path == "NOTFOUND" )
    {
    path = this->m_MainFrame->GetBuildDir();
    }
  
  wxFileDialog dialog (
    this->m_MainFrame,
    _T("Select file"),
    path.c_str(),
    file.c_str(),
    _T("All files|*.*")
    );

  if (dialog.ShowModal() == wxID_OK)
    {
    std::string str = "";
    if ( this->m_TextControl )
      {
      str += dialog.GetPath().c_str();
      static_cast<wxStaticText*>(this->m_TextControl)->SetLabel(str.c_str());
      }
    this->SetValue(str.c_str());
    }
}

void cmCacheProperty::OnPathBrowseButton(wxEvent&)
{
  std::string path = this->m_Value;
  if ( path == "NOTFOUND" )
    {
    path = this->m_MainFrame->GetBuildDir();
    }

  wxDirDialog dialog ( this->m_MainFrame, _T("Select directory"), path.c_str() );

  if (dialog.ShowModal() == wxID_OK)
    {
    if ( this->m_TextControl )
      {
      static_cast<wxStaticText*>(this->m_TextControl)->SetLabel(dialog.GetPath());
      }
    this->SetValue(dialog.GetPath().c_str());
    }
}

void cmCacheProperty::OnCheckboxButton(wxEvent& event)
{
  wxCheckBox* widget = static_cast<wxCheckBox*>( event.GetEventObject() );
  if ( !widget )
    {
    return;
    }
  int val = widget->GetValue();
  if ( val )
    {
    this->SetValue("ON");
    }
  else
    {
    this->SetValue("OFF");
    }
}

void cmCacheProperty::OnEntryChanged(wxEvent& event)
{
  wxTextCtrl* widget = static_cast<wxTextCtrl*>( event.GetEventObject() );
  if ( !widget )
    {
    return;
    }
  this->SetValue(static_cast<const char*>(widget->GetValue()));
}

void cmCacheProperty::SetupMenu(wxWindow* win)
{
  this->ConnectEventTo(win, wxEVT_RIGHT_DOWN,
                       (wxObjectEventFunction) &cmMainFrame::OnPopupMenu);
  this->ConnectEventTo(win, wxEVT_MOTION,
                       (wxObjectEventFunction) &cmMainFrame::OnCacheStatusBar);
}
