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

// wxCMakeSetup.cxx : implementation file
//
#include "cmSystemTools.h"
#include "cmWXCommandLineInfo.h"
#include "cmWXMainFrame.h"

class wxCMakeSetup : public wxApp
{
public:
  virtual bool OnInit();
  virtual int OnExit();
};

class testFrame : public wxFrame
{
public:
  testFrame(const wxString& title, const wxSize& size)
    : wxFrame((wxFrame*)NULL, 0, title, wxDefaultPosition, size)
    {
      wxPanel *panel = new wxPanel(this, -1);
      wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
      wxWindow *value = new wxStaticText(panel, -1, "Some label");
      sizer->Add(value, 1, wxALIGN_CENTER_VERTICAL );
      value = new wxButton(panel, -1, "Button", wxDefaultPosition, wxSize(25, 0));
      sizer->Add(value, 0, wxALIGN_RIGHT);
      
      panel->SetAutoLayout( TRUE );
      panel->SetSizer(sizer);
      sizer->Fit(panel);
      sizer->SetSizeHints(panel);
      sizer->Layout();
    }
};

class testFrame1 : public wxFrame
{
public:
  testFrame1(const wxString& title, const wxSize& size)
    : wxFrame((wxFrame*)NULL, 0, title, wxDefaultPosition, size)
    {
      wxPanel *panel = new wxPanel(this, -1);
      panel->SetBackgroundColour(*wxRED);
      wxSizer *sizer = new wxFlexGridSizer(2, 5, 5);
      wxWindow *value = 0;
      int cc;
      for ( cc = 0; cc < 4; cc ++ )
        {
        char buffer[200];
        sprintf(buffer, "Long, Long Label; this label should be "
		"bigger than button %d",cc);
        value = new wxStaticText(panel, -1, buffer);
        sizer->Add(value, 1, wxGROW | wxALL );
        sprintf(buffer, "Button %d", cc);
        value = new wxButton(panel, -1, buffer);
        sizer->Add(value, 1, wxGROW | wxALL );
        }
      panel->SetAutoLayout( TRUE );
      panel->SetSizer(sizer);
      sizer->Fit(panel);
      sizer->SetSizeHints(panel);
      sizer->Layout();
    }
};

class testFrame2 : public wxFrame
{
public:
  testFrame2(const wxString& title, const wxSize& size)
    : wxFrame((wxFrame*)NULL, 0, title, wxDefaultPosition, size)
    {
      wxPanel *panel = new wxPanel(this, -1);
      panel->SetBackgroundColour(*wxRED);
      new wxTextCtrl(panel, -1, "Test", wxPoint(40, 5));
      new wxButton(panel, -1, "Test", wxPoint(-1, 5));
    }
};

class testFrame3 : public wxFrame
{
public:
  testFrame3(const wxString& title, const wxSize& size)
    : wxFrame((wxFrame*)NULL, 0, title, wxDefaultPosition, size)
    {
      this->CreateStatusBar();
      this->SetSizeHints(300, 300);
      wxPanel *panel = new wxPanel(this, -1);
      wxSizer *sizer = new wxFlexGridSizer(2, 5, 5);
      wxWindow *value = 0;
      int cc;
      for ( cc = 0; cc < 10; cc ++ )
        {
        char buffer[200];
        sprintf(buffer, "Label %d",cc);
        value = new wxStaticText(panel, -1, buffer);
        sizer->Add(value, 1, wxGROW | wxALL );
        sprintf(buffer, "Button %d", cc);
        value = new wxButton(panel, -1, buffer);
        sizer->Add(value, 1, wxGROW | wxALL );
        value->SetClientData(this);
        value->Connect(-1, wxEVT_MOTION,
                       (wxObjectEventFunction) &testFrame3::OnStatusBar);
        }
      panel->SetAutoLayout( TRUE );
      panel->SetSizer(sizer);
      sizer->Fit(panel);
      sizer->SetSizeHints(panel);
      sizer->Layout();
    }
  void OnStatusBar(wxEvent& event)
    {
      wxControl* eobject = static_cast<wxControl*>(event.GetEventObject());
      testFrame3* self = static_cast<testFrame3*>(eobject->GetClientData());
      wxString str;
      const char* chars = "|-\\/jg@_^";
      char ch = chars[((int)eobject)/1024 % strlen(chars)];
      int cc;
      for ( cc = 0; cc < 10; cc ++ )
        {
        str += ch;
        }
      self->SetStatusText(str);
    }
};

bool wxCMakeSetup::OnInit()
{
  cmSystemTools::DisableRunCommandOutput();
  cmCommandLineInfo cm;
  cm.SetValidArguments("ABGHQ");
  cm.ParseCommandLine(wxApp::argc, wxApp::argv);

  this->SetVendorName("Andy");
  this->SetAppName("CMakeSetup");

  cmMainFrame *frame = new cmMainFrame("CMake", wxSize(200, 100));
  frame->Initialize(&cm);
  //wxFrame *frame = new testFrame("CMake", wxSize(200, 100));
  //wxFrame *frame = new testFrame1("Frame", wxSize(200, 100));
  //wxFrame *frame = new testFrame2("Frame", wxSize(200, 100));
  //wxFrame *frame = new testFrame3("Frame", wxSize(200, 100));
  frame->Show(TRUE);
  this->SetTopWindow(frame);

  return TRUE;				       
}

int wxCMakeSetup::OnExit()
{
  // clean up: Set() returns the active config object as Get() does, but unlike
  // Get() it doesn't try to create one if there is none (definitely not what
  // we want here!)
  //delete wxConfigBase::Set((wxConfigBase *) NULL);

  return 0;
}

IMPLEMENT_APP(wxCMakeSetup)
