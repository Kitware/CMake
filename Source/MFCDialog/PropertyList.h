#ifndef CPROPERTYLIST_H
#define CPROPERTYLIST_H



#include "../cmStandardIncludes.h"

/////////////////////////////////////////////////////////////////////////////
//CPropertyList Items
class CPropertyItem
{
// Attributes
public:
  CString m_HelpString;
  CString m_propName;
  CString m_curValue;
  int m_nItemType;
  CString m_cmbItems;
  bool m_Removed;
public:
  CPropertyItem(CString propName, CString curValue,
                CString helpString,
                int nItemType, CString cmbItems)
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
// CPropertyList window

class CPropertyList : public CListBox
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
  CPropertyList();
  
// Attributes
public:

// Operations
public:
  bool IsDirty() { return m_Dirty;  }
  void ClearDirty() { m_Dirty = false;  }
  
  int AddItem(CString txt);
  int AddProperty(const char* name,
                  const char* value,
                  const char* helpString,
                  int type,
                  const char* comboItems);
  std::set<CPropertyItem*> GetItems() 
    {
      return m_PropertyItems;
    }
  void RemoveAll();
  CPropertyItem* GetItem(int index);
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CPropertyList)
public:
  virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void PreSubclassWindow();
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CPropertyList();

  // Generated message map functions
protected:
  //{{AFX_MSG(CPropertyList)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSelchange();
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp( UINT nFlags, CPoint point );
  //}}AFX_MSG
  afx_msg void OnKillfocusCmbBox();
  afx_msg void OnSelchangeCmbBox();
  afx_msg void OnKillfocusEditBox();
  afx_msg void OnChangeEditBox();
  afx_msg void OnButton();
  afx_msg void OnDelete();
  afx_msg void OnHelp();
  afx_msg void OnCheckBox();

  DECLARE_MESSAGE_MAP()

  void InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo);
  void DisplayButton(CRect region);
  int AddPropItem(CPropertyItem* pItem);
  void InvalidateList();
  
  CComboBox m_cmbBox;
  CEdit m_editBox;
  CButton m_btnCtrl;
  CButton m_CheckBoxControl;
  
  CFont m_SSerif8Font;
	
  bool m_Dirty;
  int m_curSel;
  int m_prevSel;
  int m_nDivider;
  int m_nDivTop;
  int m_nDivBtm;
  int m_nOldDivX;
  int m_nLastBox;
  BOOL m_bTracking;
  BOOL m_bDivIsSet;
  HCURSOR m_hCursorArrow;
  HCURSOR m_hCursorSize;
  std::set<CPropertyItem*> m_PropertyItems;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYLIST_H__74205380_1B56_11D4_BC48_00105AA2186F__INCLUDED_)
