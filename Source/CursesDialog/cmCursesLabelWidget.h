#ifndef __cmCursesLabelWidget_h
#define __cmCursesLabelWidget_h

#include "cmCursesWidget.h"
#include "cmCursesStandardIncludes.h"

class cmCursesMainForm;

class cmCursesLabelWidget : public cmCursesWidget
{
public:
  cmCursesLabelWidget(int width, int height, int left, int top,
		      const std::string& name);
  virtual ~cmCursesLabelWidget();
  
  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);

protected:
  cmCursesLabelWidget(const cmCursesLabelWidget& from);
  void operator=(const cmCursesLabelWidget&);
};

#endif // __cmCursesLabelWidget_h
