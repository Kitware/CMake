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
#ifndef FLTKPROPERTYLIST_H
#define FLTKPROPERTYLIST_H

#include "../cmStandardIncludes.h"
#include <string>


class CMakeSetupGUIImplementation;  


namespace fltk {


/////////////////////////////////////////////////////////////////////////////
//PropertyList Items
class PropertyItem 
{
// Attributes
public:
  std::string m_HelpString;
  std::string m_propName;
  std::string m_curValue;
  int m_nItemType;
  std::string m_cmbItems;
  bool m_Removed;
  bool m_NewValue;
  bool m_Dirty;
public:
  PropertyItem( std::string propName, 
                std::string curValue,
                std::string helpString,
                int nItemType, 
                std::string cmbItems )
    {
      m_HelpString  = helpString;
      m_propName    = propName;
      m_curValue    = curValue;
      m_nItemType   = nItemType;
      m_cmbItems    = cmbItems;
      m_Removed     = false;
      m_NewValue    = true;
      m_Dirty       = false;
    }
};





/////////////////////////////////////////////////////////////////////////////
// PropertyList window

class PropertyList 
{
// Construction
public:
  enum ItemType 
    {
      COMBO = 0,
      EDIT,
      COLOR,
      FONT,
      FILE,
      CHECKBOX,
      PATH
    };

  PropertyList( CMakeSetupGUIImplementation * );
  
// Attributes
public:

// Operations
public:
  int AddItem( std::string txt );
  int AddProperty(const char* name,
                  const char* value,
                  const char* helpString,
                  int type,
                  const char* comboItems,
                  bool reverseOrder);
  void RemoveProperty(const char* name);
  std::set<PropertyItem*> & GetItems() 
    {
      return m_PropertyItems;
    }

  void InvalidateList(void);
  void Invalidate(void) 
  {
    // fltk redraw();
  }
  
  int GetCount(void) const 
  {
    return m_PropertyItems.size();
  }
  void OnButton(void);
  void OnHelp(void);
  void RemoveAll();
  PropertyItem* GetItem(int index);
  PropertyItem* GetItemDataPtr(int m_curSel);

  void ClearDirty(void)    { m_Dirty = false; }
  void SetDirty(void)      { m_Dirty = true;  }
  bool IsDirty(void) const { return m_Dirty;  }

// Implementation
public:
  virtual ~PropertyList();

protected:

  int AddPropItem(PropertyItem* pItem,bool reverseOrder);

  std::set<PropertyItem*> m_PropertyItems;

  CMakeSetupGUIImplementation * m_CMakeSetup;

  bool            m_Dirty;

};


} // end namespace fltk

#endif 
