#ifdef __hpux
 #define _BOOL_DEFINED
 #ifndef _XOPEN_SOURCE_EXTENDED
  #include <sys/time.h>
  #define _XOPEN_SOURCE_EXTENDED
  #include <curses.h>
  #include <form.h>
  #undef _XOPEN_SOURCE_EXTENDED
 #else
  #include <curses.h>
  #include <form.h>
 #endif  
#else  /* __hpux */
 #include <curses.h>
 #include <form.h>
#endif /* __hpux */

#ifndef getmaxyx
 #define getmaxyx(w,y,x) ((y) = getmaxy(w), (x) = getmaxx(w))
#endif
