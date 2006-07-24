/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  block.c - libtar code to handle tar archive header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtarint/internal.h>

#include <errno.h>

#ifdef STDC_HEADERS
# include <string.h>
# include <stdlib.h>
#endif


#define BIT_ISSET(bitmask, bit) ((bitmask) & (bit))


/* read a header block */
static int
th_read_internal(TAR *t)
{
  ssize_t i;
  int num_zero_blocks = 0;

#ifdef DEBUG
  printf("==> th_read_internal(TAR=\"%s\")\n", t->pathname);
#endif

  while ((i = tar_block_read(t, &(t->th_buf))) == T_BLOCKSIZE)
  {
    /* two all-zero blocks mark EOF */
    if (t->th_buf.name[0] == '\0')
    {
      num_zero_blocks++;
      if (!BIT_ISSET(t->options, TAR_IGNORE_EOT)
          && num_zero_blocks >= 2)
        return 0;  /* EOF */
      else
        continue;
    }

    /* verify magic and version */
    if (BIT_ISSET(t->options, TAR_CHECK_MAGIC)
        && strncmp(t->th_buf.magic, TMAGIC, TMAGLEN - 1) != 0)
    {
#ifdef DEBUG
      puts("!!! unknown magic value in tar header");
#endif
      return -2;
    }

    if (BIT_ISSET(t->options, TAR_CHECK_VERSION)
        && strncmp(t->th_buf.version, TVERSION, TVERSLEN) != 0)
    {
#ifdef DEBUG
      puts("!!! unknown version value in tar header");
#endif
      return -2;
    }

    /* check chksum */
    if (!BIT_ISSET(t->options, TAR_IGNORE_CRC)
        && !th_crc_ok(t))
    {
#ifdef DEBUG
      puts("!!! tar header checksum error");
#endif
      return -2;
    }

    break;
  }

#ifdef DEBUG
  printf("<== th_read_internal(): returning %d\n", i);
#endif
  return (int)i;
}


/* wrapper function for th_read_internal() to handle GNU extensions */
int
th_read(TAR *t)
{
  ssize_t i, j;
  size_t sz;
  char *ptr;

#ifdef DEBUG
  printf("==> th_read(t=0x%lx)\n", t);
#endif

  if (t->th_buf.gnu_longname != NULL)
    free(t->th_buf.gnu_longname);
  if (t->th_buf.gnu_longlink != NULL)
    free(t->th_buf.gnu_longlink);
  memset(&(t->th_buf), 0, sizeof(struct tar_header));

  i = th_read_internal(t);
  if (i == 0)
    return 1;
  else if (i != T_BLOCKSIZE)
  {
    if (i != -1)
      errno = EINVAL;
    return -1;
  }

  /* check for GNU long link extention */
  if (TH_ISLONGLINK(t))
  {
    sz = th_get_size(t);
    j = (sz / T_BLOCKSIZE) + (sz % T_BLOCKSIZE ? 1 : 0);
#ifdef DEBUG
    printf("    th_read(): GNU long linkname detected "
           "(%ld bytes, %d blocks)\n", sz, j);
#endif
    t->th_buf.gnu_longlink = (char *)malloc(j * T_BLOCKSIZE);
    if (t->th_buf.gnu_longlink == NULL)
      return -1;

    for (ptr = t->th_buf.gnu_longlink; j > 0;
         j--, ptr += T_BLOCKSIZE)
    {
#ifdef DEBUG
      printf("    th_read(): reading long linkname "
             "(%d blocks left, ptr == %ld)\n", j, ptr);
#endif
      i = tar_block_read(t, ptr);
      if (i != T_BLOCKSIZE)
      {
        if (i != -1)
          errno = EINVAL;
        return -1;
      }
#ifdef DEBUG
      printf("    th_read(): read block == \"%s\"\n", ptr);
#endif
    }
#ifdef DEBUG
    printf("    th_read(): t->th_buf.gnu_longlink == \"%s\"\n",
           t->th_buf.gnu_longlink);
#endif

    i = th_read_internal(t);
    if (i != T_BLOCKSIZE)
    {
      if (i != -1)
        errno = EINVAL;
      return -1;
    }
  }

  /* check for GNU long name extention */
  if (TH_ISLONGNAME(t))
  {
    sz = th_get_size(t);
    j = (sz / T_BLOCKSIZE) + (sz % T_BLOCKSIZE ? 1 : 0);
#ifdef DEBUG
    printf("    th_read(): GNU long filename detected "
           "(%ld bytes, %d blocks)\n", sz, j);
#endif
    t->th_buf.gnu_longname = (char *)malloc(j * T_BLOCKSIZE);
    if (t->th_buf.gnu_longname == NULL)
      return -1;

    for (ptr = t->th_buf.gnu_longname; j > 0;
         j--, ptr += T_BLOCKSIZE)
    {
#ifdef DEBUG
      printf("    th_read(): reading long filename "
             "(%d blocks left, ptr == %ld)\n", j, ptr);
#endif
      i = tar_block_read(t, ptr);
      if (i != T_BLOCKSIZE)
      {
        if (i != -1)
          errno = EINVAL;
        return -1;
      }
#ifdef DEBUG
      printf("    th_read(): read block == \"%s\"\n", ptr);
#endif
    }
#ifdef DEBUG
    printf("    th_read(): t->th_buf.gnu_longname == \"%s\"\n",
           t->th_buf.gnu_longname);
#endif

    i = th_read_internal(t);
    if (i != T_BLOCKSIZE)
    {
      if (i != -1)
        errno = EINVAL;
      return -1;
    }
  }

#if 0
  /*
  ** work-around for old archive files with broken typeflag fields
  ** NOTE: I fixed this in the TH_IS*() macros instead
  */

  /*
  ** (directories are signified with a trailing '/')
  */
  if (t->th_buf.typeflag == AREGTYPE
      && t->th_buf.name[strlen(t->th_buf.name) - 1] == '/')
    t->th_buf.typeflag = DIRTYPE;

  /*
  ** fallback to using mode bits
  */
  if (t->th_buf.typeflag == AREGTYPE)
  {
    mode = (mode_t)oct_to_int(t->th_buf.mode);

    if (S_ISREG(mode))
      t->th_buf.typeflag = REGTYPE;
    else if (S_ISDIR(mode))
      t->th_buf.typeflag = DIRTYPE;
    else if (S_ISFIFO(mode))
      t->th_buf.typeflag = FIFOTYPE;
    else if (S_ISCHR(mode))
      t->th_buf.typeflag = CHRTYPE;
    else if (S_ISBLK(mode))
      t->th_buf.typeflag = BLKTYPE;
    else if (S_ISLNK(mode))
      t->th_buf.typeflag = SYMTYPE;
  }
#endif

  return 0;
}


