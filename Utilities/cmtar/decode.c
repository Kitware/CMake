/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  decode.c - libtar code to decode tar header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtarint/internal.h>

#include <stdio.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
# include <libtar/compat.h>
#else
# include <sys/param.h>
#endif

#ifndef WIN32
#include <pwd.h>
#include <grp.h>
#endif

#ifdef STDC_HEADERS
# include <string.h>
#endif


/* determine full path name */
/* caller must "free" returned pointer when done with it */
/* th_get_pathname return values come directly from strdup */
char *
th_get_pathname(TAR *t)
{
  char filename[TAR_MAXPATHLEN];

  if (t->th_buf.gnu_longname)
    return strdup(t->th_buf.gnu_longname);

  if (t->th_buf.prefix[0] != '\0')
  {
    snprintf(filename, sizeof(filename), "%.155s/%.100s",
       t->th_buf.prefix, t->th_buf.name);
    return strdup(filename);
  }

  snprintf(filename, sizeof(filename), "%.100s", t->th_buf.name);
  return strdup(filename);
}


uid_t
th_get_uid(TAR *t)
{
  int uid;
#ifndef WIN32
  struct passwd *pw;

  pw = getpwnam(t->th_buf.uname);
  if (pw != NULL)
    return pw->pw_uid;

  /* if the password entry doesn't exist */
#endif
  sscanf(t->th_buf.uid, "%o", &uid);
  return uid;
}


gid_t
th_get_gid(TAR *t)
{
  int gid;
#ifndef WIN32
  struct group *gr;

  gr = getgrnam(t->th_buf.gname);
  if (gr != NULL)
    return gr->gr_gid;

  /* if the group entry doesn't exist */
#endif
  sscanf(t->th_buf.gid, "%o", &gid);
  return gid;
}


mode_t
th_get_mode(TAR *t)
{
  mode_t mode;

  mode = (mode_t)oct_to_int(t->th_buf.mode);
  if (! (mode & S_IFMT))
  {
    switch (t->th_buf.typeflag)
    {
#ifndef WIN32
    case SYMTYPE:
      mode |= S_IFLNK;
      break;
#endif
    case CHRTYPE:
      mode |= S_IFCHR;
      break;
    case BLKTYPE:
      mode |= S_IFBLK;
      break;
    case DIRTYPE:
      mode |= S_IFDIR;
      break;
#ifndef WIN32
    case FIFOTYPE:
      mode |= S_IFIFO;
      break;
#endif
    case AREGTYPE:
      if (t->th_buf.name[strlen(t->th_buf.name) - 1] == '/')
      {
        mode |= S_IFDIR;
        break;
      }
      /* FALLTHROUGH */
    case LNKTYPE:
    case REGTYPE:
    default:
      mode |= S_IFREG;
    }
  }

  return mode;
}


