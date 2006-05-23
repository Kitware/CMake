/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  append.c - libtar code to append files to a tar archive
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtarint/internal.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <libtar/compat.h>
#else
# include <sys/param.h>
#endif
#include <libtar/compat.h>
#include <sys/types.h>

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_IO_H
# include <io.h>
#endif

struct tar_dev
{
  dev_t td_dev;
  libtar_hash_t *td_h;
};
typedef struct tar_dev tar_dev_t;

struct tar_ino
{
  ino_t ti_ino;
  char ti_name[TAR_MAXPATHLEN];
};
typedef struct tar_ino tar_ino_t;


/* free memory associated with a tar_dev_t */
void
tar_dev_free(tar_dev_t *tdp)
{
  libtar_hash_free(tdp->td_h, free);
  free(tdp);
}


/* appends a file to the tar archive */
int
tar_append_file(TAR *t, char *realname, char *savename)
{
  struct stat s;
  libtar_hashptr_t hp;
  tar_dev_t *td;
  tar_ino_t *ti;
#if !defined(_WIN32) || defined(__CYGWIN__)
  int i;
#else
  size_t plen;
#endif
  char path[TAR_MAXPATHLEN];

#ifdef DEBUG
  printf("==> tar_append_file(TAR=0x%lx (\"%s\"), realname=\"%s\", "
         "savename=\"%s\")\n", t, t->pathname, realname,
         (savename ? savename : "[NULL]"));
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
  strncpy(path, realname, sizeof(path)-1);
  path[sizeof(path)-1] = 0;
  plen = strlen(path);
  if (path[plen-1] == '/' )
    {
    path[plen-1] = 0;
    }
  if (stat(path, &s) != 0)
#else
  if (lstat(realname, &s) != 0)
#endif
  {
#ifdef DEBUG
    perror("lstat()");
#endif
    return -1;
  }

  /* set header block */
#ifdef DEBUG
  puts("    tar_append_file(): setting header block...");
#endif
  memset(&(t->th_buf), 0, sizeof(struct tar_header));
  th_set_from_stat(t, &s);

  /* set the header path */
#ifdef DEBUG
  puts("    tar_append_file(): setting header path...");
#endif
  th_set_path(t, (savename ? savename : realname));

  /* check if it's a hardlink */
#ifdef DEBUG
  puts("    tar_append_file(): checking inode cache for hardlink...");
#endif
  libtar_hashptr_reset(&hp);
  if (libtar_hash_getkey(t->h, &hp, &(s.st_dev),
             (libtar_matchfunc_t)dev_match) != 0)
    td = (tar_dev_t *)libtar_hashptr_data(&hp);
  else
  {
#ifdef DEBUG
    printf("+++ adding hash for device (0x%lx, 0x%lx)...\n",
           major(s.st_dev), minor(s.st_dev));
#endif
    td = (tar_dev_t *)calloc(1, sizeof(tar_dev_t));
    td->td_dev = s.st_dev;
    td->td_h = libtar_hash_new(256, (libtar_hashfunc_t)ino_hash);
    if (td->td_h == NULL)
      return -1;
    if (libtar_hash_add(t->h, td) == -1)
      return -1;
  }
  libtar_hashptr_reset(&hp);
#if !defined(_WIN32) || defined(__CYGWIN__)
  if (libtar_hash_getkey(td->td_h, &hp, &(s.st_ino),
             (libtar_matchfunc_t)ino_match) != 0)
  {
    ti = (tar_ino_t *)libtar_hashptr_data(&hp);
#ifdef DEBUG
    printf("    tar_append_file(): encoding hard link \"%s\" "
           "to \"%s\"...\n", realname, ti->ti_name);
#endif
    t->th_buf.typeflag = LNKTYPE;
    th_set_link(t, ti->ti_name);
  }
  else
#endif
  {
#ifdef DEBUG
    printf("+++ adding entry: device (0x%lx,0x%lx), inode %ld "
           "(\"%s\")...\n", major(s.st_dev), minor(s.st_dev),
           s.st_ino, realname);
#endif
    ti = (tar_ino_t *)calloc(1, sizeof(tar_ino_t));
    if (ti == NULL)
      return -1;
    ti->ti_ino = s.st_ino;
    snprintf(ti->ti_name, sizeof(ti->ti_name), "%s",
       savename ? savename : realname);
    libtar_hash_add(td->td_h, ti);
  }

#if !defined(_WIN32) || defined(__CYGWIN__)
  /* check if it's a symlink */
  if (TH_ISSYM(t))
  {
#if defined(_WIN32) && !defined(__CYGWIN__)
    i = -1;
#else
    i = readlink(realname, path, sizeof(path));
#endif
    if (i == -1)
      return -1;
    if (i >= TAR_MAXPATHLEN)
      i = TAR_MAXPATHLEN - 1;
    path[i] = '\0';
#ifdef DEBUG
    printf("    tar_append_file(): encoding symlink \"%s\" -> "
           "\"%s\"...\n", realname, path);
#endif
    th_set_link(t, path);
  }
#endif

  /* print file info */
  if (t->options & TAR_VERBOSE)
    th_print_long_ls(t);

#ifdef DEBUG
  puts("    tar_append_file(): writing header");
#endif
  /* write header */
  if (th_write(t) != 0)
  {
#ifdef DEBUG
    printf("t->fd = %d\n", t->fd);
#endif
    return -1;
  }
#ifdef DEBUG
  puts("    tar_append_file(): back from th_write()");
#endif

  /* if it's a regular file, write the contents as well */
  if (TH_ISREG(t) && tar_append_regfile(t, realname) != 0)
    return -1;

  return 0;
}


/* write EOF indicator */
int
tar_append_eof(TAR *t)
{
  ssize_t i, j;
  char block[T_BLOCKSIZE];

  memset(&block, 0, T_BLOCKSIZE);
  for (j = 0; j < 2; j++)
  {
    i = tar_block_write(t, &block);
    if (i != T_BLOCKSIZE)
    {
      if (i != -1)
        errno = EINVAL;
      return -1;
    }
  }

  return 0;
}


/* add file contents to a tarchive */
int
tar_append_regfile(TAR *t, char *realname)
{
  char block[T_BLOCKSIZE];
  int filefd;
  ssize_t i, j;
  size_t size;

#if defined( _WIN32 ) || defined(__CYGWIN__)
  filefd = open(realname, O_RDONLY | O_BINARY);
#else
  filefd = open(realname, O_RDONLY);
#endif
  if (filefd == -1)
  {
#ifdef DEBUG
    perror("open()");
#endif
    return -1;
  }

  size = th_get_size(t);
  for (i = size; i > T_BLOCKSIZE; i -= T_BLOCKSIZE)
  {
    j = read(filefd, &block, T_BLOCKSIZE);
    if (j != T_BLOCKSIZE)
    {
      if (j != -1)
        {
        fprintf(stderr, "Unexpected size of read data: %d <> %d for file: %s\n",
          (int)j, T_BLOCKSIZE, realname);
        errno = EINVAL;
        }
      return -1;
    }
    if (tar_block_write(t, &block) == -1)
      return -1;
  }

  if (i > 0)
  {
    j = (size_t)read(filefd, &block, (unsigned int)i);
    if (j == -1)
      return -1;
    memset(&(block[i]), 0, T_BLOCKSIZE - i);
    if (tar_block_write(t, &block) == -1)
      return -1;
  }

  close(filefd);

  return 0;
}


