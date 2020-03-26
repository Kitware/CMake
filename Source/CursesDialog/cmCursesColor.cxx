/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCursesColor.h"

#include "cmCursesStandardIncludes.h"

bool cmCursesColor::HasColors()
{
#ifdef HAVE_CURSES_USE_DEFAULT_COLORS
  return has_colors();
#else
  return false;
#endif
}

void cmCursesColor::InitColors()
{
#ifdef HAVE_CURSES_USE_DEFAULT_COLORS
  if (HasColors()) {
    start_color();
    use_default_colors();
    init_pair(cmCursesColor::BoolOff, COLOR_RED, -1);
    init_pair(cmCursesColor::BoolOn, COLOR_GREEN, -1);
    init_pair(cmCursesColor::String, COLOR_BLUE, -1);
    init_pair(cmCursesColor::Path, COLOR_YELLOW, -1);
    init_pair(cmCursesColor::Options, COLOR_MAGENTA, -1);
  }
#endif
}
