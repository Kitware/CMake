/* prototypes for borrowed "compatibility" code */

#include <libtar/config.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef STDC_HEADERS
# include <stdarg.h>
# include <stddef.h>
#else
# include <varargs.h>
#endif

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
#if defined(NEED_BASENAME) && !defined(HAVE_BASENAME)

# ifdef basename
#  undef basename    /* fix glibc brokenness */
# endif

char *openbsd_basename(const char *);
# define basename openbsd_basename

#endif /* NEED_BASENAME && ! HAVE_BASENAME */


#if defined(NEED_DIRNAME) && !defined(HAVE_DIRNAME)

char *openbsd_dirname(const char *);
# define dirname openbsd_dirname

#endif /* NEED_DIRNAME && ! HAVE_DIRNAME */


#ifdef NEED_FNMATCH
# ifndef HAVE_FNMATCH

#  define FNM_NOMATCH  1  /* Match failed. */

#  define FNM_NOESCAPE  0x01  /* Disable backslash escaping. */
#  define FNM_PATHNAME  0x02  /* Slash must be matched by slash. */
#  define FNM_PERIOD  0x04  /* Period must be matched by period. */

#  define FNM_LEADING_DIR 0x08  /* Ignore /<tail> after Imatch. */
#  define FNM_CASEFOLD  0x10  /* Case insensitive search. */
#  define FNM_IGNORECASE FNM_CASEFOLD
#  define FNM_FILE_NAME FNM_PATHNAME

int openbsd_fnmatch(const char *, const char *, int);
#  define fnmatch openbsd_fnmatch

# else /* HAVE_FNMATCH */

#  ifdef HAVE_FNMATCH_H
#   include <fnmatch.h>
#  endif

# endif /* ! HAVE_FNMATCH */
#endif /* NEED_FNMATCH */


#ifdef NEED_GETHOSTBYNAME_R

# include <netdb.h>

# if GETHOSTBYNAME_R_NUM_ARGS != 6

int compat_gethostbyname_r(const char *, struct hostent *,
         char *, size_t, struct hostent **, int *);

#  define gethostbyname_r compat_gethostbyname_r

# endif /* GETHOSTBYNAME_R_NUM_ARGS != 6 */

#endif /* NEED_GETHOSTBYNAME_R */


#if defined(NEED_GETHOSTNAME) && !defined(HAVE_GETHOSTNAME)

int gethostname(char *, size_t);

#endif /* NEED_GETHOSTNAME && ! HAVE_GETHOSTNAME */


#ifdef NEED_GETSERVBYNAME_R

# include <netdb.h>

# if GETSERVBYNAME_R_NUM_ARGS != 6

int compat_getservbyname_r(const char *, const char *, struct servent *,
         char *, size_t, struct servent **);

#  define getservbyname_r compat_getservbyname_r

# endif /* GETSERVBYNAME_R_NUM_ARGS != 6 */

#endif /* NEED_GETSERVBYNAME_R */



#ifdef NEED_GLOB
# ifndef HAVE_GLOB

typedef struct {
  int gl_pathc;    /* Count of total paths so far. */
  int gl_matchc;    /* Count of paths matching pattern. */
  int gl_offs;    /* Reserved at beginning of gl_pathv. */
  int gl_flags;    /* Copy of flags parameter to glob. */
  char **gl_pathv;  /* List of paths matching pattern. */
        /* Copy of errfunc parameter to glob. */
  int (*gl_errfunc)(const char *, int);

  /*
   * Alternate filesystem access methods for glob; replacement
   * versions of closedir(3), readdir(3), opendir(3), stat(2)
   * and lstat(2).
   */
  void (*gl_closedir)(void *);
  struct dirent *(*gl_readdir)(void *);
  void *(*gl_opendir)(const char *);
  int (*gl_lstat)(const char *, struct stat *);
  int (*gl_stat)(const char *, struct stat *);
} glob_t;

/* Flags */
#  define GLOB_APPEND  0x0001  /* Append to output from previous call. */
#  define GLOB_DOOFFS  0x0002  /* Use gl_offs. */
#  define GLOB_ERR  0x0004  /* Return on error. */
#  define GLOB_MARK  0x0008  /* Append / to matching directories. */
#  define GLOB_NOCHECK  0x0010  /* Return pattern itself if nothing matches. */
#  define GLOB_NOSORT  0x0020  /* Don't sort. */

#  define GLOB_ALTDIRFUNC 0x0040 /* Use alternately specified directory funcs. */
#  define GLOB_BRACE  0x0080  /* Expand braces ala csh. */
#  define GLOB_MAGCHAR  0x0100  /* Pattern had globbing characters. */
#  define GLOB_NOMAGIC  0x0200  /* GLOB_NOCHECK without magic chars (csh). */
#  define GLOB_QUOTE  0x0400  /* Quote special chars with \. */
#  define GLOB_TILDE  0x0800  /* Expand tilde names from the passwd file. */
#  define GLOB_NOESCAPE  0x1000  /* Disable backslash escaping. */

