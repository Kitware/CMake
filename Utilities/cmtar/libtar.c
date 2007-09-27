/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  libtar.c - demo driver program for libtar
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/
#include <libtar/config.h>
#include <libtar/libtar.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <libtar/compat.h>
#include <io.h>
#else
#include <sys/param.h>
#endif

#ifdef STDC_HEADERS
# include <string.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
# include <stdlib.h>
#endif

#ifdef DEBUG
# include <signal.h>
#endif

#include CMTAR_ZLIB_HEADER

#include <libtar/compat.h>


char *progname;
int verbose = 0;
int use_gnu = 0;

#ifdef DEBUG
void
segv_handler(int sig)
{
  puts("OOPS!  Caught SIGSEGV, bailing out...");
  fflush(stdout);
  fflush(stderr);
}
#endif


#ifdef HAVE_LIBZ

int use_zlib = 0;

struct gzStruct
{
  gzFile* GZFile;
};
struct gzStruct GZStruct;
#if defined ( _MSC_VER) || defined(__WATCOMC__)
#include <io.h>
//Yogi: hack. this should work on windows where there is no O_ACCMODE defined
#ifndef O_ACCMODE
# define O_ACCMODE 0x0003
#endif
#endif

static int libtar_gzopen(void* call_data, const char *pathname,
  int oflags, mode_t mode)
{
  char *gzoflags;
  int fd;
  struct gzStruct* gzf = (struct gzStruct*)call_data;

  switch (oflags & O_ACCMODE)
  {
  case O_WRONLY:
    gzoflags = "wb";
    break;
  case O_RDONLY:
    gzoflags = "rb";
    break;
  default:
  case O_RDWR:
    errno = EINVAL;
    return -1;
  }

  fd = open(pathname, oflags, mode);
  if (fd == -1)
    {
    return -1;
    }

#if defined(__BEOS__) && !defined(__ZETA__)  /* no fchmod on BeOS...do pathname instead. */
  if ((oflags & O_CREAT) && chmod(pathname, mode & 07777))
    {
    return -1;
    }
#elif !defined(_WIN32) || defined(__CYGWIN__)
  if ((oflags & O_CREAT) && fchmod(fd, mode & 07777))
    {
    return -1;
    }
#endif

  gzf->GZFile = gzdopen(fd, gzoflags);
  if (!gzf->GZFile)
  {
    errno = ENOMEM;
    return -1;
  }

  return fd;
}

static int libtar_gzclose(void* call_data)
{
  struct gzStruct* gzf = (struct gzStruct*)call_data;
  return gzclose(gzf->GZFile);
}

static ssize_t libtar_gzread(void* call_data, void* buf, size_t count)
{
  struct gzStruct* gzf = (struct gzStruct*)call_data;
  return gzread(gzf->GZFile, buf, (unsigned int)count);
}

static ssize_t libtar_gzwrite(void* call_data, const void* buf, size_t count)
{
  struct gzStruct* gzf = (struct gzStruct*)call_data;
  return gzwrite(gzf->GZFile, (void*)buf, (unsigned int)count);
}

tartype_t gztype = { 
  libtar_gzopen,
  libtar_gzclose,
  libtar_gzread,
  libtar_gzwrite,
  &GZStruct
};

#endif /* HAVE_LIBZ */


static int
create(char *tarfile, char *rootdir, libtar_list_t *l)
{
  TAR *t;
  char *pathname;
  char buf[TAR_MAXPATHLEN];
  libtar_listptr_t lp;

  if (tar_open(&t, tarfile,
#ifdef HAVE_LIBZ
         (use_zlib ? &gztype : NULL),
#else
         NULL,
#endif
         O_WRONLY | O_CREAT, 0644,
         (verbose ? TAR_VERBOSE : 0)
         | (use_gnu ? TAR_GNU : 0)) == -1)
  {
    fprintf(stderr, "tar_open(): %s\n", strerror(errno));
    return -1;
  }

  libtar_listptr_reset(&lp);
  while (libtar_list_next(l, &lp) != 0)
  {
    pathname = (char *)libtar_listptr_data(&lp);
    if (pathname[0] != '/' && rootdir != NULL)
      snprintf(buf, sizeof(buf), "%s/%s", rootdir, pathname);
    else
      strlcpy(buf, pathname, sizeof(buf));
    if (tar_append_tree(t, buf, pathname) != 0)
    {
      fprintf(stderr,
        "tar_append_tree(\"%s\", \"%s\"): %s\n", buf,
        pathname, strerror(errno));
      tar_close(t);
      return -1;
    }
  }

  if (tar_append_eof(t) != 0)
  {
    fprintf(stderr, "tar_append_eof(): %s\n", strerror(errno));
    tar_close(t);
    return -1;
  }

  if (tar_close(t) != 0)
  {
    fprintf(stderr, "tar_close(): %s\n", strerror(errno));
    return -1;
  }

  return 0;
}


