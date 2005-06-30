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

#ifndef __WXPROPERTYLIST_H
#define __WXPROPERTYLIST_H


#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dynarray.h>
#include <wx/grid.h>

#include "../cmStandardIncludes.h"

#ifdef __LINUX__
  #define MC_DEFAULT_WILDCARD "*"
#else
  #define MC_DEFAULT_WILDCARD "*.*"
#endif

#if 0

// the editor for string/text data
class wxGridCellPathEditor : public wxGridCellEditor
{
public:
    wxGridCellPathEditor();

    virtual void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler);
    virtual void SetSize(const wxRect& rect);

    virtual void PaintBackground(const wxRect& rectCell, wxGridCellAttr *attr);

    virtual bool IsAcceptedKey(wxKeyEvent& event);
    virtual void BeginEdit(int row, int col, wxGrid* grid);
    virtual bool EndEdit(int row, int col, wxGrid* grid);

    virtual void Reset();
    virtual void StartingKey(wxKeyEvent& event);
    virtual void HandleReturn(wxKeyEvent& event);

    // parameters string format is "max_width"
    virtual void SetParameters(const wxString& params);

    virtual wxGridCellEditor *Clone() const
        { return new wxGridCellPathEditor; }

    // DJC MAPTEK
    // added GetValue so we can get the value which is in the control
    virtual wxString GetValue() const;
protected:
    wxTextCtrl *Text() const { return (wxTextCtrl *)m_control; }

    // parts of our virtual functions reused by the derived classes
    void DoBeginEdit(const wxString& startValue);
    void DoReset(const wxString& startValue);

private:
    size_t   m_maxChars;        // max number of chars allowed
    wxString m_startValue;

    DECLARE_NO_COPY_CLASS(wxGridCellPathEditor)
};

#endif

/////////////////////////////////////////////////////////////////////////////
//wxPropertyItem Items
class wxPropertyItem
{  
public:
    wxPropertyItem(const wxString &propName, const wxString &curValue, 
                   const wxString &helpString, int nItemType, const wxString &cmbItems)
        : m_NewValue(true)
        , m_HelpString(helpString)
        , m_Removed(false)
        , m_propName(propName)
        , m_curValue(curValue)
        , m_nItemType(nItemType)
        , m_cmbItems(cmbItems)
        , m_Advanced(false)
    {
      //m_NewValue = true;
      //m_HelpString = helpString;
      //m_Removed = false;
      //m_propName = propName;
      //m_curValue = curValue;
      //m_nItemType = nItemType;
      //m_cmbItems = cmbItems;
      //m_Advanced = false;
    }

    const wxString &GetCurValue() const {
        return m_curValue;
    };
    
    const wxString &GetPropName() const {
        return m_propName;
    };
    
    void SetCurValue(const char *str) {
        m_curValue = str;
    };
    
    void SetPropName(const char *str) {
        m_propName = str;
    };

    void SetAdvanced(bool value) {
        m_Advanced = value;
    };

    void SetHelpString(const char *str) {
        m_HelpString = str;
    };

    bool GetAdvanced() const {
        return m_Advanced;
    };

    void SetNewValue(bool value) {
            m_NewValue = value;
    };

    bool GetNewValue() const {
        return m_NewValue;
    };

    int GetItemType() const {
        return m_nItemType;
    };

    const wxString &GetHelpString() const {
        return m_HelpString;
    };

    // returns true when this property item is a filepath
    bool IsFilePath();

    // returns true when this property item is a dir path
    bool IsDirPath();

private:
    wxString m_HelpString;
    wxString m_propName;
    wxString m_curValue;
    int m_nItemType;
    wxString m_cmbItems;
    bool m_NewValue;
    bool m_Removed;
    bool m_Advanced;
};

WX_DEFINE_ARRAY(wxPropertyItem *, wxPropertyItems); 

/////////////////////////////////////////////////////////////////////////////
// wxPropertyList Grid
class wxPropertyList : public wxGrid
{
// Construction
public:
    wxPropertyList(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, 
                   const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, 
                   const wxString& name = wxPanelNameStr)
        : wxGrid(parent, id, pos, size, style, name)
    {
    };

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
  
