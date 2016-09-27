/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCursesFilePathWidget_h
#define cmCursesFilePathWidget_h

#include <cmConfigure.h> // IWYU pragma: keep

#include "cmCursesPathWidget.h"

class cmCursesFilePathWidget : public cmCursesPathWidget
{
public:
  cmCursesFilePathWidget(int width, int height, int left, int top);

protected:
  cmCursesFilePathWidget(const cmCursesFilePathWidget& from);
  void operator=(const cmCursesFilePathWidget&);
};

#endif // cmCursesFilePathWidget_h
