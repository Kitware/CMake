/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCursesStandardIncludes_h
#define cmCursesStandardIncludes_h

#include "cmConfigure.h"

#if defined(__hpux)
#define _BOOL_DEFINED
#include <sys/time.h>
#endif

#include <form.h>

// on some machines move erase and clear conflict with stl
// so remove them from the namespace
inline void curses_move(unsigned int x, unsigned int y)
{
  move(x, y);
}

inline void curses_clear()
{
  erase();
  clearok(stdscr, TRUE);
}

#undef move
#undef erase
#undef clear

#endif // cmCursesStandardIncludes_h
