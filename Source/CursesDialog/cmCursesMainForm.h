/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __cmCursesMainForm_h
#define __cmCursesMainForm_h

#include "../cmStandardIncludes.h"
#include "cmCursesForm.h"
#include "cmCursesStandardIncludes.h"

class cmCursesCacheEntryComposite;
class cmake;

/** \class cmCursesMainForm
 * \brief The main page of ccmake
 *
 * cmCursesMainForm is the main page of ccmake.
 */
class cmCursesMainForm : public cmCursesForm
{
public:
  cmCursesMainForm(std::vector<std::string> const& args, int initwidth);
  virtual ~cmCursesMainForm();
  
  /**
   * Set the widgets which represent the cache entries.
   */
  void InitializeUI();
  
  /**
   * Handle user input.
   */
  virtual void HandleInput();

  /**
   * Display form. Use a window of size width x height, starting
   * at top, left.
   */
  virtual void Render(int left, int top, int width, int height);

  /**
   * Returns true if an entry with the given key is in the
   * list of current composites.
   */
  bool LookForCacheEntry(const char* key);

  enum {
    MIN_WIDTH = 65,
    MIN_HEIGHT = 6,
    IDEAL_WIDTH = 80,
    MAX_WIDTH = 512
  };

  /**
   * This method should normally  called only by the form.
   * The only exception is during a resize.
   */
  virtual void UpdateStatusBar();

  /**
   * Display current commands and their keys on the toolbar.
   * This method should normally  called only by the form.
   * The only exception is during a resize.
   */
  void PrintKeys();

  /**
   * During a CMake run, an error handle should add errors
   * to be displayed afterwards.
   */
  virtual void AddError(const char* message, const char* title);

  /**
   * Used to do a configure.
   */
  int Configure();

  /**
   * Used to generate 
   */
  int Generate();

  /**
   * Used by main program
   */
  void LoadCache(const char *dir);
  
protected:
  cmCursesMainForm(const cmCursesMainForm& from);
  void operator=(const cmCursesMainForm&);

  // Copy the cache values from the user interface to the actual
  // cache.
  void FillCacheManagerFromUI();
  // Re-post the existing fields. Used to toggle between
  // normal and advanced modes. Render() should be called
  // afterwards.
  void RePost();
  // Remove an entry from the interface and the cache.
  void RemoveEntry(const char* value);

  // Copies of cache entries stored in the user interface
  std::vector<cmCursesCacheEntryComposite*>* m_Entries;
  // Errors produced during last run of cmake
  std::vector<std::string> m_Errors;
  // Command line argumens to be passed to cmake each time
  // it is run
  std::vector<std::string> m_Args;
  // Message displayed when user presses 'h'
  // It is: Welcome + info about current entry + common help
  std::vector<std::string> m_HelpMessage;

  // Common help
  static const char* s_ConstHelpMessage;

  // Fields displayed. Includes labels, new entry markers, entries
  FIELD** m_Fields;
  // Where is source of current project
  std::string m_WhereSource;
  // Where is cmake executable
  std::string m_WhereCMake;
  // Number of entries shown (depends on mode -normal or advanced-)
  int m_NumberOfVisibleEntries;
  bool m_AdvancedMode;
  // Did the iteration converge (no new entries) ?
  bool m_OkToGenerate;
  // Number of pages displayed
  int m_NumberOfPages;

  int m_InitialWidth;
  cmake *m_CMakeInstance;
};

#endif // __cmCursesMainForm_h
