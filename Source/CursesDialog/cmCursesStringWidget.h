#ifndef __cmCursesStringWidget_h
#define __cmCursesStringWidget_h

#include "cmCursesWidget.h"

class cmCursesStringWidget : public cmCursesWidget
{
public:
  cmCursesStringWidget(int width, int height, int left, int top);
  
  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  virtual bool HandleInput(int& key, FORM* form, WINDOW* w);

  // Description:
  // Set/Get the string.
  void SetString(const char* value);
  const char* GetString();
  virtual const char* GetValue();

  // Description:
  // Set/Get InEdit flag. Can be used to tell the widget to leave
  // edit mode (in case of a resize for example).
  void SetInEdit(bool inedit)
    { m_InEdit = inedit; }
  bool GetInEdit()
    { return m_InEdit; }

protected:
  cmCursesStringWidget(const cmCursesStringWidget& from);
  void operator=(const cmCursesStringWidget&);

  // true if the widget is in edit mode
  bool m_InEdit;
};

#endif // __cmCursesStringWidget_h
