/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCursesStandardIncludes_h
#define cmCursesStandardIncludes_h
#if defined(__sun__) && defined(__GNUC__)
 #define _MSE_INT_H
#endif

#include <cmFormConfigure.h>

#if defined(__hpux)
# define _BOOL_DEFINED
# include <sys/time.h>
# define _XOPEN_SOURCE_EXTENDED
# include <curses.h>
# include <form.h>
# undef _XOPEN_SOURCE_EXTENDED
#else
/* figure out which curses.h to include */
# if defined(CURSES_HAVE_NCURSES_H)
#  include <ncurses.h>
# elif defined(CURSES_HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(CURSES_HAVE_NCURSES_CURSES_H)
#  include <ncurses/curses.h>
# else
#  include <curses.h>
# endif

# include <form.h>
#endif

// This is a hack to prevent warnings about these functions being
// declared but not referenced.
#if defined(__sgi) && !defined(__GNUC__)
class cmCursesStandardIncludesHack
{
public:
  enum
  {
    Ref1 = sizeof(cfgetospeed(0)),
    Ref2 = sizeof(cfgetispeed(0)),
    Ref3 = sizeof(tcgetattr(0, 0)),
    Ref4 = sizeof(tcsetattr(0, 0, 0)),
    Ref5 = sizeof(cfsetospeed(0,0)),
    Ref6 = sizeof(cfsetispeed(0,0))
  };
};
#endif

#ifndef getmaxyx
 #define getmaxyx(w,y,x) ((y) = getmaxy(w), (x) = getmaxx(w))
#endif


// on some machines move erase and clear conflict with stl
// so remove them from the namespace
inline void curses_move(unsigned int x, unsigned int y)
{
  move(x,y);
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
