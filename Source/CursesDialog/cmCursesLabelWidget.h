/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmCursesStandardIncludes.h"
#include "cmCursesWidget.h"

class cmCursesMainForm;

class cmCursesLabelWidget : public cmCursesWidget
{
public:
  cmCursesLabelWidget(int width, int height, int left, int top,
                      const std::string& name);
  ~cmCursesLabelWidget() override;

  cmCursesLabelWidget(cmCursesLabelWidget const&) = delete;
  cmCursesLabelWidget& operator=(cmCursesLabelWidget const&) = delete;

  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled
  bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w) override;
};