static int
list(char *tarfile)
{
  TAR *t;
  int i;

  if (tar_open(&t, tarfile,
#ifdef HAVE_LIBZ
         (use_zlib ? &gztype : NULL),
#else
         NULL,
#endif
         O_RDONLY, 0,
         (verbose ? TAR_VERBOSE : 0)
         | (use_gnu ? TAR_GNU : 0)) == -1)
  {
    fprintf(stderr, "tar_open(): %s\n", strerror(errno));
    return -1;
  }

  while ((i = th_read(t)) == 0)
  {
    th_print_long_ls(t);
#ifdef DEBUG
    th_print(t);
#endif
    if (TH_ISREG(t) && tar_skip_regfile(t) != 0)
    {
      fprintf(stderr, "tar_skip_regfile(): %s\n",
        strerror(errno));
      return -1;
    }
  }

#ifdef DEBUG
  printf("th_read() returned %d\n", i);
  printf("EOF mark encountered after %ld bytes\n",
# ifdef HAVE_LIBZ
         (use_zlib
    ? gzseek((gzFile) t->fd, 0, SEEK_CUR)
    :
# endif
         lseek(t->fd, 0, SEEK_CUR)
# ifdef HAVE_LIBZ
         )
# endif
         );
#endif

  if (tar_close(t) != 0)
  {
    fprintf(stderr, "tar_close(): %s\n", strerror(errno));
    return -1;
  }
  (void)i;

  return 0;
}


static int
extract(char *tarfile, char *rootdir)
{
  TAR *t;

#ifdef DEBUG
  puts("opening tarfile...");
#endif
  if (tar_open(&t, tarfile,
#ifdef HAVE_LIBZ
         (use_zlib ? &gztype : NULL),
#else
         NULL,
#endif
         O_RDONLY, 0,
         (verbose ? TAR_VERBOSE : 0)
         | (use_gnu ? TAR_GNU : 0)) == -1)
  {
    fprintf(stderr, "tar_open(): %s\n", strerror(errno));
    return -1;
  }

#ifdef DEBUG
  puts("extracting tarfile...");
#endif
  if (tar_extract_all(t, rootdir) != 0)
  {
    fprintf(stderr, "tar_extract_all(): %s\n", strerror(errno));
    return -1;
  }

#ifdef DEBUG
  puts("closing tarfile...");
#endif
  if (tar_close(t) != 0)
  {
    fprintf(stderr, "tar_close(): %s\n", strerror(errno));
    return -1;
  }

  return 0;
}


static void
usage()
{
  printf("Usage: %s [-C rootdir] [-g] [-z] -x|-t filename.tar\n",
         progname);
  printf("       %s [-C rootdir] [-g] [-z] -c filename.tar ...\n",
         progname);
  exit(-1);
}


#define MODE_LIST  1
#define MODE_CREATE  2
#define MODE_EXTRACT  3

int
main(int argc, char *argv[])
{
  char* tarfile;
  char *rootdir = NULL;
  int c;
  int mode;
  libtar_list_t *l;
#if defined(_WIN32) && !defined(__CYGWIN__)
   int optind;
#endif
  progname = basename(argv[0]);

#if !defined(_WIN32) || defined(__CYGWIN__)
  mode = 0;
  while ((c = getopt(argc, argv, "cC:gtvVxz")) != -1)
    switch (c)
    {
    case 'V':
      printf("libtar %s by Mark D. Roth <roth@uiuc.edu>\n",
             libtar_version);
      break;
    case 'C':
      rootdir = strdup(optarg);
      break;
    case 'v':
      verbose = 1;
      break;
    case 'g':
      use_gnu = 1;
      break;
    case 'c':
      if (mode)
        usage();
      mode = MODE_CREATE;
      break;
    case 'x':
      if (mode)
        usage();
      mode = MODE_EXTRACT;
      break;
    case 't':
      if (mode)
        usage();
      mode = MODE_LIST;
      break;
#ifdef HAVE_LIBZ
    case 'z':
      use_zlib = 1;
      break;
#endif /* HAVE_LIBZ */
    default:
      usage();
    }
  if (!mode || ((argc - optind) < (mode == MODE_CREATE ? 2 : 1)))
  {
#ifdef DEBUG
    printf("argc - optind == %d\tmode == %d\n", argc - optind,
           mode);
#endif
    usage();
  }

#else
  mode = MODE_EXTRACT;
  use_zlib=1;
  optind = 1;
#endif

#ifdef DEBUG
  signal(SIGSEGV, segv_handler);
#endif

  switch (mode)
  {
  case MODE_EXTRACT:
    return extract(argv[optind], rootdir);
  case MODE_CREATE:
    tarfile = argv[optind];
    l = libtar_list_new(LIST_QUEUE, NULL);
    for (c = optind + 1; c < argc; c++)
      libtar_list_add(l, argv[c]);
    return create(tarfile, rootdir, l);
  case MODE_LIST:
    return list(argv[optind]);
  default:
    break;
  }

  /* NOTREACHED */
  return -2;
}


