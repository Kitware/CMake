// FLTKPropertyList.cxx : implementation file
//

#include "FLTKPropertyList.h"
#include "../cmCacheManager.h"
#include "FLTKPropertyItemRow.h"
#include "Fl/filename.H"
#include "Fl/fl_file_chooser.H"
#include "Fl/Fl_Color_Chooser.H"
#include "Fl/fl_ask.H"
#include "Fl/Fl_Button.H"
#include <cstdio>

namespace fltk {

/////////////////////////////////////////////////////////////////////////////
// PropertyList

PropertyList::PropertyList()
{
  m_Dirty = false;
  m_curSel = -1;
}

PropertyList::~PropertyList()
{
  for(std::set<PropertyItem*>::iterator i = m_PropertyItems.begin();
      i != m_PropertyItems.end(); ++i)
    {
    delete *i;
    }
}




int PropertyList::AddItem(string txt)
{
  int nIndex =0;// = AddString(txt);
  return nIndex;
}

int PropertyList::AddPropItem(PropertyItem* pItem)
{

  int nIndex =0; //= AddString(_T(""));
  // SetItemDataPtr(nIndex,pItem);

  new PropertyItemRow( pItem ); // GUI of the property row

  m_PropertyItems.insert(pItem);

  return nIndex;
}

int PropertyList::AddProperty(const char* name,
                               const char* value,
                               const char* helpString,
                               int type,
                               const char* comboItems)
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
        m_Dirty = true;
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
  return this->AddPropItem(pItem);
}




void PropertyList::OnButton()
{
  PropertyItem* pItem = (PropertyItem*) GetItemDataPtr(m_curSel);

  //display the appropriate common dialog depending on what type
  //of chooser is associated with the property
  if (pItem->m_nItemType == PropertyList::COLOR)
    {
      unsigned char red   = 0;
      unsigned char blue  = 0;
      unsigned char green = 0;
      fl_color_chooser("Please pick a color",red,green,blue);
      char buffer[300];
      sprintf(buffer,"RGB(%d,%d,%d)",red,green,blue);
      pItem->m_curValue = buffer;
      m_Dirty = true;
      Invalidate();
    }
  else if (pItem->m_nItemType == PropertyList::FILE)
    {
    string currPath   = pItem->m_curValue;

    const char * SelectedFile 
                    =  fl_file_chooser("Choose a file",
                             "*",currPath.c_str() );

    if( SelectedFile )
      {
        pItem->m_curValue = SelectedFile;
        m_Dirty = true;
        Invalidate();
      }
    }
   else if (pItem->m_nItemType == PropertyList::PATH)
    {
    string currPath   = pItem->m_curValue;
    string initialDir = currPath;
    
    const char * SelectedFile 
                    =  fl_file_chooser("Choose a directory",
                             "*/",initialDir.c_str() );

    if( SelectedFile   && filename_isdir( SelectedFile ) )
      {
      pItem->m_curValue = SelectedFile;
      m_Dirty = true;
      Invalidate();
      }
    }
  else if (pItem->m_nItemType == PropertyList::FONT)
    {	
    }
}




void PropertyList::OnHelp()
{ 
  if(m_curSel == -1 || this->GetCount() <= 0)
    {
    return;
    }
  PropertyItem* pItem = (PropertyItem*) GetItemDataPtr(m_curSel);
  fl_message(pItem->m_HelpString.c_str());
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
    // this->DeleteString(0);
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



} // end fltk namespace



