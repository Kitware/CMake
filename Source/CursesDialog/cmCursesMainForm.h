#ifndef __cmCursesMainForm_h
#define __cmCursesMainForm_h

#include "../cmStandardIncludes.h"
#include "cmCursesForm.h"
#include "cmCursesStandardIncludes.h"

class cmCursesCacheEntryComposite;

class cmCursesMainForm : public cmCursesForm
{
public:
  cmCursesMainForm(std::vector<string> const& args);
  virtual ~cmCursesMainForm();
  
  // Description:
  // Set the widgets which represent the cache entries.
  void InitializeUI();
  
  // Description:
  // Handle user input.
  virtual void HandleInput();

  // Description:
  // Display form. Use a window of size width x height, starting
  // at top, left.
  virtual void Render(int left, int top, int width, int height);

  // Description:
  // Returns true if an entry with the given key is in the
  // list of current composites.
  bool LookForCacheEntry(const char* key);

  static const int MIN_WIDTH;
  static const int MIN_HEIGHT;
  static const int IDEAL_WIDTH;
  static const int MAX_WIDTH;

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  virtual void UpdateStatusBar();

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  void PrintKeys();

  // Description:
  // During a CMake run, an error handle should add errors
  // to be displayed afterwards.
  virtual void AddError(const char* message, const char* title);

  // Description:
  // Used to run cmake.
  void RunCMake(bool generateMakefiles);

protected:
  cmCursesMainForm(const cmCursesMainForm& from);
  void operator=(const cmCursesMainForm&);

  void FillCacheManagerFromUI();
  void RePost();
  void RemoveEntry(const char* value);

  std::vector<cmCursesCacheEntryComposite*>* m_Entries;
  std::vector<std::string> m_Errors;
  std::vector<std::string> m_Args;
  std::vector<std::string> m_HelpMessage;

  static const char* s_ConstHelpMessage;

  FIELD** m_Fields;
  std::string m_WhereSource;
  std::string m_WhereCMake;
  int m_Height;
  int m_NumberOfVisibleEntries;
  bool m_AdvancedMode;
  bool m_OkToGenerate;

};

#endif // __cmCursesMainForm_h
