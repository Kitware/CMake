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


