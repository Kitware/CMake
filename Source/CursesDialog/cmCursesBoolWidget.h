#ifndef __cmCursesBoolWidget_h
#define __cmCursesBoolWidget_h

#include "cmCursesWidget.h"
class cmCursesMainForm;

class cmCursesBoolWidget : public cmCursesWidget
{
public:
  cmCursesBoolWidget(int width, int height, int left, int top);
  
  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);

  // Description:
  // Set/Get the value (on/off).
  void SetValueAsBool(bool value);
  bool GetValueAsBool();

protected:
  cmCursesBoolWidget(const cmCursesBoolWidget& from);
  void operator=(const cmCursesBoolWidget&);

};

#endif // __cmCursesBoolWidget_h
