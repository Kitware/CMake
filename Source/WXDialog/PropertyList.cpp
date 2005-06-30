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

/* for compilers that support precompilation
   includes "wx/wx.h" */

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "PropertyList.h"
#include "app_resources.h"

#include "../cmCacheManager.h"
#include "../cmSystemTools.h"
#include "../cmake.h"

BEGIN_EVENT_TABLE( wxPropertyList, wxGrid )
    EVT_GRID_CELL_LEFT_CLICK( wxPropertyList::OnSelectCell )
    EVT_GRID_CELL_CHANGE( wxPropertyList::OnCellChange )
    EVT_GRID_CMD_CELL_RIGHT_CLICK( wxID_ANY, wxPropertyList::OnCellPopup )
    EVT_MENU ( ID_CACHE_DELETE, wxPropertyList::OnDeleteCache )
    EVT_MENU ( ID_CACHE_IGNORE, wxPropertyList::OnIgnoreCache )
    EVT_MENU ( ID_CACHE_BROWSE, wxPropertyList::OnBrowseItem )
    EVT_SIZE ( wxPropertyList::OnSizeGrid )
    EVT_CHAR ( wxPropertyList::OnKeyPressed )
END_EVENT_TABLE()

#if 0

// ----------------------------------------------------------------------------
// wxGridCellPathEditor
// ----------------------------------------------------------------------------

wxGridCellPathEditor::wxGridCellPathEditor()
{
    m_maxChars = 0;
}

void wxGridCellPathEditor::Create(wxWindow* parent,
                                  wxWindowID id,
                                  wxEvtHandler* evtHandler)
{   
    m_control = new wxTextCtrl(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize
#if defined(__WXMSW__)
                               , wxTE_PROCESS_TAB | wxTE_AUTO_SCROLL
#endif
                              );
//
//  m_button = new wxButton(parent, id+1, _("..."), 
//                          wxDefaultPosition, wxDefaultSize);
//

    // set max length allowed in the textctrl, if the parameter was set
    if (m_maxChars != 0)
    {
        ((wxTextCtrl*)m_control)->SetMaxLength(m_maxChars);
    }

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellPathEditor::PaintBackground(const wxRect& WXUNUSED(rectCell),
                                           wxGridCellAttr * WXUNUSED(attr))
{
    // as we fill the entire client area, don't do anything here to minimize
    // flicker
}

void wxGridCellPathEditor::SetSize(const wxRect& rectOrig)
{
    wxRect rect(rectOrig);

    // Make the edit control large enough to allow for internal
    // margins
    //
    // TODO: remove this if the text ctrl sizing is improved esp. for
    // unix
    //
#if defined(__WXGTK__)
    if (rect.x != 0)
    {
        rect.x += 1;
        rect.y += 1;
        rect.width -= 1;
        rect.height -= 1;
    }
#else // !GTK
    int extra_x = ( rect.x > 2 )? 2 : 1;

// MB: treat MSW separately here otherwise the caret doesn't show
// when the editor is in the first row.
#if defined(__WXMSW__)
    int extra_y = 2;
#else
    int extra_y = ( rect.y > 2 )? 2 : 1;
#endif // MSW

#if defined(__WXMOTIF__)
    extra_x *= 2;
    extra_y *= 2;
#endif
    rect.SetLeft( wxMax(0, rect.x - extra_x) );
    rect.SetTop( wxMax(0, rect.y - extra_y) );
    rect.SetRight( rect.GetRight() + 2*extra_x );
    rect.SetBottom( rect.GetBottom() + 2*extra_y );
#endif // GTK/!GTK

    wxGridCellEditor::SetSize(rect);
}

void wxGridCellPathEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEditor must be Created first!"));

    m_startValue = grid->GetTable()->GetValue(row, col);

    DoBeginEdit(m_startValue);
}

