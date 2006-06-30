/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  handle.c - libtar code for initializing a TAR handle
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtarint/internal.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
//Yogi: hack. this should work on windows where there is no O_ACCMODE defined
#ifndef O_ACCMODE
# define O_ACCMODE 0x0003
#endif
#endif

const char libtar_version[] = PACKAGE_VERSION;

struct libtar_fd_file
{
  int fd;
};

static struct libtar_fd_file libtar_fd_file_pointer;

static int libtar_open(void* call_data, const char* pathname, int flags, mode_t mode)
{
  struct libtar_fd_file* lf = (struct libtar_fd_file*)call_data;
  lf->fd = open(pathname, flags, mode);
  return lf->fd;
}

static int libtar_close(void* call_data)
{
  struct libtar_fd_file* lf = (struct libtar_fd_file*)call_data;
  return close(lf->fd);
}
static ssize_t libtar_read(void* call_data, void* buf, size_t count)
{
  struct libtar_fd_file* lf = (struct libtar_fd_file*)call_data;
  return (ssize_t)read(lf->fd, buf, (unsigned int)count);
}
static ssize_t libtar_write(void* call_data, const void* buf, size_t count)
{
  struct libtar_fd_file* lf = (struct libtar_fd_file*)call_data;
  return (ssize_t) write(lf->fd, buf, (unsigned int)count);
}

static tartype_t default_type = { libtar_open, libtar_close, libtar_read, libtar_write, &libtar_fd_file_pointer };

static int
tar_init(TAR **t, char *pathname, tartype_t *type,
   int oflags, int mode, int options)
{
  (void)mode;
  if ((oflags & O_ACCMODE) == O_RDWR)
  {
    errno = EINVAL;
    return -1;
  }

  *t = (TAR *)calloc(1, sizeof(TAR));
  if (*t == NULL)
    return -1;

  (*t)->pathname = pathname;
  (*t)->options = options;
  (*t)->type = (type ? type : &default_type);
  (*t)->oflags = oflags;

  if ((oflags & O_ACCMODE) == O_RDONLY)
    (*t)->h = libtar_hash_new(256,
            (libtar_hashfunc_t)path_hashfunc);
  else
    (*t)->h = libtar_hash_new(16, (libtar_hashfunc_t)dev_hash);
  if ((*t)->h == NULL)
  {
    free(*t);
    return -1;
  }

  return 0;
}


/* open a new tarfile handle */
int
tar_open(TAR **t, char *pathname, tartype_t *type,
   int oflags, int mode, int options)
{
  int res;
  if (tar_init(t, pathname, type, oflags, mode, options) == -1)
    return -1;

  if ((options & TAR_NOOVERWRITE) && (oflags & O_CREAT))
    oflags |= O_EXCL;

#ifdef O_BINARY
  oflags |= O_BINARY;
#endif

  res = (*((*t)->type->openfunc))((*t)->type->call_data, pathname, oflags, mode);
  if (res == -1)
  {
    free(*t);
    return -1;
  }

  return 0;
}


int
tar_fdopen(TAR **t, int fd, char *pathname, tartype_t *type,
     int oflags, int mode, int options)
{
  if (tar_init(t, pathname, type, oflags, mode, options) == -1)
    return -1;

  if ( !type )
    {
    struct libtar_fd_file* lf = (struct libtar_fd_file*)((*t)->type->call_data);
    lf->fd = fd;
    }
  return 0;
}

/* close tarfile handle */
int
tar_close(TAR *t)
{
  int i;

  i = (*(t->type->closefunc))(t->type->call_data);

  if (t->h != NULL)
    libtar_hash_free(t->h, ((t->oflags & O_ACCMODE) == O_RDONLY
          ? free
          : (libtar_freefunc_t)tar_dev_free));
  free(t);

  return i;
}


