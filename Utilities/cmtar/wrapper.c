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
#include <libtar/compat.h>
#ifdef _MSC_VER
#include <libtarint/filesystem.h>
#else
#include <sys/param.h>
#include <dirent.h>
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
  char buf[MAXPATHLEN];
  int i;

  while ((i = th_read(t)) == 0)
  {
    filename = th_get_pathname(t);
    if (fnmatch(globname, filename, FNM_PATHNAME | FNM_PERIOD))
    {
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
      return -1;
  }

  return (i == 1 ? 0 : -1);
}


int
tar_extract_all(TAR *t, char *prefix)
{
  char *filename;
  char buf[MAXPATHLEN];
  int i;

#ifdef DEBUG
  printf("==> tar_extract_all(TAR *t, \"%s\")\n",
         (prefix ? prefix : "(null)"));
#endif

  while ((i = th_read(t)) == 0)
  {
#ifdef DEBUG
    puts("    tar_extract_all(): calling th_get_pathname()");
#endif
    filename = th_get_pathname(t);
    if (t->options & TAR_VERBOSE)
      th_print_long_ls(t);
    if (prefix != NULL)
      snprintf(buf, sizeof(buf), "%s/%s", prefix, filename);
    else
      strlcpy(buf, filename, sizeof(buf));
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
  char realpath[MAXPATHLEN];
  char savepath[MAXPATHLEN];
#ifndef _MSC_VER
  struct dirent *dent;
  DIR *dp;
#else  
  kwDirEntry * dent;
  kwDirectory *dp;
#endif  
  struct stat s;

#ifdef DEBUG
  printf("==> tar_append_tree(0x%lx, \"%s\", \"%s\")\n",
         t, realdir, (savedir ? savedir : "[NULL]"));
#endif

  if (tar_append_file(t, realdir, savedir) != 0)
    return -1;

#ifdef DEBUG
  puts("    tar_append_tree(): done with tar_append_file()...");
#endif

#ifdef _MSC_VER
  dp = kwOpenDir(realdir);
#else
  dp = opendir(realdir);
#endif

  if (dp == NULL)
  {
    if (errno == ENOTDIR)
      return 0;
    return -1;
  }
#ifdef _MSC_VER
  while ((dent = kwReadDir(dp)) != NULL)
#else
  while ((dent = readdir(dp)) != NULL)
#endif
  {
    if (strcmp(dent->d_name, ".") == 0 ||
        strcmp(dent->d_name, "..") == 0)
      continue;

    snprintf(realpath, MAXPATHLEN, "%s/%s", realdir,
       dent->d_name);
    if (savedir)
      snprintf(savepath, MAXPATHLEN, "%s/%s", savedir,
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

#ifdef _MSC_VER
  kwCloseDir(dp);
#else
  closedir(dp);
#endif

  return 0;
}