void wxGridCellPathEditor::DoBeginEdit(const wxString& startValue)
{
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
    Text()->SetSelection(-1,-1);
    Text()->SetFocus();
}

bool wxGridCellPathEditor::EndEdit(int row, int col,
                                   wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEditor must be Created first!"));

    bool changed = false;
    wxString value = Text()->GetValue();
    if (value != m_startValue)
        changed = true;

    if (changed)
        grid->GetTable()->SetValue(row, col, value);

    m_startValue = wxEmptyString;
    // No point in setting the text of the hidden control
    //Text()->SetValue(m_startValue);

    return changed;
}


void wxGridCellPathEditor::Reset()
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEditor must be Created first!"));

    DoReset(m_startValue);
}

void wxGridCellPathEditor::DoReset(const wxString& startValue)
{
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
}

bool wxGridCellPathEditor::IsAcceptedKey(wxKeyEvent& event)
{
    return wxGridCellEditor::IsAcceptedKey(event);
}

void wxGridCellPathEditor::StartingKey(wxKeyEvent& event)
{
    // Since this is now happening in the EVT_CHAR event EmulateKeyPress is no
    // longer an appropriate way to get the character into the text control.
    // Do it ourselves instead.  We know that if we get this far that we have
    // a valid character, so not a whole lot of testing needs to be done.

    wxTextCtrl* tc = Text();
    wxChar ch;
    long pos;
    
#if wxUSE_UNICODE
    ch = event.GetUnicodeKey();
    if (ch <= 127)
        ch = (wxChar)event.GetKeyCode();
#else
    ch = (wxChar)event.GetKeyCode();
#endif
    switch (ch)
    {
        case WXK_DELETE:
            // delete the character at the cursor
            pos = tc->GetInsertionPoint();
            if (pos < tc->GetLastPosition())
                tc->Remove(pos, pos+1);
            break;

        case WXK_BACK:
            // delete the character before the cursor
            pos = tc->GetInsertionPoint();
            if (pos > 0)
                tc->Remove(pos-1, pos);
            break;

        default:
            tc->WriteText(ch);
            break;
    }
}

void wxGridCellPathEditor::HandleReturn( wxKeyEvent &event )
{
#if defined(__WXMOTIF__) || defined(__WXGTK__)
    // wxMotif needs a little extra help...
    size_t pos = (size_t)( Text()->GetInsertionPoint() );
    wxString s( Text()->GetValue() );
    s = s.Left(pos) + wxT("\n") + s.Mid(pos);
    Text()->SetValue(s);
    Text()->SetInsertionPoint( pos );
#else
    // the other ports can handle a Return key press
    //
    event.Skip();
#endif
}

void wxGridCellPathEditor::SetParameters(const wxString& params)
{
    if ( !params )
    {
        // reset to default
        m_maxChars = 0;
    }
    else
    {
        long tmp;
        if ( !params.ToLong(&tmp) )
        {
            wxLogDebug(_T("Invalid wxGridCellPathEditor parameter string '%s' ignored"), params.c_str());
        }
        else
        {
            m_maxChars = (size_t)tmp;
        }
    }
}

// return the value in the text control
wxString wxGridCellPathEditor::GetValue() const
{
  return Text()->GetValue();
}

#endif

/////////////////////////////////////////////////////////////////////////////
// wxPropertyItem

// returns true when this property item is a filepath
bool wxPropertyItem::IsFilePath()
{
    return m_nItemType == wxPropertyList::FILE;
}

// returns true when this property item is a dir path
bool wxPropertyItem::IsDirPath()
{
    return m_nItemType == wxPropertyList::PATH;
}

/////////////////////////////////////////////////////////////////////////////
// wxPropertyList

wxPropertyList::~wxPropertyList()
{
    WX_CLEAR_ARRAY(m_PropertyItems);
}

int wxPropertyList::AddItem(const wxString &txt)
{
    // TODO: Add the item to the grid!
    
    //int nIndex = AddString(txt);
    //return nIndex;

    return 0;
}

