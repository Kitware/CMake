#ifndef FLTKPROPERTYLIST_H
#define FLTKPROPERTYLIST_H

#include "../cmStandardIncludes.h"
#include <string>


namespace fltk {

using std::string;

/////////////////////////////////////////////////////////////////////////////
//PropertyList Items
class PropertyItem
{
// Attributes
public:
  string m_HelpString;
  string m_propName;
  string m_curValue;
  int m_nItemType;
  string m_cmbItems;
  bool m_Removed;
public:
  PropertyItem(string propName, string curValue,
                string helpString,
                int nItemType, string cmbItems)
    {
      m_HelpString = helpString;
      m_Removed = false;
      m_propName = propName;
      m_curValue = curValue;
      m_nItemType = nItemType;
      m_cmbItems = cmbItems;
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
  PropertyList();
  
// Attributes
public:

// Operations
public:
  int AddItem(string txt);
  int AddProperty(const char* name,
                  const char* value,
                  const char* helpString,
                  int type,
                  const char* comboItems);
  std::set<PropertyItem*> GetItems() 
    {
      return m_PropertyItems;
    }
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

// Implementation
public:
  virtual ~PropertyList();

  // Generated message map functions
protected:

  int AddPropItem(PropertyItem* pItem);

//  CComboBox m_cmbBox;
//  CEdit m_editBox;
//  CButton m_btnCtrl;
//  CButton m_CheckBoxControl;
  

  bool m_Dirty;
  int m_curSel;
  int m_prevSel;
  int m_nDivider;
  int m_nDivTop;
  int m_nDivBtm;
  int m_nOldDivX;
  int m_nLastBox;

  std::set<PropertyItem*> m_PropertyItems;

};


} // end namespace fltk

#endif 
