#ifndef cmCursesStandardIncludes_h
#define cmCursesStandardIncludes_h
#if defined(__sun__) && defined(__GNUC__)
 #define _MSE_INT_H
#endif

#if defined(__hpux)
 #define _BOOL_DEFINED
 #include <sys/time.h>
 #define _XOPEN_SOURCE_EXTENDED
 #include <curses.h>
 #include <form.h>
 #undef _XOPEN_SOURCE_EXTENDED
#else
 #include <curses.h>
 #include <form.h>
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

#undef move
#undef erase
#undef clear


#endif // cmCursesStandardIncludes_h