// order = 0 sorted (not supported yet)
// order = 1 add to top
// order = 2 add to bottom
int wxPropertyList::AddPropItem(wxPropertyItem* pItem, int order)
{
    m_PropertyItems.Add(pItem);
    if(pItem->GetAdvanced() && ! m_ShowAdvanced)
        return 0;

    // disable in progress editing
    HideControls();
        
    return AddPropertyToGrid(pItem, order);
}

int wxPropertyList::AddPropertyToGrid(wxPropertyItem *pItem, int order)
{
    int row = 0;
    if(order == 1)
        InsertRows(0,1);
    else 
    {
        AppendRows(1);    
        row = GetNumberRows() - 1;
    }
    
    // initialise the type of renderer
    if(pItem->GetItemType() == wxPropertyList::CHECKBOX)
    {
        SetCellRenderer(row, 1, new wxGridCellBoolRenderer);
        SetCellEditor(row, 1, new wxGridCellBoolEditor);
    }

#ifdef __LINUX__
    // fix to make sure scrollbars are drawn properly
    wxGrid::AdjustScrollbars();
#endif
    
    // the property display is read only
    UpdatePropertyItem(pItem, row);
    return row;
}

void wxPropertyList::AddProperty(const char* name, const char* value, const char* helpString,
                                int type, const char* comboItems, bool reverseOrder, bool advanced)
{ 
    wxPropertyItem* pItem = 0; 
   
    // add or update the property item
    for(size_t i = 0; i < m_PropertyItems.Count(); i++)
    {
        if(m_PropertyItems[i]->GetPropName().IsSameAs(name))
        {
            pItem = m_PropertyItems[i];
            if(!pItem->GetCurValue().IsSameAs(value))
            {
                pItem->SetCurValue(value);
                pItem->SetHelpString(helpString);
                pItem->SetAdvanced(advanced);
            
                // update the property item
                int row = FindProperty(pItem);
                if(row != -1)
                    UpdatePropertyItem(pItem, row);         
            }
            return;
        }
    }

    // if it is not found, then create a new one
    if(!pItem)
    {
        pItem = new wxPropertyItem(name, value, helpString, type, comboItems);
        pItem->SetAdvanced(advanced);
  
        AddPropItem(pItem, 1);
    }
}

void wxPropertyList::UpdateGridView()
{
    // make sure all items are shown, remove items that should not be shown
    bool keepItem;
    int row;
    for(size_t i = 0; i < m_PropertyItems.Count(); i++)
    {
        // to begin with, does this item fit the query?
        keepItem = m_strQuery.IsEmpty() || (m_PropertyItems[i]->GetPropName().Find(m_strQuery) != -1);
        if(keepItem)
        {
            // when advanced items are allowed to be shown, keep when ok
            if(!m_ShowAdvanced)
                keepItem = !m_PropertyItems[i]->GetAdvanced();
        }

        // find the item, if not present but keep is true, add, if 
        // present but keep is false, remove
        row = -1;
        for(size_t j = 0; j < (size_t)GetNumberRows(); j++)
        {
            if(m_PropertyItems[i]->GetPropName().IsSameAs(GetCellValue(j, 0)))
            {
                row = j;
                break;
            }
        }

        if(row == -1 && keepItem)
            AddPropertyToGrid(m_PropertyItems[i], (m_ShowAdvanced ? 2 : 0));                
        else if(row != -1 && !keepItem)
            DeleteRows(row, 1);
    }

#ifdef __LINUX__
    // fix to make sure scrollbars are drawn properly
    wxGrid::AdjustScrollbars();
#endif
}

void wxPropertyList::HideControls()
{
    DisableCellEditControl();
}

