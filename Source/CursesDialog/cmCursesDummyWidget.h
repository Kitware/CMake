#ifndef __cmCursesDummyWidget_h
#define __cmCursesDummyWidget_h

#include "cmCursesWidget.h"

class cmCursesDummyWidget : public cmCursesWidget
{
public:
  cmCursesDummyWidget(int width, int height, int left, int top);
  
  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  virtual bool HandleInput(int& key, FORM* form, WINDOW* w);

protected:
  cmCursesDummyWidget(const cmCursesDummyWidget& from);
  void operator=(const cmCursesDummyWidget&);

};

#endif // __cmCursesDummyWidget_h
