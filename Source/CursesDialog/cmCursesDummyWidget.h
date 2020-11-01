/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCursesStandardIncludes.h"
#include "cmCursesWidget.h"

class cmCursesMainForm;

class cmCursesDummyWidget : public cmCursesWidget
{
public:
  cmCursesDummyWidget(int width, int height, int left, int top);

  cmCursesDummyWidget(cmCursesDummyWidget const&) = delete;
  cmCursesDummyWidget& operator=(cmCursesDummyWidget const&) = delete;

  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w) override;
};