void wxPropertyList::RemoveProperty(wxPropertyItem *pItem)
{
    HideControls();
  
    // look for property in grid, delete it when present        
    for(size_t j = 0; j < (size_t)GetNumberRows(); j++)
    {
        if(pItem->GetPropName().IsSameAs(GetCellValue(j, 0), false))
        {
            DeleteRows(j, 1);                   
            
#ifdef __LINUX__
            // fix to make sure scrollbars are drawn properly
            wxGrid::AdjustScrollbars();
#endif
             break;
        }
    }

    // delete the item from the list
    m_PropertyItems.Remove(pItem);
    delete pItem;
}

wxPropertyItem *wxPropertyList::FindPropertyByName(const wxString &name)
{
    for(size_t i = 0; i < m_PropertyItems.Count(); i++)
    {
        // we have an advanced item, go through table and if not present, show it
        if(m_PropertyItems[i]->GetPropName().IsSameAs(name, true))
            return m_PropertyItems[i];
    }

    return 0;   
}

/**
void wxPropertyList::OnIgnore()
{
  if(m_curSel == -1 || this->GetCount() <= 0)
    {
    return;
    }
  wxPropertyItem* pItem = (wxPropertyItem*) GetItemDataPtr(m_curSel);
  pItem->m_curValue = "IGNORE";
  InvalidateList();
}
*/

/**
void wxPropertyList::OnDelete()
{ 
  if(m_curSel == -1 || this->GetCount() <= 0)
    {
    return;
    }
  wxPropertyItem* pItem = (wxPropertyItem*) GetItemDataPtr(m_curSel);
  m_CMakeSetupDialog->GetCMakeInstance()->GetCacheManager()->RemoveCacheEntry(pItem->m_propName);
  m_PropertyItems.erase(pItem);
  delete pItem; 
  this->DeleteString(m_curSel);
  this->HideControls();
  this->SetTopIndex(0);
  InvalidateList();
}
*/

/**
void wxPropertyList::OnHelp()
{ 
  if(m_curSel == -1 || this->GetCount() <= 0)
    {
    return;
    }
  wxPropertyItem* pItem = (wxPropertyItem*) GetItemDataPtr(m_curSel);
  MessageBox(pItem->m_HelpString, pItem->m_propName, MB_OK|MB_ICONINFORMATION);
}
*/

void wxPropertyList::RemoveAll()
{   
    WX_CLEAR_ARRAY(m_PropertyItems);
    m_generatedProjects = false;

    if(GetNumberRows() > 0) 
        DeleteRows(0, GetNumberRows());
    
    m_strQuery.Empty();

#ifdef __LINUX__
    // fix to make sure scrollbars are drawn properly
    wxGrid::AdjustScrollbars();
#endif
}

void wxPropertyList::ShowAdvanced()
{
    // set flag in the control
    m_ShowAdvanced = true; 
    UpdateGridView();
}


void wxPropertyList::HideAdvanced()
{
    // set flag in the control
    m_ShowAdvanced = false; 
    UpdateGridView();   
}

int wxPropertyList::FindProperty(wxPropertyItem *pItem)
{
    if(GetNumberRows() > 0 && pItem != 0) 
    {
        // find the property the traditional way
        for(size_t j = 0; j < (size_t)GetNumberRows(); j++)
        {
            if(pItem->GetPropName().IsSameAs(GetCellValue(j, 0)))
                return j;
        }
    }

    return -1;
}

wxPropertyItem *wxPropertyList::GetPropertyItemFromRow(int row)
{
    if(row < GetNumberRows() && row >= 0) 
    {
        wxString str = GetCellValue(row, 0);
        // find the property the traditional way
        for(size_t i = 0; i < (size_t)m_PropertyItems.Count(); i++)
        {
            if(m_PropertyItems[i]->GetPropName().IsSameAs(str))
                return m_PropertyItems[i];
        }
    }

    return 0;   
}

