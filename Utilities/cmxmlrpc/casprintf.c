#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "xmlrpc_config.h"  /* For HAVE_ASPRINTF */
#include "casprintf.h"

void GNU_PRINTF_ATTR(2,3)
casprintf(const char ** const retvalP, const char * const fmt, ...) {

    char *retval;

    va_list varargs;  /* mysterious structure used by variable arg facility */

    va_start(varargs, fmt); /* start up the mysterious variable arg facility */

#if HAVE_ASPRINTF
    vasprintf(&retval, fmt, varargs);
#else
    retval = malloc(8192);
    vsnprintf(retval, 8192, fmt, varargs);
#endif
    *retvalP = retval;
}



void
strfree(const char * const string) {
    free((void *)string);
}



