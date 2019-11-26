/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCursesPathWidget_h
#define cmCursesPathWidget_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmCursesStandardIncludes.h"
#include "cmCursesStringWidget.h"

class cmCursesMainForm;

class cmCursesPathWidget : public cmCursesStringWidget
{
public:
  cmCursesPathWidget(int width, int height, int left, int top);

  cmCursesPathWidget(cmCursesPathWidget const&) = delete;
  cmCursesPathWidget& operator=(cmCursesPathWidget const&) = delete;

  /**
   * This method is called when different keys are pressed. The
   * subclass can have a special implementation handler for this.
   */
  void OnTab(cmCursesMainForm* fm, WINDOW* w) override;
  void OnReturn(cmCursesMainForm* fm, WINDOW* w) override;
  void OnType(int& key, cmCursesMainForm* fm, WINDOW* w) override;

protected:
  std::string LastString;
  std::string LastGlob;
  bool Cycle;
  std::string::size_type CurrentIndex;
};

#endif // cmCursesPathWidget_h
