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
#ifndef __cmMainFrame__h__
#define __cmMainFrame__h__

#include "cmStandardIncludes.h"

#include "wxincludes.h"

class cmCacheProperty;
class cmCommandLineInfo;
class cmake;
class wxButton;
class wxCheckBox;
class wxComboBox;
class wxMenu;
class wxPanel;
class wxScrolledWindow;
class wxSizer;
class wxStaticBox;
class wxStaticText;

/** \class cmMainFrame
 * \brief GUI for CMake with wxWindows toolkit
 *
 * The main dialog for the CMake
 * 
 */
class cmMainFrame : public wxFrame
{
public:
  cmMainFrame(const wxString& title, const wxSize& size);
  ~cmMainFrame();

  //! Initialize the paths and read the cache.
  void Initialize(cmCommandLineInfo*);

  //! Different callbacks for events
  void OnOk(wxCommandEvent& event);
  void OnConfigure(wxCommandEvent& event);
  void OnCancel(wxCommandEvent& event);
  void OnHelp(wxCommandEvent& event);
  void OnBrowseSource(wxCommandEvent& event);
  void OnSourceSelected(wxCommandEvent& event);
  void OnSourceUpdated(wxCommandEvent& event);
  void OnBrowseBinary(wxCommandEvent& event);
  void OnBinarySelected(wxCommandEvent& event);
  void OnBinaryUpdated(wxCommandEvent& event);
  void OnShowAdvancedValues(wxCommandEvent& event);
  void OnResize(wxSizeEvent& event);
  void OnPropertyChanged(wxEvent& event);
  void OnRandomEvent(wxEvent& event);
  void OnGeneratorSelected(wxEvent& event);
  void OnPopupMenu(wxMouseEvent& event);
  void OnCacheStatusBar(wxEvent& event);
  void OnStatusBar(wxEvent& event);
  void OnExitTimer(wxEvent& event);

  //! Callbacks for menu events
  void OnPopupMenuIgnore(wxEvent& event);
  void OnPopupMenuDelete(wxEvent& event);
  void OnPopupMenuHelp(wxEvent& event);
  void OnPopupMenuEntry(wxEvent& event, int idx);

  // Connect widget and event with method.
  void ConnectEvent(wxWindow*, wxEventType, wxObjectEventFunction);
  void ConnectEventTo(wxWindow*, wxEventType, wxObjectEventFunction);

  //! Callback for the error message.
  static void MessageCallback(const char* m, const char* title, bool& nomore, void* cd);
  void DisplayMessage(const char* m, const char* title, bool& nomore);

  //! Retrieve the current build directory.
  const std::string& GetBuildDir() { return this->m_WhereBuild; }

private:
  //! Load cache file from m_WhereBuild and display in GUI editor
  void LoadCacheFromDiskToGUI();

  //! Save GUI values to cmCacheManager and then save to disk.
  void SaveCacheFromGUI();

  // copy from the cache manager to the cache edit list box
  void FillCacheGUIFromCacheManager();

  // copy from the list box to the cache manager
  void FillCacheManagerFromCacheGUI();

  // set the current generator
  void SetGenerator(const char* generator);

  // Set the status bar binding.
  void SetupStatusBarBinding(wxWindow*);
  
  // set the current source and binary dir
  bool SetSourceDir(const char* dir);
  std::string GetSourceDir();
  bool SetBinaryDir(const char* dir);
  std::string GetBinaryDir();

  void ChangeDirectoriesFromFile(const char* buffer);

  // update source and binary menus.
  void UpdateSourceBuildMenus();

  // Check wether cache is dirty.  
  bool IsDirty() { return !this->m_Clean; }
  void SetDirty() { this->m_Clean = false; }
  void ClearDirty() { this->m_Clean = true; }
  
  // Run the CMake
  void RunCMake(bool generateProjectFiles);

  void RemoveAdvancedValues();
  void UpdateCacheValuesDisplay();

  // Change the build directory.
  void ChangeWhereSource();
  void ChangeWhereBuild();
  bool SourceDirectoryChanged();
  bool BuildDirectoryChanged();

  // Clear the Cache
  void ClearCache();

  void RemoveCacheEntry(cmCacheProperty*);
  void IgnoreCacheEntry(const char* key);
  void HelpCacheEntry(const char* key, const char* help);

  void LoadFromRegistry();
  void SaveToRegistry();
  virtual void SetStatusText(const wxString& text, int number = 0);
  void ResizeInternal();

  //! Change the type of mouse cursor. Set argument to true to store
  // the type.
  void CursorBusy(bool s=false);
  void CursorNormal(bool s=false);

  // Main panel
  wxPanel*          m_MainPanel;
  wxSizer*          m_TopMostSizer;

  // Main sizer
  wxSizer*          m_MainSizer;

  // Top row of main sizer
  wxSizer*          m_TopGrid;
  
  // Top line:
  wxStaticText*     m_TextSource;
  wxComboBox*       m_PathSource;
  wxButton*         m_BrowseSource;

  // Top line end frame:
  wxSizer*          m_GeneratorFrame;
  wxStaticText*     m_BuildFor;
  wxComboBox*       m_GeneratorMenu;

  // Bottom line:
  wxStaticText*     m_TextBinary;
  wxComboBox*       m_PathBinary;
  wxButton*         m_BrowseBinary;
  wxCheckBox*       m_ShowAdvancedValues;

  // Cache values:
  wxStaticBox*      m_CacheValuesBox;
  wxSizer*          m_CacheValuesFrame;
  wxScrolledWindow* m_CacheValuesScroll;
  wxPanel*          m_CacheValuesPanel;
  wxSizer*          m_CacheValuesSizer;

  // Help text:
  wxStaticText*     m_HelpText;
  
  // Buttons:
  wxSizer*          m_BottomButtonsFrame;
  wxStaticText*     m_VersionText;
  wxButton*         m_ConfigureButton;
  wxButton*         m_OKButton;
  wxButton*         m_CancelButton;
  wxButton*         m_HelpButton;

  // This is set when the cache has to be updated.
  bool              m_Update;

  // This is to detect when cache is not valid such as when cache
  // entry is removed or when some new entries are present. You have
  // to rerun cmake to set valid to true.
  bool              m_Valid;

  // This is needed for mac, because on mac dialog has to be redrawn
  // after the menu is removed.
  bool              m_EntryRemoved;

  std::string                             m_WhereSource;
  std::string                             m_WhereBuild;
  std::string                             m_PathToExecutable;
  bool                                    m_Clean;
  bool                                    m_BuildPathChanged;
  bool                                    m_CursorChanged; 

  typedef std::map<std::string, cmCacheProperty*> CacheMapType;

  CacheMapType*                           m_CacheEntries;
  cmake*                                  m_CMakeInstance;
  wxTimer*                                m_ExitTimer;

  enum Events {
    ID_MainFrame,
    ID_Resize,
    ID_OKButton,
    ID_ConfigureButton,
    ID_CancelButton,
    ID_HelpButton,
    ID_AdvancedValues
  };
};

#endif // __cmMainFrame__h__
