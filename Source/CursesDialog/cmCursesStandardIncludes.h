#if defined(__sun__) && defined(__GNUC__)
 #define _MSE_INT_H
#endif

#define _BOOL_DEFINED
#include <sys/time.h>
#define _XOPEN_SOURCE_EXTENDED
#include <curses.h>
#include <form.h>
#undef _XOPEN_SOURCE_EXTENDED