/* Error values returned by glob(3) */
#  define GLOB_NOSPACE  (-1)  /* Malloc call failed. */
#  define GLOB_ABORTED  (-2)  /* Unignored error. */
#  define GLOB_NOMATCH  (-3)  /* No match and GLOB_NOCHECK not set. */
#  define GLOB_NOSYS  (-4)  /* Function not supported. */
#  define GLOB_ABEND  GLOB_ABORTED

int openbsd_glob(const char *, int, int (*)(const char *, int), glob_t *);
void openbsd_globfree(glob_t *);
#  define glob openbsd_glob
#  define globfree openbsd_globfree

# else /* HAVE_GLOB */

#  ifdef HAVE_GLOB_H
#   include <glob.h>
#  endif

# endif /* ! HAVE_GLOB */
#endif /* NEED_GLOB */


#if defined(NEED_INET_ATON) && !defined(HAVE_INET_ATON)

int inet_aton(const char *, struct in_addr *);

#endif /* NEED_INET_ATON && ! HAVE_INET_ATON */


#ifdef NEED_MAKEDEV

# ifdef MAJOR_IN_MKDEV
#  include <sys/mkdev.h>
# else
#  ifdef MAJOR_IN_SYSMACROS
#   include <sys/sysmacros.h>
#  endif
# endif

/*
** On most systems makedev() has two args.
** Some weird systems, like QNX6, have makedev() functions that expect
** an extra first argument for "node", which can be 0 for a local
** machine.
*/

# ifdef MAKEDEV_THREE_ARGS
#  define compat_makedev(maj, min)  makedev(0, maj, min)
# else
#  define compat_makedev    makedev
# endif

#endif /* NEED_MAKEDEV */

#ifdef _MSC_VER /* compile snprintf only onwin32 */
#if !defined(HAVE_SNPRINTF)
int mutt_snprintf(char *, size_t, const char *, ...);
# define snprintf mutt_snprintf
#endif

#if !defined(HAVE_VSNPRINTF)
int mutt_vsnprintf(char *, size_t, const char *, va_list);
# define vsnprintf mutt_vsnprintf
#endif /* NEED_SNPRINTF && ! HAVE_SNPRINTF */
#endif

#if defined(NEED_STRLCAT) && !defined(HAVE_STRLCAT)

size_t strlcat(char *, const char *, size_t);

#endif /* NEED_STRLCAT && ! HAVE_STRLCAT */


#if defined(NEED_STRLCPY) && !defined(HAVE_STRLCPY)

size_t strlcpy(char *, const char *, size_t);

#endif /* NEED_STRLCPY && ! HAVE_STRLCPY */


#if defined(NEED_STRDUP) && !defined(HAVE_STRDUP)

char *openbsd_strdup(const char *);
# define strdup openbsd_strdup

#endif /* NEED_STRDUP && ! HAVE_STRDUP */


#if defined(NEED_STRMODE) && !defined(HAVE_STRMODE)

void strmode(register mode_t, register char *);

#endif /* NEED_STRMODE && ! HAVE_STRMODE */


#if defined(NEED_STRRSTR) && !defined(HAVE_STRRSTR)

char *strrstr(char *, char *);

#endif /* NEED_STRRSTR && ! HAVE_STRRSTR */


#ifdef NEED_STRSEP

# ifdef HAVE_STRSEP
#  define _LINUX_SOURCE_COMPAT    /* needed on AIX 4.3.3 */
# else

char *strsep(register char **, register const char *);

# endif

#endif /* NEED_STRSEP */

#ifdef _MSC_VER
#include <stdlib.h>
#define MAXPATHLEN _MAX_PATH
#ifndef O_ACCMODE
# define O_ACCMODE 0x0003
#endif
#endif




#ifndef S_ISREG
#ifndef S_IFREG
#ifndef _S_IFREG
#define S_IFREG    (-1)
#else
#define S_IFREG    _S_IFREG
#endif
#endif
#define S_ISREG(m)  (((m)&S_IFREG)==S_IFREG)
#endif


#ifndef S_ISDIR
#ifndef S_IFDIR
#ifndef _S_IFDIR
#define S_IFDIR    (-1)
#else
#define S_IFDIR    _S_IFDIR
#endif
#endif
#define S_ISDIR(m)  (((m)&S_IFDIR)==S_IFDIR)
#endif

#ifndef S_ISBLK
#ifndef S_IFBLK
#ifndef _S_IFBLK
#define S_IFBLK    (-1)
#else
#define S_IFBLK    _S_IFBLK
#endif
#endif
#define S_ISBLK(m)  (((m)&S_IFBLK)==S_IFBLK)
#endif

#ifndef S_ISFIFO
#ifndef S_IFFIFO
#ifndef _S_IFFIFO
#define S_IFFIFO    (-1)
#else
#define S_IFFIFO    _S_IFFIFO
#endif
#endif
#define S_ISFIFO(m)  (((m)&S_IFFIFO)==S_IFFIFO)
#endif

#if !defined(TAR_MAXPATHLEN)
# if defined(PATH_MAX)
#  define TAR_MAXPATHLEN PATH_MAX
# elif defined(MAXPATHLEN)
#  define TAR_MAXPATHLEN MAXPATHLEN
# else
#  define TAR_MAXPATHLEN 16384
# endif
#endif

#ifdef __cplusplus
}
#endif