bool wxPropertyList::UpdatePropertyItem(wxPropertyItem *pItem, int row)
{
    wxCHECK(row < GetNumberRows(), false);

    // reflect the property's state to match the grid row

    SetReadOnly(row, 0);
    // TODO: Make this a UpdatePropItem where ADVANCED, and new edit values are reflected
    SetCellValue(row,0, pItem->GetPropName());
    
    // boolean renderer
    if(pItem->GetItemType() == wxPropertyList::CHECKBOX)
    {
        // translate ON or TRUE (case insensitive to a checkbox)
        if(pItem->GetCurValue().IsSameAs(wxT("ON"), false) ||
           pItem->GetCurValue().IsSameAs(wxT("TRUE"), false))
            SetCellValue(row, 1, wxT("1"));
        else
            SetCellValue(row, 1, wxT("0"));
    }
    else
    {
        // for normal path values, give bold in cell when
        // the NOTFOUND is present, for emphasis
        wxString str = pItem->GetPropName() + wxT("-NOTFOUND");     
        if(pItem->GetCurValue().IsSameAs(str))
        {
            wxFont fnt = GetCellFont(row, 0);       
            fnt.SetWeight(wxFONTWEIGHT_BOLD);
            SetCellFont(row, 1, fnt);
        }
        else
            SetCellFont(row, 1, GetCellFont(row, 0));

        SetCellValue(row,1, pItem->GetCurValue());
    }

    if(pItem->GetCurValue().IsSameAs("IGNORE"))
    {
        // ignored cell is completely dimmed
        wxColour col(192,192,192);
        SetCellTextColour(row, 1, col);
    }
    else
    {
        // we colour paths blue, filenames green, all else black
        wxColour col;
        if(pItem->IsDirPath())
            col.Set(0,0,255);
        else if(pItem->IsFilePath())
            col.Set(0,128,0);
        else
            col = GetCellTextColour(row, 0);

        SetCellTextColour(row, 1, col);
    }

    if(pItem->GetNewValue())
    {
        // new cell is red
        wxColour col(255,100,100);
        SetCellBackgroundColour(row, 0, col);
    }
    else
    {
        // old cell is grey
        wxColour col(192, 192, 192);
        SetCellBackgroundColour(row, 0, col);
    }

    return true;
}

void wxPropertyList::OnSelectCell( wxGridEvent& event )
{
    this->SetFocus();
    event.Skip();
}

void wxPropertyList::OnCellChange( wxGridEvent& event )
{
    int row = event.GetRow();

    wxPropertyItem *pItem = GetPropertyItemFromRow(row);
    if(pItem && row != wxNOT_FOUND)
    {
        // write propery back, and set as new
        pItem->SetNewValue(true);
        
        // write back bool
        if(pItem->GetItemType() == CHECKBOX)
        {
            if(GetCellValue(row, 1).IsSameAs("1"))
                pItem->SetCurValue("ON");
            else
                pItem->SetCurValue("OFF");
        }
        else
            pItem->SetCurValue(GetCellValue(row, 1));
    
        UpdatePropertyItem(pItem, row);
        event.Skip();
    }
}

void wxPropertyList::OnCellPopup( wxGridEvent& event )
{
    wxPoint pt;
    int row = event.GetRow();
    
    //pt = ::wxGetMousePosition();
    //ScreenToClient(pt);

    //row = YToRow(pt.y);
    if(row != wxNOT_FOUND)
    {
        wxPropertyItem *pItem = GetPropertyItemFromRow(row);
    
        if(pItem)
        {
            // select the row first if already in selection, don't
            // this will clear the previous selection
            if(!IsInSelection(row, 0))
                SelectRow(row);

            // show popup menu
            wxMenu *menu = AppResources::CreatePopupMenu();
            
            // enable when it is browsable, and selected one only
            wxMenuItem *item = menu->FindItem(ID_CACHE_BROWSE);
            if(item)
                item->Enable(IsSelectedItemBrowsable());
            
            PopupMenu(menu);

            delete menu;
        }
    }
}

