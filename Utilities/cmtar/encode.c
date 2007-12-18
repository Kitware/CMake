/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  encode.c - libtar code to encode tar header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtarint/internal.h>

#include <stdio.h>
#ifndef WIN32
#include <pwd.h>
#include <grp.h>
#endif
#include <sys/types.h>

#ifdef STDC_HEADERS
# include <string.h>
# include <stdlib.h>
#endif


/* magic, version, and checksum */
void
th_finish(TAR *t)
{
  int i, sum = 0;

  if (t->options & TAR_GNU)
    strncpy(t->th_buf.magic, "ustar  ", 8);
  else
  {
    strncpy(t->th_buf.version, TVERSION, TVERSLEN);
    strncpy(t->th_buf.magic, TMAGIC, TMAGLEN);
  }

  for (i = 0; i < T_BLOCKSIZE; i++)
    sum += ((char *)(&(t->th_buf)))[i];
  for (i = 0; i < 8; i++)
    sum += (' ' - t->th_buf.chksum[i]);
  int_to_oct(sum, t->th_buf.chksum, 8);
}


/* map a file mode to a typeflag */
void
th_set_type(TAR *t, mode_t mode)
{
#ifdef S_ISLNK
  if (S_ISLNK(mode))
    t->th_buf.typeflag = SYMTYPE;
#endif
  if (S_ISREG(mode))
    t->th_buf.typeflag = REGTYPE;
  if (S_ISDIR(mode))
    t->th_buf.typeflag = DIRTYPE;
#ifdef S_ISCHR
  if (S_ISCHR(mode))
    t->th_buf.typeflag = CHRTYPE;
#endif
  if (S_ISBLK(mode))
    t->th_buf.typeflag = BLKTYPE;
  if (S_ISFIFO(mode)
#ifdef S_ISSOCK
     || S_ISSOCK(mode))
#else
    )
#endif
    t->th_buf.typeflag = FIFOTYPE;
}


/* encode file path */
void
th_set_path(TAR *t, char *pathname)
{
  char suffix[2] = "";
  char *tmp;

#ifdef DEBUG
  printf("in th_set_path(th, pathname=\"%s\")\n", pathname);
#endif

  if (t->th_buf.gnu_longname != NULL)
    free(t->th_buf.gnu_longname);
  t->th_buf.gnu_longname = NULL;

  if (pathname[strlen(pathname) - 1] != '/' && TH_ISDIR(t))
    strcpy(suffix, "/");

  if (strlen(pathname)+strlen(suffix) >= T_NAMELEN && (t->options & TAR_GNU))
  {
    /* GNU-style long name */
    t->th_buf.gnu_longname = strdup(pathname);
    strncpy(t->th_buf.name, t->th_buf.gnu_longname, T_NAMELEN);
  }
  else if (strlen(pathname)+ strlen(suffix) >= T_NAMELEN)
  {
    /* POSIX-style prefix field */
    tmp = strrchr(pathname, '/');
    if (tmp == NULL)
    {
      printf("!!! '/' not found in \"%s\"\n", pathname);
      return;
    }
    snprintf(t->th_buf.name, 100, "%s%s", &(tmp[1]), suffix);
    snprintf(t->th_buf.prefix,
       ((tmp - pathname + 1) <
        155 ? (tmp - pathname + 1) : 155), "%s", pathname);
  }
  else
    /* classic tar format */
    snprintf(t->th_buf.name, 100, "%s%s", pathname, suffix);

#ifdef DEBUG
  puts("returning from th_set_path()...");
#endif
}


/* encode link path */
void
th_set_link(TAR *t, char *linkname)
{
#ifdef DEBUG
  printf("==> th_set_link(th, linkname=\"%s\")\n", linkname);
#endif

  if (strlen(linkname) > T_NAMELEN && (t->options & TAR_GNU))
  {
    /* GNU longlink format */
    t->th_buf.gnu_longlink = strdup(linkname);
    strcpy(t->th_buf.linkname, "././@LongLink");
  }
  else
  {
    /* classic tar format */
    strlcpy(t->th_buf.linkname, linkname,
      sizeof(t->th_buf.linkname));
    if (t->th_buf.gnu_longlink != NULL)
      free(t->th_buf.gnu_longlink);
    t->th_buf.gnu_longlink = NULL;
  }
}


/* encode device info */
void
th_set_device(TAR *t, dev_t device)
{
#ifdef DEBUG
  printf("th_set_device(): major = %d, minor = %d\n",
         major(device), minor(device));
#endif
  int_to_oct(major(device), t->th_buf.devmajor, 8);
  int_to_oct(minor(device), t->th_buf.devminor, 8);
}


/* encode user info */
void
th_set_user(TAR *t, uid_t uid)
{
#ifndef WIN32
  struct passwd *pw;

  pw = getpwuid(uid);
  if (pw != NULL)
    strlcpy(t->th_buf.uname, pw->pw_name, sizeof(t->th_buf.uname));
#endif
  int_to_oct(uid, t->th_buf.uid, 8);
}


/* encode group info */
void
th_set_group(TAR *t, gid_t gid)
{
#ifndef WIN32
  struct group *gr;

  gr = getgrgid(gid);
  if (gr != NULL)
    strlcpy(t->th_buf.gname, gr->gr_name, sizeof(t->th_buf.gname));
#endif
  int_to_oct(gid, t->th_buf.gid, 8);
}


/* encode file mode */
void
th_set_mode(TAR *t, mode_t fmode)
{
#ifndef WIN32
#ifndef __BEOS__
  if (S_ISSOCK(fmode))
  {
    fmode &= ~S_IFSOCK;
    fmode |= S_IFIFO;
  }
#endif
#endif
  /* Looks like on windows the st_mode is longer than 8 characters. */
  int_to_oct(fmode & 07777777, (t)->th_buf.mode, 8);
}


void
th_set_from_stat(TAR *t, struct stat *s)
{
  th_set_type(t, s->st_mode);
#ifndef WIN32
  if (S_ISCHR(s->st_mode) || S_ISBLK(s->st_mode))
    th_set_device(t, s->st_rdev);
#endif
  th_set_user(t, s->st_uid);
  th_set_group(t, s->st_gid);
  th_set_mode(t, s->st_mode);
  th_set_mtime(t, s->st_mtime);
  if (S_ISREG(s->st_mode))
    th_set_size(t, s->st_size);
  else
    th_set_size(t, 0);
}


