/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  extract.c - libtar code to extract a file from a tar archive
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtarint/internal.h>

#include <stdio.h>
#include <libtar/compat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
# ifdef _MSC_VER
#  include <sys/utime.h>
# else
#  include <utime.h>
# endif
# include <io.h>
# include <direct.h>
#else
# include <utime.h>
# include <sys/param.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_MKDEV_H
# include <sys/mkdev.h>
#endif


struct linkname
{
  char ln_save[TAR_MAXPATHLEN];
  char ln_real[TAR_MAXPATHLEN];
};
typedef struct linkname linkname_t;


static int
tar_set_file_perms(TAR *t, char *realname)
{
  mode_t mode;
  uid_t uid;
  gid_t gid;
  struct utimbuf ut;
  char *filename;
  char *pathname = 0;

  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }

  mode = th_get_mode(t);
  uid = th_get_uid(t);
  gid = th_get_gid(t);
  ut.modtime = ut.actime = th_get_mtime(t);

  /* change owner/group */
#ifndef WIN32
  if (geteuid() == 0)
#ifdef HAVE_LCHOWN
    if (lchown(filename, uid, gid) == -1)
    {
# ifdef DEBUG
      fprintf(stderr, "lchown(\"%s\", %d, %d): %s\n",
        filename, uid, gid, strerror(errno));
# endif
#else /* ! HAVE_LCHOWN */
    if (!TH_ISSYM(t) && chown(filename, uid, gid) == -1)
    {
# ifdef DEBUG
      fprintf(stderr, "chown(\"%s\", %d, %d): %s\n",
        filename, uid, gid, strerror(errno));
# endif
#endif /* HAVE_LCHOWN */
    if (pathname)
      {
      free(pathname);
      }
      return -1;
    }

  /* change access/modification time */
  if (!TH_ISSYM(t) && utime(filename, &ut) == -1)
  {
#ifdef DEBUG
    perror("utime()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }
  /* change permissions */
  if (!TH_ISSYM(t) && chmod(filename, mode) == -1)
  {
#ifdef DEBUG
    perror("chmod()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

#else /* WIN32 */
  (void)filename;
  (void)gid;
  (void)uid;
  (void)mode;
#endif /* WIN32 */

  if (pathname)
    {
    free(pathname);
    }
  return 0;
}


/* switchboard */
int
tar_extract_file(TAR *t, char *realname)
{
  int i;
  linkname_t *lnp;
  char *pathname;

  if (t->options & TAR_NOOVERWRITE)
  {
    struct stat s;

#ifdef WIN32
    if (stat(realname, &s) == 0 || errno != ENOENT)
#else
    if (lstat(realname, &s) == 0 || errno != ENOENT)
#endif
    {
      errno = EEXIST;
      return -1;
    }
  }

  if (TH_ISDIR(t))
  {
    i = tar_extract_dir(t, realname);
    if (i == 1)
      i = 0;
  }
#ifndef _WIN32
  else if (TH_ISLNK(t))
    i = tar_extract_hardlink(t, realname);
  else if (TH_ISSYM(t))
    i = tar_extract_symlink(t, realname);
  else if (TH_ISCHR(t))
    i = tar_extract_chardev(t, realname);
  else if (TH_ISBLK(t))
    i = tar_extract_blockdev(t, realname);
  else if (TH_ISFIFO(t))
    i = tar_extract_fifo(t, realname);
#endif
  else /* if (TH_ISREG(t)) */
    i = tar_extract_regfile(t, realname);

  if (i != 0)
    return i;

  i = tar_set_file_perms(t, realname);
  if (i != 0)
    return i;

  lnp = (linkname_t *)calloc(1, sizeof(linkname_t));
  if (lnp == NULL)
    return -1;
  pathname = th_get_pathname(t);
  strlcpy(lnp->ln_save, pathname, sizeof(lnp->ln_save));
  strlcpy(lnp->ln_real, realname, sizeof(lnp->ln_real));
#ifdef DEBUG
  printf("tar_extract_file(): calling libtar_hash_add(): key=\"%s\", "
         "value=\"%s\"\n", pathname, realname);
#endif
  if (pathname)
    {
    free(pathname);
    }
  if (libtar_hash_add(t->h, lnp) != 0)
    return -1;

  return 0;
}


/* extract regular file */
int
tar_extract_regfile(TAR *t, char *realname)
{
  mode_t mode;
  size_t size;
  uid_t uid;
  gid_t gid;
  int fdout;
  ssize_t i, k;
  char buf[T_BLOCKSIZE];
  char *filename;
  char *pathname = 0;

#ifdef DEBUG
  printf("==> tar_extract_regfile(t=0x%lx, realname=\"%s\")\n", t,
         realname);
#endif

  if (!TH_ISREG(t))
  {
    errno = EINVAL;
    return -1;
  }

  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }
  mode = th_get_mode(t);
  size = th_get_size(t);
  uid = th_get_uid(t);
  gid = th_get_gid(t);

  /* Make a copy of the string because dirname and mkdirhier may modify the
   * string */
  strncpy(buf, filename, sizeof(buf)-1);
  buf[sizeof(buf)-1] = 0;

  if (mkdirhier(dirname(buf)) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

#ifdef DEBUG
  printf("  ==> extracting: %s (mode %04o, uid %d, gid %d, %d bytes)\n",
         filename, mode, uid, gid, size);
#endif
  fdout = open(filename, O_WRONLY | O_CREAT | O_TRUNC
#ifdef O_BINARY
         | O_BINARY
#endif
        , 0666);
  if (fdout == -1)
  {
#ifdef DEBUG
    perror("open()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

#if 0
  /* change the owner.  (will only work if run as root) */
  if (fchown(fdout, uid, gid) == -1 && errno != EPERM)
  {
#ifdef DEBUG
    perror("fchown()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

  /* make sure the mode isn't inheritted from a file we're overwriting */
  if (fchmod(fdout, mode & 07777) == -1)
  {
#ifdef DEBUG
    perror("fchmod()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }
#endif

  /* extract the file */
  for (i = size; i > 0; i -= T_BLOCKSIZE)
  {
    k = tar_block_read(t, buf);
    if (k != T_BLOCKSIZE)
    {
      if (k != -1)
        errno = EINVAL;
      if (pathname)
        {
        free(pathname);
        }
      return -1;
    }

    /* write block to output file */
    if (write(fdout, buf,
        ((i > T_BLOCKSIZE) ? T_BLOCKSIZE : (unsigned int)i)) == -1)
      {
      if (pathname)
        {
        free(pathname);
        }
      return -1;
      }
  }

  /* close output file */
  if (close(fdout) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

#ifdef DEBUG
  printf("### done extracting %s\n", filename);
#endif

  (void)filename;
  (void)gid;
  (void)uid;
  (void)mode;

  if (pathname)
    {
    free(pathname);
    }
  return 0;
}


/* skip regfile */
int
tar_skip_regfile(TAR *t)
{
  ssize_t i, k;
  size_t size;
  char buf[T_BLOCKSIZE];

  if (!TH_ISREG(t))
  {
    errno = EINVAL;
    return -1;
  }

  size = th_get_size(t);
  for (i = size; i > 0; i -= T_BLOCKSIZE)
  {
    k = tar_block_read(t, buf);
    if (k != T_BLOCKSIZE)
    {
      if (k != -1)
        errno = EINVAL;
      return -1;
    }
  }

  return 0;
}


/* hardlink */
int
tar_extract_hardlink(TAR * t, char *realname)
{
  char *filename;
  char *linktgt;
  linkname_t *lnp;
  libtar_hashptr_t hp;
  char buf[T_BLOCKSIZE];
  char *pathname = 0;

  if (!TH_ISLNK(t))
  {
    errno = EINVAL;
    return -1;
  }

  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }

  /* Make a copy of the string because dirname and mkdirhier may modify the
   * string */
  strncpy(buf, filename, sizeof(buf)-1);
  buf[sizeof(buf)-1] = 0;

  if (mkdirhier(dirname(buf)) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }
  libtar_hashptr_reset(&hp);
  if (libtar_hash_getkey(t->h, &hp, th_get_linkname(t),
             (libtar_matchfunc_t)libtar_str_match) != 0)
  {
    lnp = (linkname_t *)libtar_hashptr_data(&hp);
    linktgt = lnp->ln_real;
  }
  else
    linktgt = th_get_linkname(t);

#ifdef DEBUG
  printf("  ==> extracting: %s (link to %s)\n", filename, linktgt);
#endif
#ifndef WIN32
  if (link(linktgt, filename) == -1)
#else
  (void)linktgt;
#endif
  {
#ifdef DEBUG
    perror("link()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

#ifndef WIN32
  if (pathname)
    {
    free(pathname);
    }
  return 0;
#endif
}


/* symlink */
int
tar_extract_symlink(TAR *t, char *realname)
{
  char *filename;
  char buf[T_BLOCKSIZE];
  char *pathname = 0;

#ifndef _WIN32
  if (!TH_ISSYM(t))
  {
    errno = EINVAL;
    return -1;
  }
#endif

  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }

  /* Make a copy of the string because dirname and mkdirhier may modify the
   * string */
  strncpy(buf, filename, sizeof(buf)-1);
  buf[sizeof(buf)-1] = 0;

  if (mkdirhier(dirname(buf)) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

  if (unlink(filename) == -1 && errno != ENOENT)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

#ifdef DEBUG
  printf("  ==> extracting: %s (symlink to %s)\n",
         filename, th_get_linkname(t));
#endif
#ifndef WIN32
  if (symlink(th_get_linkname(t), filename) == -1)
#endif
  {
#ifdef DEBUG
    perror("symlink()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

#ifndef WIN32
  if (pathname)
    {
    free(pathname);
    }
  return 0;
#endif
}


/* character device */
int
tar_extract_chardev(TAR *t, char *realname)
{
  mode_t mode;
  unsigned long devmaj, devmin;
  char *filename;
  char buf[T_BLOCKSIZE];
  char *pathname = 0;

#ifndef _WIN32
  if (!TH_ISCHR(t))
  {
    errno = EINVAL;
    return -1;
  }
#endif
  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }
  mode = th_get_mode(t);
  devmaj = th_get_devmajor(t);
  devmin = th_get_devminor(t);

  /* Make a copy of the string because dirname and mkdirhier may modify the
   * string */
  strncpy(buf, filename, sizeof(buf)-1);
  buf[sizeof(buf)-1] = 0;

  if (mkdirhier(dirname(buf)) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

#ifdef DEBUG
  printf("  ==> extracting: %s (character device %ld,%ld)\n",
         filename, devmaj, devmin);
#endif
#ifndef WIN32
  if (mknod(filename, mode | S_IFCHR,
      compat_makedev(devmaj, devmin)) == -1)
#else
  (void)devmin;
  (void)devmaj;
  (void)mode;
#endif
  {
#ifdef DEBUG
    perror("mknod()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

#ifndef WIN32
  if (pathname)
    {
    free(pathname);
    }
  return 0;
#endif
}


/* block device */
int
tar_extract_blockdev(TAR *t, char *realname)
{
  mode_t mode;
  unsigned long devmaj, devmin;
  char *filename;
  char buf[T_BLOCKSIZE];
  char *pathname = 0;

  if (!TH_ISBLK(t))
  {
    errno = EINVAL;
    return -1;
  }

  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }
  mode = th_get_mode(t);
  devmaj = th_get_devmajor(t);
  devmin = th_get_devminor(t);

  /* Make a copy of the string because dirname and mkdirhier may modify the
   * string */
  strncpy(buf, filename, sizeof(buf)-1);
  buf[sizeof(buf)-1] = 0;

  if (mkdirhier(dirname(buf)) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

#ifdef DEBUG
  printf("  ==> extracting: %s (block device %ld,%ld)\n",
         filename, devmaj, devmin);
#endif
#ifndef WIN32
  if (mknod(filename, mode | S_IFBLK,
      compat_makedev(devmaj, devmin)) == -1)
#else
  (void)devmin;
  (void)devmaj;
  (void)mode;
#endif
  {
#ifdef DEBUG
    perror("mknod()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

#ifndef WIN32
  if (pathname)
    {
    free(pathname);
    }
  return 0;
#endif
}


/* directory */
int
tar_extract_dir(TAR *t, char *realname)
{
  mode_t mode;
  char *filename;
  char buf[T_BLOCKSIZE];
  char *pathname = 0;

  if (!TH_ISDIR(t))
  {
    errno = EINVAL;
    return -1;
  }

  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }
  mode = th_get_mode(t);

  /* Make a copy of the string because dirname and mkdirhier may modify the
   * string */
  strncpy(buf, filename, sizeof(buf)-1);
  buf[sizeof(buf)-1] = 0;

  if (mkdirhier(dirname(buf)) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

#ifdef DEBUG
  printf("  ==> extracting: %s (mode %04o, directory)\n", filename,
         mode);
#endif
#ifdef WIN32
  if (mkdir(filename) == -1)
#else
  if (mkdir(filename, mode) == -1)
#endif
  {
#ifdef __BORLANDC__
  /* There is a bug in the Borland Run time library which makes MKDIR
     return EACCES when it should return EEXIST
     if it is some other error besides directory exists
     then return false */
    if ( errno == EACCES) 
    {
      errno = EEXIST;
    }
#endif      
    if (errno == EEXIST)
    {
      if (chmod(filename, mode) == -1)
      {
#ifdef DEBUG
        perror("chmod()");
#endif
        if (pathname)
          {
          free(pathname);
          }
        return -1;
      }
      else
      {
#ifdef DEBUG
        puts("  *** using existing directory");
#endif
        if (pathname)
          {
          free(pathname);
          }
        return 1;
      }
    }
    else
    {
#ifdef DEBUG
      perror("mkdir()");
#endif
      if (pathname)
        {
        free(pathname);
        }
      return -1;
    }
  }

  if (pathname)
    {
    free(pathname);
    }
  return 0;
}


/* FIFO */
int
tar_extract_fifo(TAR *t, char *realname)
{
  mode_t mode;
  char *filename;
  char buf[T_BLOCKSIZE];
  char *pathname = 0;

  if (!TH_ISFIFO(t))
  {
    errno = EINVAL;
    return -1;
  }

  if (realname)
    {
    filename = realname;
    }
  else
    {
    pathname = th_get_pathname(t);
    filename = pathname;
    }
  mode = th_get_mode(t);

  /* Make a copy of the string because dirname and mkdirhier may modify the
   * string */
  strncpy(buf, filename, sizeof(buf)-1);
  buf[sizeof(buf)-1] = 0;

  if (mkdirhier(dirname(buf)) == -1)
    {
    if (pathname)
      {
      free(pathname);
      }
    return -1;
    }

#ifdef DEBUG
  printf("  ==> extracting: %s (fifo)\n", filename);
#endif
#ifndef WIN32
  if (mkfifo(filename, mode) == -1)
#else
    (void)mode;
#endif
  {
#ifdef DEBUG
    perror("mkfifo()");
#endif
    if (pathname)
      {
      free(pathname);
      }
    return -1;
  }

#ifndef WIN32
  if (pathname)
    {
    free(pathname);
    }
  return 0;
#endif
}