void wxPropertyList::OnIgnoreCache( wxCommandEvent& event )
{
    HideControls();
  
    // ignore all selected items
    for(size_t i = 0; i < (size_t)GetNumberRows(); i++)
    {
        if(IsInSelection(i, 0))
        {
            wxPropertyItem *pItem = GetPropertyItemFromRow(i);      
            if(pItem)
            {
                pItem->SetCurValue("IGNORE");
                UpdatePropertyItem(pItem, i);
            }
        }
    }
}

void wxPropertyList::OnDeleteCache( wxCommandEvent& event )
{
    HideControls();
  
    // convert selections to prop items
    wxArrayPtrVoid items;
    for(size_t i = 0; i < (size_t)GetNumberRows(); i++)
    {
        // if selected, query for removal
        if(IsInSelection(i, 0))
        {
            wxPropertyItem *pItem = GetPropertyItemFromRow(i);      
            if(pItem)
                items.Add((void *)pItem);
        }
    }

    // now delete all prop items in cells
    for(size_t i = 0; i < items.Count(); i++)
        RemoveProperty((wxPropertyItem *)items[i]);
}

void wxPropertyList::OnBrowseItem( wxCommandEvent& event )
{
    BrowseSelectedItem();
}

bool wxPropertyList::IsSelectedItemBrowsable(int row)
{
    // when there is only one selection, and our current item
    // is browsable, make sure it can be selected.
    wxPropertyItem *pItem = 0;
    
    size_t count = 0;
    for(size_t i = 0; i < (size_t)GetNumberRows() && (count < 2); i++)
    {
        if(IsInSelection(i, 0))
        {
            if(!pItem)
                pItem = GetPropertyItemFromRow(i);
            count ++;
        }
    }

    // if we found nothing, take row (because the event EVT_GRID_CELL_SELECTED 
    // deselects the cells first before selecting the new one again
    if(row != -1 && !pItem)
    {
        pItem = GetPropertyItemFromRow(row);
        count ++; // needed because of next loop
    }

    // only one item allowed to select
    if(pItem && count == 1)
    {
        if(pItem)
            return pItem->IsDirPath() || pItem->IsFilePath();   
    }
    
    return false;
}


void wxPropertyList::BrowseSelectedItem()
{
    HideControls();
  
    for(size_t i = 0; i < (size_t)GetNumberRows(); i++)
    {
        if(IsInSelection(i, 0))
        {
            // browse for file or directory
            wxPropertyItem *pItem = GetPropertyItemFromRow(i);      
            if(pItem)
            {
                wxString title;
                wxString str = pItem->GetPropName() + _("-NOTFOUND");       
                if(pItem->GetCurValue().IsSameAs(str, true))
                    str.Empty();
                else
                    str = pItem->GetCurValue();

                // browse the directory path
                
                if(pItem->IsDirPath())
                {
                    title = _("Select path for ") + pItem->GetPropName();
                    str = ::wxDirSelector(title, str, 0, wxDefaultPosition, this);
                }
                else if(pItem->IsFilePath())
                {
                    title = _("Select file for ") + pItem->GetPropName();
                    str = ::wxFileSelector(title, str, _(""), _(""), _(MC_DEFAULT_WILDCARD), wxFILE_MUST_EXIST, this);
                }
                else
                    str.Empty();
                
                if(!str.IsEmpty())
                {
                    pItem->SetCurValue(str.c_str());
                    UpdatePropertyItem(pItem, i);
                }
            }
            
            // only allow one item to browse
            break;
        }
    }
}

void wxPropertyList::OnSizeGrid( wxSizeEvent &event )
{
    int width, height;
    
    // make sure the grid's cells are equally adjusted
    GetClientSize(&width, &height);
    SetDefaultColSize(width / 2, true);

    wxGrid::AdjustScrollbars();
}

void wxPropertyList::OnKeyPressed( wxKeyEvent &event )
{
    event.Skip();
}
