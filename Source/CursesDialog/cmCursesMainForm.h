#ifndef __cmCursesMainForm_h
#define __cmCursesMainForm_h

#include "../cmStandardIncludes.h"
#include "cmCursesForm.h"
#include "cmCursesStandardIncludes.h"

class cmCursesCacheEntryComposite;

class cmCursesMainForm : public cmCursesForm
{
public:
  cmCursesMainForm(const char* whereSource, bool newCache);
  virtual ~cmCursesMainForm();
  
  // Description:
  // Set the widgets which represent the cache entries.
  void InitializeUI(WINDOW* w);
  
  // Description:
  // Handle user input.
  virtual void HandleInput();

  // Description:
  // Display form. Use a window of size width x height, starting
  // at top, left.
  virtual void Render(int left, int top, int width, int height);

  // Description:
  // Change the window containing the form.
  void SetWindow(WINDOW* w)
    { m_Window = w; }

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
  void UpdateStatusBar();

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  void PrintKeys();

protected:
  cmCursesMainForm(const cmCursesMainForm& from);
  void operator=(const cmCursesMainForm&);

  void RunCMake(bool generateMakefiles);
  void FillCacheManagerFromUI();

  std::vector<cmCursesCacheEntryComposite*>* m_Entries;
  FIELD** m_Fields;
  WINDOW* m_Window;
  std::string m_WhereSource;
  int m_Height;

};

#endif // __cmCursesMainForm_h
