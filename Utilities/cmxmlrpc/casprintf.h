#ifndef CASPRINTF_H_INCLUDED

/* GNU_PRINTF_ATTR lets the GNU compiler check pm_message() and pm_error()
   calls to be sure the arguments match the format string, thus preventing
   runtime segmentation faults and incorrect messages.
*/
#ifdef __GNUC__
#define GNU_PRINTF_ATTR(a,b) __attribute__ ((format (printf, a, b)))
#else
#define GNU_PRINTF_ATTR(a,b)
#endif

void GNU_PRINTF_ATTR(2,3)
casprintf(const char ** const retvalP, const char * const fmt, ...);

void
strfree(const char * const string);

#endif