/* write a header block */
int
th_write(TAR *t)
{
  ssize_t i, j;
  char type2;
  size_t sz, sz2;
  char *ptr;
  char buf[T_BLOCKSIZE];

#ifdef DEBUG
  printf("==> th_write(TAR=\"%s\")\n", t->pathname);
  th_print(t);
#endif

  if ((t->options & TAR_GNU) && t->th_buf.gnu_longlink != NULL)
  {
#ifdef DEBUG
    printf("th_write(): using gnu_longlink (\"%s\")\n",
           t->th_buf.gnu_longlink);
#endif
    /* save old size and type */
    type2 = t->th_buf.typeflag;
    sz2 = th_get_size(t);

    /* write out initial header block with fake size and type */
    t->th_buf.typeflag = GNU_LONGLINK_TYPE;
    sz = strlen(t->th_buf.gnu_longlink);
    th_set_size(t, sz);
    th_finish(t);
    i = tar_block_write(t, &(t->th_buf));
    if (i != T_BLOCKSIZE)
    {
      if (i != -1)
        errno = EINVAL;
      return -1;
    }

    /* write out extra blocks containing long name */
    for (j = (sz / T_BLOCKSIZE) + (sz % T_BLOCKSIZE ? 1 : 0),
         ptr = t->th_buf.gnu_longlink; j > 1;
         j--, ptr += T_BLOCKSIZE)
    {
      i = tar_block_write(t, ptr);
      if (i != T_BLOCKSIZE)
      {
        if (i != -1)
          errno = EINVAL;
        return -1;
      }
    }
    memset(buf, 0, T_BLOCKSIZE);
    strncpy(buf, ptr, T_BLOCKSIZE);
    i = tar_block_write(t, &buf);
    if (i != T_BLOCKSIZE)
    {
      if (i != -1)
        errno = EINVAL;
      return -1;
    }

    /* reset type and size to original values */
    t->th_buf.typeflag = type2;
    th_set_size(t, sz2);
  }

  if ((t->options & TAR_GNU) && t->th_buf.gnu_longname != NULL)
  {
#ifdef DEBUG
    printf("th_write(): using gnu_longname (\"%s\")\n",
           t->th_buf.gnu_longname);
#endif
    /* save old size and type */
    type2 = t->th_buf.typeflag;
    sz2 = th_get_size(t);

    /* write out initial header block with fake size and type */
    t->th_buf.typeflag = GNU_LONGNAME_TYPE;
    sz = strlen(t->th_buf.gnu_longname);
    th_set_size(t, sz);
    th_finish(t);
    i = tar_block_write(t, &(t->th_buf));
    if (i != T_BLOCKSIZE)
    {
      if (i != -1)
        errno = EINVAL;
      return -1;
    }

    /* write out extra blocks containing long name */
    for (j = (sz / T_BLOCKSIZE) + (sz % T_BLOCKSIZE ? 1 : 0),
         ptr = t->th_buf.gnu_longname; j > 1;
         j--, ptr += T_BLOCKSIZE)
    {
      i = tar_block_write(t, ptr);
      if (i != T_BLOCKSIZE)
      {
        if (i != -1)
          errno = EINVAL;
        return -1;
      }
    }
    memset(buf, 0, T_BLOCKSIZE);
    strncpy(buf, ptr, T_BLOCKSIZE);
    i = tar_block_write(t, &buf);
    if (i != T_BLOCKSIZE)
    {
      if (i != -1)
        errno = EINVAL;
      return -1;
    }

    /* reset type and size to original values */
    t->th_buf.typeflag = type2;
    th_set_size(t, sz2);
  }

  th_finish(t);

#ifdef DEBUG
  /* print tar header */
  th_print(t);
#endif

  i = tar_block_write(t, &(t->th_buf));
  if (i != T_BLOCKSIZE)
  {
    if (i != -1)
      errno = EINVAL;
    return -1;
  }

#ifdef DEBUG
  puts("th_write(): returning 0");
#endif
  return 0;
}


