/* PDCurses */

#include <curspriv.h>

/*man-start**************************************************************

debug
-----

### Synopsis

    void traceon(void);
    void traceoff(void);
    void PDC_debug(const char *, ...);

### Description

   traceon() and traceoff() toggle the recording of debugging
   information to the file "trace". Although not standard, similar
   functions are in some other curses implementations.

   PDC_debug() is the function that writes to the file, based on whether
   traceon() has been called. It's used from the PDC_LOG() macro.

   The environment variable PDC_TRACE_FLUSH controls whether the trace
   file contents are fflushed after each write. The default is not. Set
   it to enable this (may affect performance).

### Portability
                             X/Open  ncurses  NetBSD
    traceon                     -       -       -
    traceoff                    -       -       -
    PDC_debug                   -       -       -

**man-end****************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

static bool want_fflush = FALSE;

void PDC_debug(const char *fmt, ...)
{
    va_list args;
    char hms[9];
    time_t now;

    if (!SP || !SP->dbfp)
        return;

    time(&now);
    strftime(hms, 9, "%H:%M:%S", localtime(&now));
    fprintf(SP->dbfp, "At: %8.8ld - %s ", (long) clock(), hms);

    va_start(args, fmt);
    vfprintf(SP->dbfp, fmt, args);
    va_end(args);

    /* If you are crashing and losing debugging information, enable this
       by setting the environment variable PDC_TRACE_FLUSH. This may
       impact performance. */

    if (want_fflush)
        fflush(SP->dbfp);

    /* If with PDC_TRACE_FLUSH enabled you are still losing logging in
       crashes, you may need to add a platform-dependent mechanism to
       flush the OS buffers as well (such as fsync() on POSIX) -- but
       expect terrible performance. */
}

void traceon(void)
{
    if (!SP)
        return;

    if (SP->dbfp)
        fclose(SP->dbfp);

    /* open debug log file append */
    SP->dbfp = fopen("trace", "a");
    if (!SP->dbfp)
    {
        fprintf(stderr, "PDC_debug(): Unable to open debug log file\n");
        return;
    }

    if (getenv("PDC_TRACE_FLUSH"))
        want_fflush = TRUE;

    PDC_LOG(("traceon() - called\n"));
}

void traceoff(void)
{
    if (!SP || !SP->dbfp)
        return;

    PDC_LOG(("traceoff() - called\n"));

    fclose(SP->dbfp);
    SP->dbfp = NULL;
    want_fflush = FALSE;
}
