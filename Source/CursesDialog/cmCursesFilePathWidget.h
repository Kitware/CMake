#ifndef __cmCursesFilePathWidget_h
#define __cmCursesFilePathWidget_h

#include "cmCursesStringWidget.h"

class cmCursesFilePathWidget : public cmCursesStringWidget
{
public:
  cmCursesFilePathWidget(int width, int height, int left, int top);

protected:
  cmCursesFilePathWidget(const cmCursesFilePathWidget& from);
  void operator=(const cmCursesFilePathWidget&);

};

#endif // __cmCursesFilePathWidget_h
