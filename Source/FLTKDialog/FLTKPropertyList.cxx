// FLTKPropertyList.cxx : implementation file
//

#include "FLTKPropertyList.h"
#include "../cmCacheManager.h"
#include "FLTKPropertyItemRow.h"
#include "FL/filename.H"
#include "FL/fl_file_chooser.H"
#include "FL/Fl_Color_Chooser.H"
#include "FL/fl_ask.H"
#include "FL/Fl_Button.H"
#include "CMakeSetupGUIImplementation.h"


namespace fltk {

/////////////////////////////////////////////////////////////////////////////
// PropertyList

PropertyList::PropertyList( CMakeSetupGUIImplementation * cmakeSetup )
{
  m_CMakeSetup = cmakeSetup;
  PropertyItemRow::SetCMakeSetupGUI( cmakeSetup );
  m_Dirty = false;
}



PropertyList::~PropertyList()
{
  for(std::set<PropertyItem*>::iterator i = m_PropertyItems.begin();
      i != m_PropertyItems.end(); ++i)
    {
    delete *i;
    }
}




int PropertyList::AddItem( std::string txt)
{
  int nIndex =0;
  return nIndex;
}



int PropertyList::AddPropItem(PropertyItem* pItem, bool reverseOrder)
{

  int nIndex =0; 
  if(reverseOrder)
    {
    nIndex = 0;
    }
  else
    {
    nIndex = m_PropertyItems.size();
    }

  new PropertyItemRow( pItem ); // GUI of the new property row

  m_PropertyItems.insert(pItem);

  return nIndex;
}



int PropertyList::AddProperty(const char* name,
                               const char* value,
                               const char* helpString,
                               int type,
                               const char* comboItems,
                               bool reverseOrder)
{ 

  PropertyItem* pItem = 0;
  for(int i =0; i < this->GetCount(); ++i)
    {
    PropertyItem* item = this->GetItem(i);
    if(item->m_propName == name)
      {
      pItem = item;
      if(pItem->m_curValue != value)
        {
        pItem->m_curValue = value;
        pItem->m_HelpString = helpString;
        Invalidate();
        }
      return i;
      }
    }
  // if it is not found, then create a new one
  if(!pItem)
    {
    pItem = new PropertyItem(name, value, helpString, type, comboItems);
    }
  return this->AddPropItem(pItem,reverseOrder);
}


void PropertyList::RemoveProperty(const char* name)
{
  for(int i =0; i < this->GetCount(); ++i)
    {
    PropertyItem* pItem = (PropertyItem*) GetItemDataPtr(i);
    if(pItem->m_propName == name)
      {
      m_PropertyItems.erase(pItem);
      delete pItem; 
      return;
      }
    }
}



void PropertyList::RemoveAll()
{
  int c = this->GetCount();
  for(int i =0; i < c; ++i)
    {
    PropertyItem* pItem = (PropertyItem*) GetItemDataPtr(0);
    cmCacheManager::GetInstance()->RemoveCacheEntry(pItem->m_propName.c_str());
    m_PropertyItems.erase(pItem);
    delete pItem;
    }
  Invalidate();
}



PropertyItem * PropertyList::GetItemDataPtr(int index)
{
    std::set<PropertyItem*>::iterator it =  m_PropertyItems.begin();
    for(int i=0; it != m_PropertyItems.end() && i<index; i++) 
    {
      ++it;
    }
    return *it;
}


PropertyItem * PropertyList::GetItem(int index)
{
    std::set<PropertyItem*>::iterator it =  m_PropertyItems.begin();
    for(int i=0; it != m_PropertyItems.end() && i<index; i++) 
    {
      ++it;
    }
    return *it;
}

void
PropertyList
::InvalidateList(void)
{
  Invalidate();
  m_Dirty = true;
}


} // end fltk namespace



