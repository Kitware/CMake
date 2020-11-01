/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCursesStandardIncludes.h"
#include "cmCursesWidget.h"

class cmCursesMainForm;

class cmCursesOptionsWidget : public cmCursesWidget
{
public:
  cmCursesOptionsWidget(int width, int height, int left, int top);

  cmCursesOptionsWidget(cmCursesOptionsWidget const&) = delete;
  cmCursesOptionsWidget& operator=(cmCursesOptionsWidget const&) = delete;

  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w) override;
  void SetOption(const std::string&);
  void AddOption(std::string const&);
  void NextOption();
  void PreviousOption();

protected:
  std::vector<std::string> Options;
  std::vector<std::string>::size_type CurrentOption;
};
