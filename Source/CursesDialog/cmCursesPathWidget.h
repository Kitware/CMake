#ifndef __cmCursesPathWidget_h
#define __cmCursesPathWidget_h

#include "cmCursesStringWidget.h"

class cmCursesPathWidget : public cmCursesStringWidget
{
public:
  cmCursesPathWidget(int width, int height, int left, int top);

protected:
  cmCursesPathWidget(const cmCursesPathWidget& from);
  void operator=(const cmCursesPathWidget&);

};

#endif // __cmCursesPathWidget_h
