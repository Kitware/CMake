#ifndef __cmCursesWidget_h
#define __cmCursesWidget_h

#include <curses.h>
#include <form.h>
#include "../cmCacheManager.h"

class cmCursesWidget
{
public:
  cmCursesWidget(int width, int height, int left, int top);
  virtual ~cmCursesWidget();
  
  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled
  virtual bool HandleInput(int& key, FORM* form, WINDOW* w) = 0;

  // Description:
  // Change the position of the widget. Set isNewPage to true
  // if this widget marks the beginning of a new page.
  virtual void Move(int x, int y, bool isNewPage);

  // Description:
  // Set/Get the value (setting the value also changes the contents
  // of the field buffer).
  virtual void SetValue(const char* value);
  virtual const char* GetValue();

  // Description:
  // Get the type of the widget (STRING, PATH etc...)
  cmCacheManager::CacheEntryType GetType()
    { return m_Type; }

  friend class cmCursesMainForm;

protected:
  cmCursesWidget(const cmCursesWidget& from);
  void operator=(const cmCursesWidget&);

  cmCacheManager::CacheEntryType m_Type;
  std::string m_Value;
  FIELD* m_Field;
};

#endif // __cmCursesWidget_h