    virtual ~wxPropertyList();
  
public:
    bool GetShowAdvanced() const {
        return m_ShowAdvanced;
    };
        
    void SetProjectGenerated(bool value) { 
        m_generatedProjects = value;  
    };
    
    bool IsGenerated() {
        return m_generatedProjects;
    };

    int AddItem(const wxString &txt);
    
    void AddProperty(const char* name, const char* value, const char* helpString, int type,
                     const char* comboItems, bool reverseOrder, bool advanced);
    
    void RemoveProperty(wxPropertyItem *pItem);
    
    void HideControls();
    void ShowAdvanced();
    void HideAdvanced();
    
    void RemoveAll();

    wxPropertyItem* GetItem(size_t index) {
        wxCHECK(index < GetCount(), 0);
        return m_PropertyItems[index];
    };
    
    size_t GetCount() const {
        return m_PropertyItems.Count();
    };

    const wxPropertyItems &GetItems() {
        return m_PropertyItems;
    };
    
    void SetShowAdvanced(bool value) {
        m_ShowAdvanced = value;
    };

    bool UpdatePropertyItem(wxPropertyItem *pItem, int row);

    /** Returns the row of the grid on which the item is found. -1 is returned when not found */
    int FindProperty(wxPropertyItem *pItem);

    /** Returns true when the cache is still dirty, which means
        after a configuration run, there are still new items */
    bool IsCacheDirty() {
        // check if we have items that are new
        for(size_t i = 0; i < m_PropertyItems.Count(); i++)
        {
            //if((m_PropertyItems[i]->GetNewValue() && m_ShowAdvanced && m_PropertyItems[i]->GetAdvanced()) ||
            //   (m_PropertyItems[i]->GetNewValue() && !m_ShowAdvanced))
            if(m_PropertyItems[i]->GetNewValue())
                return true;
        }   
        
        // return flag if generated cache or not
        return false;
    };

    /** Returns the property item from the current row. 0 if no property item found */
    wxPropertyItem *GetPropertyItemFromRow(int row);

    /** Finds the property by name and returns the property item */
    wxPropertyItem *FindPropertyByName(const wxString &name);

    /** Sets the search filter. Making sure that the items currently displayed
        match this substring query */
    void SetQuery(const wxString &query) {
        m_strQuery = query;
        UpdateGridView();
    };

    /** Returns the last used query. */
    const wxString &GetQuery() {
        return m_strQuery;
    };

    /** Function that tries to browse for the item that is selected */
    void BrowseSelectedItem();

    /** Returns true when the item is browsable (file or path) and only one item is 
        selected */
    bool IsSelectedItemBrowsable(int row = -1);

private:    
    // order = 0 sorted
    // order = 1 add to top
    // order = 2 add to bottom
    int AddPropItem(wxPropertyItem* pItem, int order);
    
    /** Adds the property item to the grid. */
    int AddPropertyToGrid(wxPropertyItem *pItem, int order)    ;
    
    /** Adds / removes all the items that are currently (not) matching the 
        query filter */
    void UpdateGridView();

    void OnSelectCell( wxGridEvent& event );

    void OnCellChange( wxGridEvent& event );

    void OnCellPopup( wxGridEvent& event );

    void OnIgnoreCache( wxCommandEvent& event );

    void OnDeleteCache( wxCommandEvent& event );

    void OnSizeGrid( wxSizeEvent &event );

    void OnBrowseItem( wxCommandEvent& event );

    void OnKeyPressed( wxKeyEvent &event );

    int m_curSel;
    int m_prevSel;
    int m_nDivider;
    int m_nDivTop;
    int m_nDivBtm;
    int m_nOldDivX;
    int m_nLastBox;
    bool m_bTracking;
    bool m_bDivIsSet;
    bool m_ShowAdvanced;
    // flag to indicate the last cache load has resulted
    // in generating a valid project
    bool m_generatedProjects;
    wxString m_strQuery;
    wxPropertyItems m_PropertyItems;

    DECLARE_EVENT_TABLE()
};

#endif
