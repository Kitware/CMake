/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCursesPathWidget.h"

class cmCursesFilePathWidget : public cmCursesPathWidget
{
public:
  cmCursesFilePathWidget(int width, int height, int left, int top);

  cmCursesFilePathWidget(cmCursesFilePathWidget const&) = delete;
  cmCursesFilePathWidget& operator=(cmCursesFilePathWidget const&) = delete;
};
