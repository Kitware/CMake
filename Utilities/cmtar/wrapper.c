/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  wrapper.c - libtar high-level wrapper code
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtarint/internal.h>

#include <stdio.h>
#include <stdlib.h>

#include <libtar/compat.h>
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#else
#include <libtarint/filesystem.h>
#endif
#include <errno.h>

#ifdef HAVE_FNMATCH_H
#include <fnmatch.h>
#endif

#ifdef STDC_HEADERS
# include <string.h>
#endif


int
tar_extract_glob(TAR *t, char *globname, char *prefix)
{
  char *filename;
  char buf[TAR_MAXPATHLEN];
  int i;
  char *pathname;

  while ((i = th_read(t)) == 0)
  {
    pathname = th_get_pathname(t);
    filename = pathname;

    if (fnmatch(globname, filename, FNM_PATHNAME | FNM_PERIOD))
    {
      if (pathname)
        {
        free(pathname);
        }

      if (TH_ISREG(t) && tar_skip_regfile(t))
        return -1;
      continue;
    }

    if (t->options & TAR_VERBOSE)
      th_print_long_ls(t);

    if (prefix != NULL)
      snprintf(buf, sizeof(buf), "%s/%s", prefix, filename);
    else
      strlcpy(buf, filename, sizeof(buf));

    if (tar_extract_file(t, filename) != 0)
      {
      if (pathname)
        {
        free(pathname);
        }
      return -1;
      }

    if (pathname)
      {
      free(pathname);
      }
  }

  return (i == 1 ? 0 : -1);
}


int
tar_extract_all(TAR *t, char *prefix)
{
  char *filename;
  char buf[TAR_MAXPATHLEN];
  int i;
  char *pathname;

#ifdef DEBUG
  printf("==> tar_extract_all(TAR *t, \"%s\")\n",
         (prefix ? prefix : "(null)"));
#endif

  while ((i = th_read(t)) == 0)
  {
#ifdef DEBUG
    puts("    tar_extract_all(): calling th_get_pathname()");
#endif

    pathname = th_get_pathname(t);
    filename = pathname;

    if (t->options & TAR_VERBOSE)
      th_print_long_ls(t);
    if (prefix != NULL)
      snprintf(buf, sizeof(buf), "%s/%s", prefix, filename);
    else
      strlcpy(buf, filename, sizeof(buf));

    if (pathname)
      {
      free(pathname);
      }

#ifdef DEBUG
    printf("    tar_extract_all(): calling tar_extract_file(t, "
           "\"%s\")\n", buf);
#endif

    if (tar_extract_file(t, buf) != 0)
      return -1;
  }

  return (i == 1 ? 0 : -1);
}


int
tar_append_tree(TAR *t, char *realdir, char *savedir)
{
  char realpath[TAR_MAXPATHLEN];
  char savepath[TAR_MAXPATHLEN];
  size_t plen;
#if defined(HAVE_DIRENT_H)
  struct dirent *dent;
  DIR *dp;
#else  
  kwDirEntry * dent;
  kwDirectory *dp;
#endif  
  struct stat s;
  strncpy(realpath, realdir, sizeof(realpath));
  realpath[sizeof(realpath)-1] = 0;
  plen = strlen(realpath);
  if ( realpath[plen-1] == '/' )
    {
    realpath[plen-1] = 0;
    }
  

#ifdef DEBUG
  printf("==> tar_append_tree(0x%lx, \"%s\", \"%s\")\n",
         t, realdir, (savedir ? savedir : "[NULL]"));
#endif

  if (tar_append_file(t, realdir, savedir) != 0)
    return -1;

#ifdef DEBUG
  puts("    tar_append_tree(): done with tar_append_file()...");
#endif

  if ( stat(realpath, &s) != 0 )
    {
    return -1;   
    }
  if ( 
#if defined(_WIN32) && !defined(__CYGWIN__)
    (s.st_mode & _S_IFDIR) == 0
#else
    !S_ISDIR(s.st_mode)
#endif
  )
    return 0;
#if defined(HAVE_DIRENT_H)
  dp = opendir(realdir);
#else
  dp = kwOpenDir(realdir);
#endif

  if (dp == NULL)
  {
    if (errno == ENOTDIR)
      return 0;
    return -1;
  }
#if defined(HAVE_DIRENT_H)
  while ((dent = readdir(dp)) != NULL)
#else
  while ((dent = kwReadDir(dp)) != NULL)
#endif
  {
    if (strcmp(dent->d_name, ".") == 0 ||
        strcmp(dent->d_name, "..") == 0)
      continue;

    snprintf(realpath, TAR_MAXPATHLEN, "%s/%s", realdir,
       dent->d_name);
    if (savedir)
      snprintf(savepath, TAR_MAXPATHLEN, "%s/%s", savedir,
         dent->d_name);

#ifndef WIN32
    if (lstat(realpath, &s) != 0)
      return -1;
#else
    if (stat(realpath, &s) != 0)
      return -1;
#endif
    if (S_ISDIR(s.st_mode))
    {
      if (tar_append_tree(t, realpath,
              (savedir ? savepath : NULL)) != 0)
        return -1;
      continue;
    }

    if (tar_append_file(t, realpath,
            (savedir ? savepath : NULL)) != 0)
      return -1;
  }

#if defined(HAVE_DIRENT_H)
  closedir(dp);
#else
  kwCloseDir(dp);
#endif

  return 0;
}


