/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCursesStandardIncludes.h"
#include "cmCursesWidget.h"

class cmCursesMainForm;

class cmCursesBoolWidget : public cmCursesWidget
{
public:
  cmCursesBoolWidget(int width, int height, int left, int top);

  cmCursesBoolWidget(cmCursesBoolWidget const&) = delete;
  cmCursesBoolWidget& operator=(cmCursesBoolWidget const&) = delete;

  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w) override;

  // Description:
  // Set/Get the value (on/off).
  void SetValueAsBool(bool value);
  bool GetValueAsBool();
};
