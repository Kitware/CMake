/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmCursesStandardIncludes_h
#define cmCursesStandardIncludes_h
#if defined(__sun__) && defined(__GNUC__)
 #define _MSE_INT_H
#endif

#if defined(__hpux)
# define _BOOL_DEFINED
# include <sys/time.h>
# define _XOPEN_SOURCE_EXTENDED
# include <curses.h>
# include <form.h>
# undef _XOPEN_SOURCE_EXTENDED
#else
# include <curses.h>
# include <form.h>
#endif

// This is a hack to prevent warnings about these functions being
// declared but not referenced.
#if defined(__sgi) && !defined(__GNUC__)
/*REFERENCED*/ speed_t cfgetospeed (const struct termios *__t);
/*REFERENCED*/ int cfsetospeed (struct termios *__t, speed_t __s); 
/*REFERENCED*/ speed_t cfgetispeed (const struct termios *__t);
/*REFERENCED*/ int cfsetispeed (struct termios *__t, speed_t __s);
/*REFERENCED*/ int tcgetattr (int __fd, struct termios *__t);
/*REFERENCED*/ int tcsetattr (int __fd, int __act, const struct termios *__t);
enum
{
  cmCursesStandardIncludesHackRef1 = sizeof(cfgetospeed(0)),
  cmCursesStandardIncludesHackRef2 = sizeof(cfgetispeed(0)),
  cmCursesStandardIncludesHackRef3 = sizeof(tcgetattr(0, 0)),
  cmCursesStandardIncludesHackRef4 = sizeof(tcsetattr(0, 0, 0))
  cmCursesStandardIncludesHackRef5 = sizeof(cfsetospeed(0,0)),
  cmCursesStandardIncludesHackRef6 = sizeof(cfsetispeed(0,0)),
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
