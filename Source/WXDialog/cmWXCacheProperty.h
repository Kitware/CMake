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
#ifndef CMCACHEPROPERTY_H
#define CMCACHEPROPERTY_H

#include "cmStandardIncludes.h"

#include "wxincludes.h"

class cmMainFrame;
class wxControl;
class wxPanel;
class wxSizer;
class wxWindow;

/** \class cmCacheProperty
 * \brief GUI Control class for cmake's cache property
 *
 * Stores cache property as displayed on GUI, caches its value, colors
 * red when new.
 * 
 */
class cmCacheProperty
{
public:
  cmCacheProperty(cmMainFrame*, const std::string& name);
  ~cmCacheProperty();
  const std::string& GetName() { return this->m_Name; }

  //! Get and set the value
  const std::string& GetValue() { return this->m_Value; }
  void SetValue(const std::string& s) { this->m_Value = s; }
  
  //! Get and set the help value
  void SetHelp(const std::string& s) { this->m_HelpString = s; }
  const std::string& GetHelp() { return this->m_HelpString; }

  //! Display the property in the window. Return the maximum height.
  int Display(wxSizer*, wxPanel*);

  //! Remove the property from the window
  void Remove(wxSizer*, wxPanel*);

  //! This method is called when property is changed
  void OnPropertyChanged(wxEvent& event);

  //! Mark cache entry as being removed.
  void MarkRemoved() { this->m_Removed = true; }

  //! Check if the entry was removed
  bool IsRemoved() { return this->m_Removed; }

  //! Get and set the new flag.
  void SetNewFlag(bool f) { this->m_NewValue = f; }
  bool GetNewFlag() { return this->m_NewValue; }

  //! Mark cache entry as being removed.
  void MarkAdvanced() { this->m_Advanced = true; }

  //! Check if the entry was removed
  bool IsAdvanced() { return this->m_Advanced; }

  //! Set item type
  void SetItemType(int t) { this->m_ItemType = t; }

  //! Get the main frame asociated with the cache property
  cmMainFrame* GetMainFrame() { return this->m_MainFrame; }

  enum ItemType
    {
      NOTHING = 0,
      EDIT,
      FILE,
      CHECKBOX,
      PATH      
    };

  enum 
    {
      Menu_Popup_Ignore = 200,
      Menu_Popup_Delete,
      Menu_Popup_Help
    };

protected:
  bool         m_NewValue;
  bool         m_Advanced;
  int          m_ItemType;

  wxWindow*    m_KeyWindow;
  wxWindow*    m_ValueWindow;

  std::string  m_Name;
  std::string  m_Value;
  std::string  m_HelpString;
  bool         m_Removed;

  //! The following methods set the events handling of widgets for the
  // cache property.
  void ConnectEvent(wxWindow* win, wxEventType et, wxObjectEventFunction func);
  void ConnectEventTo(wxWindow* win, wxEventType et, wxObjectEventFunction func);
  void SetupMenu(wxWindow* win);
  
  //! This are event callbacks for different events.
  void OnFileBrowseButton(wxEvent& event);
  void OnPathBrowseButton(wxEvent& event);
  void OnCheckboxButton(wxEvent& event);
  void OnEntryChanged(wxEvent& event);

private:
  cmMainFrame* m_MainFrame;
  wxControl*   m_TextControl;
};


#endif
