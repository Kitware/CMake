/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmCPackTGZGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"

#include <cmsys/SystemTools.hxx>
#include <cmzlib/zlib.h>

#include <memory> // auto_ptr
#include <fcntl.h> // O_WRONLY, O_CREAT

//----------------------------------------------------------------------
cmCPackTGZGenerator::cmCPackTGZGenerator()
{
}

//----------------------------------------------------------------------
cmCPackTGZGenerator::~cmCPackTGZGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::ProcessGenerator()
{
  return this->Superclass::ProcessGenerator();
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::Initialize(const char* name)
{
  return this->Superclass::Initialize(name);
}

//----------------------------------------------------------------------
// The following code is modified version of bsdtar
/*-
* Copyright (c) 2003-2004 Tim Kientzle
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer
*    in this position and unchanged.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  */

#if defined(HAVE_IO_H)
#  include <io.h>
#endif
#if defined(HAVE_UNISTD_H)
#  include <unistd.h> // for geteuid
#endif
#if defined(HAVE_INTTYPES_H)
#  include <inttypes.h> // for uintptr_t
#endif
#include <sys/types.h>
#include <errno.h> // for ENOMEM
#include <sys/stat.h> // for struct stat
#include <time.h> // for time

#if defined(WIN32) && !defined(__CYGWIN__)
#  ifndef S_ISREG
#    define S_ISREG(x) ((x) & _S_IFREG)
#  endif
#  ifndef S_ISLNK
#    define S_ISLNK(x) (0)
#  endif
#endif

  //--- archive_plarform.h
  //
  /* Set up defaults for internal error codes. */
#ifndef ARCHIVE_ERRNO_FILE_FORMAT
#if HAVE_EFTYPE
#define ARCHIVE_ERRNO_FILE_FORMAT EFTYPE
#else
#if HAVE_EILSEQ
#define ARCHIVE_ERRNO_FILE_FORMAT EILSEQ
#else
#define ARCHIVE_ERRNO_FILE_FORMAT EINVAL
#endif
#endif
#endif

#ifndef ARCHIVE_ERRNO_PROGRAMMER
#define ARCHIVE_ERRNO_PROGRAMMER EINVAL
#endif

#ifndef ARCHIVE_ERRNO_MISC
#define ARCHIVE_ERRNO_MISC (-1)
#endif

  /* Select the best way to set/get hi-res timestamps. */
#if HAVE_STRUCT_STAT_ST_MTIMESPEC_TV_NSEC
  /* FreeBSD uses "timespec" members. */
#define ARCHIVE_STAT_ATIME_NANOS(st) (st)->st_atimespec.tv_nsec
#define ARCHIVE_STAT_CTIME_NANOS(st) (st)->st_ctimespec.tv_nsec
#define ARCHIVE_STAT_MTIME_NANOS(st) (st)->st_mtimespec.tv_nsec
#define ARCHIVE_STAT_SET_ATIME_NANOS(st, n) (st)->st_atimespec.tv_nsec = (n)
#define ARCHIVE_STAT_SET_CTIME_NANOS(st, n) (st)->st_ctimespec.tv_nsec = (n)
#define ARCHIVE_STAT_SET_MTIME_NANOS(st, n) (st)->st_mtimespec.tv_nsec = (n)
#else
#if HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC
  /* Linux uses "tim" members. */
#define ARCHIVE_STAT_ATIME_NANOS(pstat) (pstat)->st_atim.tv_nsec
#define ARCHIVE_STAT_CTIME_NANOS(pstat) (pstat)->st_ctim.tv_nsec
#define ARCHIVE_STAT_MTIME_NANOS(pstat) (pstat)->st_mtim.tv_nsec
#define ARCHIVE_STAT_SET_ATIME_NANOS(st, n) (st)->st_atim.tv_nsec = (n)
#define ARCHIVE_STAT_SET_CTIME_NANOS(st, n) (st)->st_ctim.tv_nsec = (n)
#define ARCHIVE_STAT_SET_MTIME_NANOS(st, n) (st)->st_mtim.tv_nsec = (n)
#else
  /* If we can't find a better way, just use stubs. */
#define ARCHIVE_STAT_ATIME_NANOS(pstat) 0
#define ARCHIVE_STAT_CTIME_NANOS(pstat) 0
#define ARCHIVE_STAT_MTIME_NANOS(pstat) 0
#define ARCHIVE_STAT_SET_ATIME_NANOS(st, n)
#define ARCHIVE_STAT_SET_CTIME_NANOS(st, n)
#define ARCHIVE_STAT_SET_MTIME_NANOS(st, n)
#endif
#endif

  //--- archive.h

#define ARCHIVE_BYTES_PER_RECORD   512
#define ARCHIVE_DEFAULT_BYTES_PER_BLOCK 10240

  /* Declare our basic types. */
  struct archive;
  struct archive_entry;

  /*
   * Error codes: Use archive_errno() and archive_error_string()
   * to retrieve details.  Unless specified otherwise, all functions
   * that return 'int' use these codes.
   */
#define ARCHIVE_EOF   1 /* Found end of archive. */
#define ARCHIVE_OK   0 /* Operation was successful. */
#define ARCHIVE_RETRY (-10) /* Retry might succeed. */
#define ARCHIVE_WARN (-20) /* Partial sucess. */
#define ARCHIVE_FATAL (-30) /* No more operations are possible. */

  /*
   * As far as possible, archive_errno returns standard platform errno codes.
   * Of course, the details vary by platform, so the actual definitions
   * here are stored in "archive_platform.h".  The symbols are listed here
   * for reference; as a rule, clients should not need to know the exact
   * platform-dependent error code.
   */
  /* Unrecognized or invalid file format. */
  /* #define ARCHIVE_ERRNO_FILE_FORMAT */
  /* Illegal usage of the library. */
  /* #define ARCHIVE_ERRNO_PROGRAMMER_ERROR */
  /* Unknown or unclassified error. */
  /* #define ARCHIVE_ERRNO_MISC */

  /*
   * Callbacks are invoked to automatically read/write/open/close the archive.
   * You can provide your own for complex tasks (like breaking archives
   * across multiple tapes) or use standard ones built into the library.
   */

  /* Returns pointer and size of next block of data from archive. */
  typedef ssize_t archive_read_callback(struct archive *, void *_client_data,
    const void **_buffer);
/* Returns size actually written, zero on EOF, -1 on error. */
typedef ssize_t archive_write_callback(struct archive *, void *_client_data,
  void *_buffer, size_t _length);
typedef int archive_open_callback(struct archive *, void *_client_data);
typedef int archive_close_callback(struct archive *, void *_client_data);

/*
 * Codes for archive_compression.
 */
#define ARCHIVE_COMPRESSION_NONE 0
#define ARCHIVE_COMPRESSION_GZIP 1
#define ARCHIVE_COMPRESSION_BZIP2 2
#define ARCHIVE_COMPRESSION_COMPRESS 3

/*
 * Codes returned by archive_format.
 *
 * Top 16 bits identifies the format family (e.g., "tar"); lower
 * 16 bits indicate the variant.  This is updated by read_next_header.
 * Note that the lower 16 bits will often vary from entry to entry.
 */
#define ARCHIVE_FORMAT_BASE_MASK  0xff0000U
#define ARCHIVE_FORMAT_CPIO   0x10000
#define ARCHIVE_FORMAT_CPIO_POSIX  (ARCHIVE_FORMAT_CPIO | 1)
#define ARCHIVE_FORMAT_SHAR   0x20000
#define ARCHIVE_FORMAT_SHAR_BASE  (ARCHIVE_FORMAT_SHAR | 1)
#define ARCHIVE_FORMAT_SHAR_DUMP  (ARCHIVE_FORMAT_SHAR | 2)
#define ARCHIVE_FORMAT_TAR   0x30000
#define ARCHIVE_FORMAT_TAR_USTAR  (ARCHIVE_FORMAT_TAR | 1)
#define ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE (ARCHIVE_FORMAT_TAR | 2)
#define ARCHIVE_FORMAT_TAR_PAX_RESTRICTED (ARCHIVE_FORMAT_TAR | 3)
#define ARCHIVE_FORMAT_TAR_GNUTAR  (ARCHIVE_FORMAT_TAR | 4)
#define ARCHIVE_FORMAT_ISO9660   0x40000
#define ARCHIVE_FORMAT_ISO9660_ROCKRIDGE (ARCHIVE_FORMAT_ISO9660 | 1)
#define ARCHIVE_FORMAT_ZIP   0x50000

/*-
 * Basic outline for reading an archive:
 *   1) Ask archive_read_new for an archive reader object.
 *   2) Update any global properties as appropriate.
 *      In particular, you'll certainly want to call appropriate
 *      archive_read_support_XXX functions.
 *   3) Call archive_read_open_XXX to open the archive
 *   4) Repeatedly call archive_read_next_header to get information about
 *      successive archive entries.  Call archive_read_data to extract
 *      data for entries of interest.
 *   5) Call archive_read_finish to end processing.
 */
struct archive *archive_read_new(void);

/*
 * The archive_read_support_XXX calls enable auto-detect for this
 * archive handle.  They also link in the necessary support code.
 * For example, if you don't want bzlib linked in, don't invoke
 * support_compression_bzip2().  The "all" functions provide the
 * obvious shorthand.
 */
int   archive_read_support_compression_all(struct archive *);
int   archive_read_support_compression_bzip2(struct archive *);
int   archive_read_support_compression_compress(struct archive *);
int   archive_read_support_compression_gzip(struct archive *);
int   archive_read_support_compression_none(struct archive *);

int   archive_read_support_format_all(struct archive *);
int   archive_read_support_format_cpio(struct archive *);
int   archive_read_support_format_gnutar(struct archive *);
int   archive_read_support_format_iso9660(struct archive *);
int   archive_read_support_format_tar(struct archive *);
int   archive_read_support_format_tp(struct archive *a);
int   archive_read_support_format_zip(struct archive *);


/* Open the archive using callbacks for archive I/O. */
int   archive_read_open(struct archive *, void *_client_data,
  archive_open_callback *, archive_read_callback *,
  archive_close_callback *);

/*
 * The archive_read_open_file function is a convenience function built
 * on archive_read_open that uses a canned callback suitable for
 * common situations.  Note that a NULL filename indicates stdin.
 */
int   archive_read_open_file(struct archive *, const char *_file,
  size_t _block_size);
int   archive_read_open_fd(struct archive *, int _fd,
  size_t _block_size);

/* Parses and returns next entry header. */
int   archive_read_next_header(struct archive *,
  struct archive_entry **);

/*
 * Retrieve the byte offset in UNCOMPRESSED data where last-read
 * header started.
 */
int64_t   archive_read_header_position(struct archive *);

/* Read data from the body of an entry.  Similar to read(2). */
ssize_t   archive_read_data(struct archive *, void *, size_t);
/*
 * A zero-copy version of archive_read_data that also exposes the file offset
 * of each returned block.  Note that the client has no way to specify
 * the desired size of the block.  The API does gaurantee that offsets will
 * be strictly increasing and that returned blocks will not overlap.
 */
int   archive_read_data_block(struct archive *a,
  const void **buff, size_t *size, off_t *offset);

/*-
 * Some convenience functions that are built on archive_read_data:
 *  'skip': skips entire entry
 *  'into_buffer': writes data into memory buffer that you provide
 *  'into_fd': writes data to specified filedes
 */
int   archive_read_data_skip(struct archive *);
int   archive_read_data_into_buffer(struct archive *, void *buffer,
  ssize_t len);
int   archive_read_data_into_fd(struct archive *, int fd);

/*-
 * Convenience function to recreate the current entry (whose header
 * has just been read) on disk.
 *
 * This does quite a bit more than just copy data to disk. It also:
 *  - Creates intermediate directories as required.
 *  - Manages directory permissions:  non-writable directories will
 *    be initially created with write permission enabled; when the
 *    archive is closed, dir permissions are edited to the values specified
 *    in the archive.
 *  - Checks hardlinks:  hardlinks will not be extracted unless the
 *    linked-to file was also extracted within the same session. (TODO)
 */

/* The "flags" argument selects optional behavior, 'OR' the flags you want. */
/* TODO: The 'Default' comments here are not quite correct; clean this up. */
#define ARCHIVE_EXTRACT_OWNER (1) /* Default: owner/group not restored */
#define ARCHIVE_EXTRACT_PERM (2) /* Default: restore perm only for reg file*/
#define ARCHIVE_EXTRACT_TIME (4) /* Default: mod time not restored */
#define ARCHIVE_EXTRACT_NO_OVERWRITE (8) /* Default: Replace files on disk */
#define ARCHIVE_EXTRACT_UNLINK (16) /* Default: don't unlink existing files */
#define ARCHIVE_EXTRACT_ACL (32) /* Default: don't restore ACLs */
#define ARCHIVE_EXTRACT_FFLAGS (64) /* Default: don't restore fflags */

int   archive_read_extract(struct archive *, struct archive_entry *,
  int flags);
void   archive_read_extract_set_progress_callback(struct archive *,
  void (*_progress_func)(void *), void *_user_data);

/* Close the file and release most resources. */
int   archive_read_close(struct archive *);
/* Release all resources and destroy the object. */
/* Note that archive_read_finish will call archive_read_close for you. */
void   archive_read_finish(struct archive *);

/*-
 * To create an archive:
 *   1) Ask archive_write_new for a archive writer object.
 *   2) Set any global properties.  In particular, you should set
 *      the compression and format to use.
 *   3) Call archive_write_open to open the file (most people
 *       will use archive_write_open_file or archive_write_open_fd,
 *       which provide convenient canned I/O callbacks for you).
 *   4) For each entry:
 *      - construct an appropriate struct archive_entry structure
 *      - archive_write_header to write the header
 *      - archive_write_data to write the entry data
 *   5) archive_write_close to close the output
 *   6) archive_write_finish to cleanup the writer and release resources
 */
struct archive *archive_write_new(void);
int   archive_write_set_bytes_per_block(struct archive *,
  int bytes_per_block);
/* XXX This is badly misnamed; suggestions appreciated. XXX */
int   archive_write_set_bytes_in_last_block(struct archive *,
  int bytes_in_last_block);

int   archive_write_set_compression_bzip2(struct archive *);
int   archive_write_set_compression_gzip(struct archive *);
int   archive_write_set_compression_none(struct archive *);
/* A convenience function to set the format based on the code or name. */
int   archive_write_set_format(struct archive *, int format_code);
int   archive_write_set_format_by_name(struct archive *,
  const char *name);
/* To minimize link pollution, use one or more of the following. */
int   archive_write_set_format_cpio(struct archive *);
/* TODO: int archive_write_set_format_old_tar(struct archive *); */
int   archive_write_set_format_pax(struct archive *);
int   archive_write_set_format_pax_restricted(struct archive *);
int   archive_write_set_format_shar(struct archive *);
int   archive_write_set_format_shar_dump(struct archive *);
int   archive_write_set_format_ustar(struct archive *);
int   archive_write_open(struct archive *, void *,
  archive_open_callback *, archive_write_callback *,
  archive_close_callback *);
int   archive_write_open_fd(struct archive *, int _fd);
int   archive_write_open_file(struct archive *, const char *_file);

/*
 * Note that the library will truncate writes beyond the size provided
 * to archive_write_header or pad if the provided data is short.
 */
int   archive_write_header(struct archive *,
  struct archive_entry *);
/* TODO: should be ssize_t, but that might require .so version bump? */
int   archive_write_data(struct archive *, const void *, size_t);
int   archive_write_close(struct archive *);
void   archive_write_finish(struct archive *);

/*
 * Accessor functions to read/set various information in
 * the struct archive object:
 */
/* Bytes written after compression or read before decompression. */
int64_t   archive_position_compressed(struct archive *);
/* Bytes written to compressor or read from decompressor. */
int64_t   archive_position_uncompressed(struct archive *);

const char *archive_compression_name(struct archive *);
int   archive_compression(struct archive *);
int   archive_errno(struct archive *);
const char *archive_error_string(struct archive *);
const char *archive_format_name(struct archive *);
int   archive_format(struct archive *);
void   archive_set_error(struct archive *, int _err, const char *fmt, ...);

//--- archive_string.h
/*
 * Basic resizable/reusable string support a la Java's "StringBuffer."
 *
 * Unlike sbuf(9), the buffers here are fully reusable and track the
 * length throughout.
 *
 * Note that all visible symbols here begin with "__archive" as they
 * are internal symbols not intended for anyone outside of this library
 * to see or use.
 */

struct archive_string {
  char *s;  /* Pointer to the storage */
  size_t  length; /* Length of 's' */
  size_t  buffer_length; /* Length of malloc-ed storage */
};

/* Initialize an archive_string object on the stack or elsewhere. */
#define archive_string_init(a) \
  do { (a)->s = NULL; (a)->length = 0; (a)->buffer_length = 0; } while(0)

/* Append a C char to an archive_string, resizing as necessary. */
struct archive_string *
__archive_strappend_char(struct archive_string *, char);
#define archive_strappend_char __archive_strappend_char

/* Append a char to an archive_string using UTF8. */
struct archive_string *
__archive_strappend_char_UTF8(struct archive_string *, int);
#define archive_strappend_char_UTF8 __archive_strappend_char_UTF8

/* Append an integer in the specified base (2 <= base <= 16). */
struct archive_string *
__archive_strappend_int(struct archive_string *as, int d, int base);
#define archive_strappend_int __archive_strappend_int

/* Basic append operation. */
struct archive_string *
__archive_string_append(struct archive_string *as, const char *p, size_t s);

/* Ensure that the underlying buffer is at least as large as the request. */
struct archive_string *
__archive_string_ensure(struct archive_string *, size_t);
#define archive_string_ensure __archive_string_ensure

/* Append C string, which may lack trailing \0. */
struct archive_string *
__archive_strncat(struct archive_string *, const char *, size_t);
#define archive_strncat  __archive_strncat

/* Append a C string to an archive_string, resizing as necessary. */
#define archive_strcat(as,p) __archive_string_append((as),(p),strlen(p))

/* Copy a C string to an archive_string, resizing as necessary. */
#define archive_strcpy(as,p) \
  ((as)->length = 0, __archive_string_append((as), (p), strlen(p)))

/* Copy a C string to an archive_string with limit, resizing as necessary. */
#define archive_strncpy(as,p,l) \
  ((as)->length=0, archive_strncat((as), (p), (l)))

/* Return length of string. */
#define archive_strlen(a) ((a)->length)

/* Set string length to zero. */
#define archive_string_empty(a) ((a)->length = 0)

/* Release any allocated storage resources. */
void __archive_string_free(struct archive_string *);
#define archive_string_free  __archive_string_free

/* Like 'vsprintf', but resizes the underlying string as necessary. */
void __archive_string_vsprintf(struct archive_string *, const char *,
  va_list);
#define archive_string_vsprintf __archive_string_vsprintf
//--- archive_private.h

#define ARCHIVE_WRITE_MAGIC (0xb0c5c0deU)
#define ARCHIVE_READ_MAGIC (0xdeb0c5U)

struct archive {
  /*
   * The magic/state values are used to sanity-check the
   * client's usage.  If an API function is called at a
   * rediculous time, or the client passes us an invalid
   * pointer, these values allow me to catch that.
   */
  unsigned   magic;
  unsigned   state;

  struct archive_entry *entry;
  uid_t    user_uid; /* UID of current user. */

  /* Dev/ino of the archive being read/written. */
  dev_t    skip_file_dev;
  ino_t    skip_file_ino;

  /* Utility:  Pointer to a block of nulls. */
  const unsigned char *nulls;
  size_t    null_length;

  /*
   * Used by archive_read_data() to track blocks and copy
   * data to client buffers, filling gaps with zero bytes.
   */
  const char  *read_data_block;
  off_t    read_data_offset;
  off_t    read_data_output_offset;
  size_t    read_data_remaining;

  /* Callbacks to open/read/write/close archive stream. */
  archive_open_callback *client_opener;
  archive_read_callback *client_reader;
  archive_write_callback *client_writer;
  archive_close_callback *client_closer;
  void   *client_data;

  /*
   * Blocking information.  Note that bytes_in_last_block is
   * misleadingly named; I should find a better name.  These
   * control the final output from all compressors, including
   * compression_none.
   */
  int    bytes_per_block;
  int    bytes_in_last_block;

  /*
   * These control whether data within a gzip/bzip2 compressed
   * stream gets padded or not.  If pad_uncompressed is set,
   * the data will be padded to a full block before being
   * compressed.  The pad_uncompressed_byte determines the value
   * that will be used for padding.  Note that these have no
   * effect on compression "none."
   */
  int    pad_uncompressed;
  int    pad_uncompressed_byte; /* TODO: Support this. */

  /* Position in UNCOMPRESSED data stream. */
  off_t    file_position;
  /* Position in COMPRESSED data stream. */
  off_t    raw_position;
  /* File offset of beginning of most recently-read header. */
  off_t    header_position;

  /*
   * Detection functions for decompression: bid functions are
   * given a block of data from the beginning of the stream and
   * can bid on whether or not they support the data stream.
   * General guideline: bid the number of bits that you actually
   * test, e.g., 16 if you test a 2-byte magic value.  The
   * highest bidder will have their init function invoked, which
   * can set up pointers to specific handlers.
   *
   * On write, the client just invokes an archive_write_set function
   * which sets up the data here directly.
   */
  int   compression_code; /* Currently active compression. */
  const char *compression_name;
  struct {
    int (*bid)(const void *buff, size_t);
    int (*init)(struct archive *, const void *buff, size_t);
  } decompressors[4];
  /* Read/write data stream (with compression). */
  void  *compression_data;  /* Data for (de)compressor. */
  int (*compression_init)(struct archive *); /* Initialize. */
  int (*compression_finish)(struct archive *);
  int (*compression_write)(struct archive *, const void *, size_t);
  /*
   * Read uses a peek/consume I/O model: the decompression code
   * returns a pointer to the requested block and advances the
   * file position only when requested by a consume call.  This
   * reduces copying and also simplifies look-ahead for format
   * detection.
   */
  ssize_t (*compression_read_ahead)(struct archive *,
    const void **, size_t request);
  ssize_t (*compression_read_consume)(struct archive *, size_t);

  /*
   * Format detection is mostly the same as compression
   * detection, with two significant differences: The bidders
   * use the read_ahead calls above to examine the stream rather
   * than having the supervisor hand them a block of data to
   * examine, and the auction is repeated for every header.
   * Winning bidders should set the archive_format and
   * archive_format_name appropriately.  Bid routines should
   * check archive_format and decline to bid if the format of
   * the last header was incompatible.
   *
   * Again, write support is considerably simpler because there's
   * no need for an auction.
   */
  int    archive_format;
  const char  *archive_format_name;

  struct archive_format_descriptor {
    int (*bid)(struct archive *);
    int (*read_header)(struct archive *, struct archive_entry *);
    int (*read_data)(struct archive *, const void **, size_t *, off_t *);
    int (*read_data_skip)(struct archive *);
    int (*cleanup)(struct archive *);
    void  *format_data; /* Format-specific data for readers. */
  } formats[8];
  struct archive_format_descriptor *format; /* Active format. */

  /*
   * Storage for format-specific data.  Note that there can be
   * multiple format readers active at one time, so we need to
   * allow for multiple format readers to have their data
   * available.  The pformat_data slot here is the solution: on
   * read, it is gauranteed to always point to a void* variable
   * that the format can use.
   */
  void **pformat_data;  /* Pointer to current format_data. */
  void  *format_data;  /* Used by writers. */

  /*
   * Pointers to format-specific functions for writing.  They're
   * initialized by archive_write_set_format_XXX() calls.
   */
  int (*format_init)(struct archive *); /* Only used on write. */
  int (*format_finish)(struct archive *);
  int (*format_finish_entry)(struct archive *);
  int  (*format_write_header)(struct archive *,
    struct archive_entry *);
  int (*format_write_data)(struct archive *,
    const void *buff, size_t);

  /*
   * Various information needed by archive_extract.
   */
  struct extract   *extract;
  void   (*extract_progress)(void *);
  void    *extract_progress_user_data;
  void   (*cleanup_archive_extract)(struct archive *);

  int    archive_error_number;
  const char  *error;
  struct archive_string error_string;
};


#define ARCHIVE_STATE_ANY 0xFFFFU
#define ARCHIVE_STATE_NEW 1U
#define ARCHIVE_STATE_HEADER 2U
#define ARCHIVE_STATE_DATA 4U
#define ARCHIVE_STATE_EOF 8U
#define ARCHIVE_STATE_CLOSED 0x10U
#define ARCHIVE_STATE_FATAL 0x8000U

/* Check magic value and state; exit if it isn't valid. */
void __archive_check_magic(struct archive *, unsigned magic,
  unsigned state, const char *func);


int __archive_read_register_format(struct archive *a,
  void *format_data,
  int (*bid)(struct archive *),
  int (*read_header)(struct archive *, struct archive_entry *),
  int (*read_data)(struct archive *, const void **, size_t *, off_t *),
  int (*read_data_skip)(struct archive *),
  int (*cleanup)(struct archive *));

int __archive_read_register_compression(struct archive *a,
  int (*bid)(const void *, size_t),
  int (*init)(struct archive *, const void *, size_t));

void __archive_errx(int retvalue, const char *msg);

#define err_combine(a,b) ((a) < (b) ? (a) : (b))


                                             /*
                                              * Utility function to format a USTAR header into a buffer.  If
                                              * "strict" is set, this tries to create the absolutely most portable
                                              * version of a ustar header.  If "strict" is set to 0, then it will
                                              * relax certain requirements.
                                              *
                                              * Generally, format-specific declarations don't belong in this
                                              * header; this is a rare example of a function that is shared by
                                              * two very similar formats (ustar and pax).
                                              */
                                             int
                                             __archive_write_format_header_ustar(struct archive *, char buff[512],
                                               struct archive_entry *, int tartype, int strict);

//--- archive_entry.h

/*
 * Description of an archive entry.
 *
 * Basically, a "struct stat" with a few text fields added in.
 *
 * TODO: Add "comment", "charset", and possibly other entries that are
 * supported by "pax interchange" format.  However, GNU, ustar, cpio,
 * and other variants don't support these features, so they're not an
 * excruciatingly high priority right now.
 *
 * TODO: "pax interchange" format allows essentially arbitrary
 * key/value attributes to be attached to any entry.  Supporting
 * such extensions may make this library useful for special
 * applications (e.g., a package manager could attach special
 * package-management attributes to each entry).
 */
struct archive_entry;

/*
 * Basic object manipulation
 */

struct archive_entry *archive_entry_clear(struct archive_entry *);
/* The 'clone' function does a deep copy; all of the strings are copied too. */
struct archive_entry *archive_entry_clone(struct archive_entry *);
void    archive_entry_free(struct archive_entry *);
struct archive_entry *archive_entry_new(void);

/*
 * Retrieve fields from an archive_entry.
 */

time_t    archive_entry_atime(struct archive_entry *);
long    archive_entry_atime_nsec(struct archive_entry *);
time_t    archive_entry_ctime(struct archive_entry *);
long    archive_entry_ctime_nsec(struct archive_entry *);
dev_t    archive_entry_dev(struct archive_entry *);
void    archive_entry_fflags(struct archive_entry *,
  unsigned long *set, unsigned long *clear);
const char  *archive_entry_fflags_text(struct archive_entry *);
gid_t    archive_entry_gid(struct archive_entry *);
const char  *archive_entry_gname(struct archive_entry *);
const wchar_t  *archive_entry_gname_w(struct archive_entry *);
const char  *archive_entry_hardlink(struct archive_entry *);
const wchar_t  *archive_entry_hardlink_w(struct archive_entry *);
ino_t    archive_entry_ino(struct archive_entry *);
mode_t    archive_entry_mode(struct archive_entry *);
time_t    archive_entry_mtime(struct archive_entry *);
long    archive_entry_mtime_nsec(struct archive_entry *);
const char  *archive_entry_pathname(struct archive_entry *);
const wchar_t  *archive_entry_pathname_w(struct archive_entry *);
dev_t    archive_entry_rdev(struct archive_entry *);
int64_t    archive_entry_size(struct archive_entry *);
const struct stat *archive_entry_stat(struct archive_entry *);
const char  *archive_entry_symlink(struct archive_entry *);
const wchar_t  *archive_entry_symlink_w(struct archive_entry *);
uid_t    archive_entry_uid(struct archive_entry *);
const char  *archive_entry_uname(struct archive_entry *);
const wchar_t  *archive_entry_uname_w(struct archive_entry *);

/*
 * Set fields in an archive_entry.
 *
 * Note that string 'set' functions do not copy the string, only the pointer.
 * In contrast, 'copy' functions do copy the object pointed to.
 */

void archive_entry_copy_stat(struct archive_entry *, const struct stat *);
void archive_entry_set_atime(struct archive_entry *, time_t, long);
void archive_entry_set_ctime(struct archive_entry *, time_t, long);
void archive_entry_set_fflags(struct archive_entry *,
  unsigned long set, unsigned long clear);
/* Returns pointer to start of first invalid token, or NULL if none. */
/* Note that all recognized tokens are processed, regardless. */
const wchar_t *archive_entry_copy_fflags_text_w(struct archive_entry *,
  const wchar_t *);
void archive_entry_set_gid(struct archive_entry *, gid_t);
void archive_entry_set_gname(struct archive_entry *, const char *);
void archive_entry_copy_gname_w(struct archive_entry *, const wchar_t *);
void archive_entry_set_hardlink(struct archive_entry *, const char *);
void archive_entry_copy_hardlink(struct archive_entry *, const char *);
void archive_entry_copy_hardlink_w(struct archive_entry *, const wchar_t *);
void archive_entry_set_link(struct archive_entry *, const char *);
void archive_entry_set_mode(struct archive_entry *, mode_t);
void archive_entry_set_mtime(struct archive_entry *, time_t, long);
void archive_entry_set_pathname(struct archive_entry *, const char *);
void archive_entry_copy_pathname(struct archive_entry *, const char *);
void archive_entry_copy_pathname_w(struct archive_entry *, const wchar_t *);
void archive_entry_set_size(struct archive_entry *, int64_t);
void archive_entry_set_symlink(struct archive_entry *, const char *);
void archive_entry_copy_symlink_w(struct archive_entry *, const wchar_t *);
void archive_entry_set_uid(struct archive_entry *, uid_t);
void archive_entry_set_uname(struct archive_entry *, const char *);
void archive_entry_copy_uname_w(struct archive_entry *, const wchar_t *);

/*
 * ACL routines.  This used to simply store and return text-format ACL
 * strings, but that proved insufficient for a number of reasons:
 *   = clients need control over uname/uid and gname/gid mappings
 *   = there are many different ACL text formats
 *   = would like to be able to read/convert archives containing ACLs
 *     on platforms that lack ACL libraries
 */

/*
 * Permission bits mimic POSIX.1e.  Note that I've not followed POSIX.1e's
 * "permset"/"perm" abstract type nonsense.  A permset is just a simple
 * bitmap, following long-standing Unix tradition.
 */
#define ARCHIVE_ENTRY_ACL_EXECUTE 1
#define ARCHIVE_ENTRY_ACL_WRITE  2
#define ARCHIVE_ENTRY_ACL_READ  4

/* We need to be able to specify either or both of these. */
#define ARCHIVE_ENTRY_ACL_TYPE_ACCESS 256
#define ARCHIVE_ENTRY_ACL_TYPE_DEFAULT 512

/* Tag values mimic POSIX.1e */
#define ARCHIVE_ENTRY_ACL_USER  10001 /* Specified user. */
#define ARCHIVE_ENTRY_ACL_USER_OBJ  10002 /* User who owns the file. */
#define ARCHIVE_ENTRY_ACL_GROUP  10003 /* Specified group. */
#define ARCHIVE_ENTRY_ACL_GROUP_OBJ 10004 /* Group who owns the file. */
#define ARCHIVE_ENTRY_ACL_MASK  10005 /* Modify group access. */
#define ARCHIVE_ENTRY_ACL_OTHER  10006 /* Public. */

/*
 * Set the ACL by clearing it and adding entries one at a time.
 * Unlike the POSIX.1e ACL routines, you must specify the type
 * (access/default) for each entry.  Internally, the ACL data is just
 * a soup of entries.  API calls here allow you to retrieve just the
 * entries of interest.  This design (which goes against the spirit of
 * POSIX.1e) is useful for handling archive formats that combine
 * default and access information in a single ACL list.
 */
void  archive_entry_acl_clear(struct archive_entry *);
void  archive_entry_acl_add_entry(struct archive_entry *,
  int type, int permset, int tag, int qual, const char *name);
void  archive_entry_acl_add_entry_w(struct archive_entry *,
  int type, int permset, int tag, int qual, const wchar_t *name);

/*
 * To retrieve the ACL, first "reset", then repeatedly ask for the
 * "next" entry.  The want_type parameter allows you to request only
 * access entries or only default entries.
 */
int  archive_entry_acl_reset(struct archive_entry *, int want_type);
int  archive_entry_acl_next(struct archive_entry *, int want_type,
  int *type, int *permset, int *tag, int *qual, const char **name);
int  archive_entry_acl_next_w(struct archive_entry *, int want_type,
  int *type, int *permset, int *tag, int *qual,
  const wchar_t **name);

/*
 * Construct a text-format ACL.  The flags argument is a bitmask that
 * can include any of the following:
 *
 * ARCHIVE_ENTRY_ACL_TYPE_ACCESS - Include access entries.
 * ARCHIVE_ENTRY_ACL_TYPE_DEFAULT - Include default entries.
 * ARCHIVE_ENTRY_ACL_STYLE_EXTRA_ID - Include extra numeric ID field in
 *    each ACL entry.  (As used by 'star'.)
 * ARCHIVE_ENTRY_ACL_STYLE_MARK_DEFAULT - Include "default:" before each
 *    default ACL entry.
 */
#define ARCHIVE_ENTRY_ACL_STYLE_EXTRA_ID 1024
#define ARCHIVE_ENTRY_ACL_STYLE_MARK_DEFAULT 2048
const wchar_t *archive_entry_acl_text_w(struct archive_entry *, int flags);

/* Return a count of entries matching 'want_type' */
int  archive_entry_acl_count(struct archive_entry *, int want_type);

/*
 * Private ACL parser.  This is private because it handles some
 * very weird formats that clients should not be messing with.
 * Clients should only deal with their platform-native formats.
 * Because of the need to support many formats cleanly, new arguments
 * are likely to get added on a regular basis.  Clients who try to use
 * this interface are likely to be surprised when it changes.
 *
 * You were warned!
 */
int   __archive_entry_acl_parse_w(struct archive_entry *,
  const wchar_t *, int type);
//--- archive_write.h

/*
 * Allocate, initialize and return an archive object.
 */
  struct archive *
archive_write_new(void)
{
  struct archive *a;
  unsigned char *nulls;

  a = (struct archive*)malloc(sizeof(*a));
  if (a == NULL)
    return (NULL);
  memset(a, 0, sizeof(*a));
  a->magic = ARCHIVE_WRITE_MAGIC;
#ifdef HAVE_GETEUID
  a->user_uid = geteuid();
#endif
  a->bytes_per_block = ARCHIVE_DEFAULT_BYTES_PER_BLOCK;
  a->bytes_in_last_block = -1; /* Default */
  a->state = ARCHIVE_STATE_NEW;
  a->pformat_data = &(a->format_data);

  /* Initialize a block of nulls for padding purposes. */
  a->null_length = 1024;
  nulls = (unsigned char*)malloc(a->null_length);
  if (nulls == NULL) {
    free(a);
    return (NULL);
  }
  memset(nulls, 0, a->null_length);
  a->nulls = nulls;
  /*
   * Set default compression, but don't set a default format.
   * Were we to set a default format here, we would force every
   * client to link in support for that format, even if they didn't
   * ever use it.
   */
  archive_write_set_compression_none(a);
  return (a);
}


/*
 * Set the block size.  Returns 0 if successful.
 */
  int
archive_write_set_bytes_per_block(struct archive *a, int bytes_per_block)
{
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_NEW, "archive_write_set_bytes_per_block");
  a->bytes_per_block = bytes_per_block;
  return (ARCHIVE_OK);
}


/*
 * Set the size for the last block.
 * Returns 0 if successful.
 */
  int
archive_write_set_bytes_in_last_block(struct archive *a, int bytes)
{
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_ANY, "archive_write_set_bytes_in_last_block");
  a->bytes_in_last_block = bytes;
  return (ARCHIVE_OK);
}


/*
 * Open the archive using the current settings.
 */
  int
archive_write_open(struct archive *a, void *client_data,
  archive_open_callback *opener, archive_write_callback *writer,
  archive_close_callback *closer)
{
  int ret;

  ret = ARCHIVE_OK;
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_NEW, "archive_write_open");
  archive_string_empty(&a->error_string);
  a->state = ARCHIVE_STATE_HEADER;
  a->client_data = client_data;
  a->client_writer = writer;
  a->client_opener = opener;
  a->client_closer = closer;
  ret = (a->compression_init)(a);
  if (a->format_init && ret == ARCHIVE_OK)
    ret = (a->format_init)(a);
  return (ret);
}


/*
 * Close out the archive.
 *
 * Be careful: user might just call write_new and then write_finish.
 * Don't assume we actually wrote anything or performed any non-trivial
 * initialization.
 */
  int
archive_write_close(struct archive *a)
{
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_ANY, "archive_write_close");

  /* Finish the last entry. */
  if (a->state & ARCHIVE_STATE_DATA)
    ((a->format_finish_entry)(a));

  /* Finish off the archive. */
  if (a->format_finish != NULL)
    (a->format_finish)(a);

  /* Finish the compression and close the stream. */
  if (a->compression_finish != NULL)
    (a->compression_finish)(a);

  a->state = ARCHIVE_STATE_CLOSED;
  return (ARCHIVE_OK);
}

/*
 * Destroy the archive structure.
 */
  void
archive_write_finish(struct archive *a)
{
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_ANY, "archive_write_finish");
  if (a->state != ARCHIVE_STATE_CLOSED)
    archive_write_close(a);

  /* Release various dynamic buffers. */
  free((void *)(uintptr_t)(const void *)a->nulls);
  archive_string_free(&a->error_string);
  a->magic = 0;
  free(a);
}


/*
 * Write the appropriate header.
 */
  int
archive_write_header(struct archive *a, struct archive_entry *entry)
{
  int ret;

  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC,
    ARCHIVE_STATE_HEADER | ARCHIVE_STATE_DATA, "archive_write_header");
  archive_string_empty(&a->error_string);

  /* Finish last entry. */
  if (a->state & ARCHIVE_STATE_DATA)
    ((a->format_finish_entry)(a));

  if (archive_entry_dev(entry) == a->skip_file_dev &&
    archive_entry_ino(entry) == a->skip_file_ino) {
    archive_set_error(a, 0, "Can't add archive to itself");
    return (ARCHIVE_WARN);
  }

  /* Format and write header. */
  ret = ((a->format_write_header)(a, entry));

  a->state = ARCHIVE_STATE_DATA;
  return (ret);
}

/*
 * Note that the compressor is responsible for blocking.
 */
/* Should be "ssize_t", but that breaks the ABI.  <sigh> */
  int
archive_write_data(struct archive *a, const void *buff, size_t s)
{
  int ret;
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_DATA, "archive_write_data");
  archive_string_empty(&a->error_string);
  ret = (a->format_write_data)(a, buff, s);
  return (ret == ARCHIVE_OK ? (ssize_t)s : -1);
}

//--- archive_write_set_compression_none.c

static int archive_compressor_none_finish(struct archive *a);
static int archive_compressor_none_init(struct archive *);
static int archive_compressor_none_write(struct archive *, const void *,
  size_t);

struct archive_none {
  char *buffer;
  ssize_t  buffer_size;
  char *next;  /* Current insert location */
  ssize_t  avail;  /* Free space left in buffer */
};

  int
archive_write_set_compression_none(struct archive *a)
{
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_NEW, "archive_write_set_compression_none");
  a->compression_init = &archive_compressor_none_init;
  a->compression_code = ARCHIVE_COMPRESSION_NONE;
  a->compression_name = "none";
  return (0);
}

/*
 * Setup callback.
 */
  static int
archive_compressor_none_init(struct archive *a)
{
  int ret;
  struct archive_none *state;

  a->compression_code = ARCHIVE_COMPRESSION_NONE;
  a->compression_name = "none";

  if (a->client_opener != NULL) {
    ret = (a->client_opener)(a, a->client_data);
    if (ret != 0)
      return (ret);
  }

  state = (struct archive_none *)malloc(sizeof(*state));
  if (state == NULL) {
    archive_set_error(a, ENOMEM,
      "Can't allocate data for output buffering");
    return (ARCHIVE_FATAL);
  }
  memset(state, 0, sizeof(*state));

  state->buffer_size = a->bytes_per_block;
  state->buffer = (char*)malloc(state->buffer_size);

  if (state->buffer == NULL) {
    archive_set_error(a, ENOMEM,
      "Can't allocate output buffer");
    free(state);
    return (ARCHIVE_FATAL);
  }

  state->next = state->buffer;
  state->avail = state->buffer_size;

  a->compression_data = state;
  a->compression_write = archive_compressor_none_write;
  a->compression_finish = archive_compressor_none_finish;
  return (ARCHIVE_OK);
}

/*
 * Write data to the stream.
 */
  static int
archive_compressor_none_write(struct archive *a, const void *vbuff,
  size_t length)
{
  const char *buff;
  ssize_t remaining, to_copy;
  ssize_t bytes_written;
  struct archive_none *state;

  state = (struct archive_none*)a->compression_data;
  buff = (const char*)vbuff;
  if (a->client_writer == NULL) {
    archive_set_error(a, ARCHIVE_ERRNO_PROGRAMMER,
      "No write callback is registered?  "
      "This is probably an internal programming error.");
    return (ARCHIVE_FATAL);
  }

  remaining = length;
  while (remaining > 0) {
    /*
     * If we have a full output block, write it and reset the
     * output buffer.
     */
    if (state->avail == 0) {
      bytes_written = (a->client_writer)(a, a->client_data,
        state->buffer, state->buffer_size);
      if (bytes_written <= 0)
        return (ARCHIVE_FATAL);
      /* XXX TODO: if bytes_written < state->buffer_size */
      a->raw_position += bytes_written;
      state->next = state->buffer;
      state->avail = state->buffer_size;
    }

    /* Now we have space in the buffer; copy new data into it. */
    to_copy = (remaining > state->avail) ?
      state->avail : remaining;
    memcpy(state->next, buff, to_copy);
    state->next += to_copy;
    state->avail -= to_copy;
    buff += to_copy;
    remaining -= to_copy;
  }
  a->file_position += length;
  return (ARCHIVE_OK);
}


/*
 * Finish the compression.
 */
  static int
archive_compressor_none_finish(struct archive *a)
{
  ssize_t block_length;
  ssize_t target_block_length;
  ssize_t bytes_written;
  int ret;
  int ret2;
  struct archive_none *state;

  state = (struct archive_none*)a->compression_data;
  ret = ret2 = ARCHIVE_OK;
  if (a->client_writer == NULL) {
    archive_set_error(a, ARCHIVE_ERRNO_PROGRAMMER,
      "No write callback is registered?  "
      "This is probably an internal programming error.");
    return (ARCHIVE_FATAL);
  }

  /* If there's pending data, pad and write the last block */
  if (state->next != state->buffer) {
    block_length = state->buffer_size - state->avail;

    /* Tricky calculation to determine size of last block */
    target_block_length = block_length;
    if (a->bytes_in_last_block <= 0)
      /* Default or Zero: pad to full block */
      target_block_length = a->bytes_per_block;
    else
      /* Round to next multiple of bytes_in_last_block. */
      target_block_length = a->bytes_in_last_block *
        ( (block_length + a->bytes_in_last_block - 1) /
          a->bytes_in_last_block);
    if (target_block_length > a->bytes_per_block)
      target_block_length = a->bytes_per_block;
    if (block_length < target_block_length) {
      memset(state->next, 0,
        target_block_length - block_length);
      block_length = target_block_length;
    }
    bytes_written = (a->client_writer)(a, a->client_data,
      state->buffer, block_length);
    if (bytes_written <= 0)
      ret = ARCHIVE_FATAL;
    else {
      a->raw_position += bytes_written;
      ret = ARCHIVE_OK;
    }
  }

  /* Close the output */
  if (a->client_closer != NULL)
    ret2 = (a->client_closer)(a, a->client_data);

  free(state->buffer);
  free(state);
  a->compression_data = NULL;

  return (ret != ARCHIVE_OK ? ret : ret2);
}

//--- archive_check_magic.c

  static void
errmsg(const char *m)
{
  cmSystemTools::Error("CPack error: ", m);
}

  static void
diediedie(void)
{
  *(char *)0 = 1; /* Deliberately segfault and force a coredump. */
  _exit(1); /* If that didn't work, just exit with an error. */
}

  static const char *
state_name(unsigned s)
{
  switch (s) {
  case ARCHIVE_STATE_NEW:  return ("new");
  case ARCHIVE_STATE_HEADER: return ("header");
  case ARCHIVE_STATE_DATA: return ("data");
  case ARCHIVE_STATE_EOF:  return ("eof");
  case ARCHIVE_STATE_CLOSED: return ("closed");
  case ARCHIVE_STATE_FATAL: return ("fatal");
  default:   return ("??");
  }
}


  static void
write_all_states(int states)
{
  unsigned lowbit;

  /* A trick for computing the lowest set bit. */
  while ((lowbit = states & (-states)) != 0) {
    states &= ~lowbit;  /* Clear the low bit. */
    errmsg(state_name(lowbit));
    if (states != 0)
      errmsg("/");
  }
}

/*
 * Check magic value and current state; bail if it isn't valid.
 *
 * This is designed to catch serious programming errors that violate
 * the libarchive API.
 */
  void
__archive_check_magic(struct archive *a, unsigned magic, unsigned state,
  const char *function)
{
  if (a->magic != magic) {
    errmsg("INTERNAL ERROR: Function ");
    errmsg(function);
    errmsg(" invoked with invalid struct archive structure.\n");
    diediedie();
  }

  if (state == ARCHIVE_STATE_ANY)
    return;

  if ((a->state & state) == 0) {
    errmsg("INTERNAL ERROR: Function '");
    errmsg(function);
    errmsg("' invoked with archive structure in state '");
    write_all_states(a->state);
    errmsg("', should be in state '");
    write_all_states(state);
    errmsg("'\n");
    diediedie();
  }
}

//--- archive_util.c
  int
archive_errno(struct archive *a)
{
  return (a->archive_error_number);
}

  const char *
archive_error_string(struct archive *a)
{

  if (a->error != NULL  &&  *a->error != '\0')
    return (a->error);
  else
    return ("(Empty error message)");
}


  int
archive_format(struct archive *a)
{
  return (a->archive_format);
}

  const char *
archive_format_name(struct archive *a)
{
  return (a->archive_format_name);
}


  int
archive_compression(struct archive *a)
{
  return (a->compression_code);
}

  const char *
archive_compression_name(struct archive *a)
{
  return (a->compression_name);
}


/*
 * Return a count of the number of compressed bytes processed.
 */
  int64_t
archive_position_compressed(struct archive *a)
{
  return (a->raw_position);
}

/*
 * Return a count of the number of uncompressed bytes processed.
 */
  int64_t
archive_position_uncompressed(struct archive *a)
{
  return (a->file_position);
}


  void
archive_set_error(struct archive *a, int error_number, const char *fmt, ...)
{
  va_list ap;
#ifdef HAVE_STRERROR_R
  char errbuff[512];
#endif
  char *errp;

  a->archive_error_number = error_number;
  if (fmt == NULL) {
    a->error = NULL;
    return;
  }

  va_start(ap, fmt);
  archive_string_vsprintf(&(a->error_string), fmt, ap);
  if (error_number > 0) {
    archive_strcat(&(a->error_string), ": ");
#ifdef HAVE_STRERROR_R
#ifdef STRERROR_R_CHAR_P
    errp = strerror_r(error_number, errbuff, sizeof(errbuff));
#else
    strerror_r(error_number, errbuff, sizeof(errbuff));
    errp = errbuff;
#endif
#else
    /* Note: this is not threadsafe! */
    errp = strerror(error_number);
#endif
    archive_strcat(&(a->error_string), errp);
  }
  a->error = a->error_string.s;
  va_end(ap);
}

  void
__archive_errx(int retvalue, const char *msg)
{
  static const char *msg1 = "Fatal Internal Error in libarchive: ";
  write(2, msg1, strlen(msg1));
  write(2, msg, strlen(msg));
  write(2, "\n", 1);
  exit(retvalue);
}

//--- archive_string.c
  struct archive_string *
__archive_string_append(struct archive_string *as, const char *p, size_t s)
{
  __archive_string_ensure(as, as->length + s + 1);
  memcpy(as->s + as->length, p, s);
  as->s[as->length + s] = 0;
  as->length += s;
  return (as);
}

  void
__archive_string_free(struct archive_string *as)
{
  as->length = 0;
  as->buffer_length = 0;
  if (as->s != NULL)
    free(as->s);
}

  struct archive_string *
__archive_string_ensure(struct archive_string *as, size_t s)
{
  if (as->s && (s <= as->buffer_length))
    return (as);

  if (as->buffer_length < 32)
    as->buffer_length = 32;
  while (as->buffer_length < s)
    as->buffer_length *= 2;
  as->s = (char*)realloc(as->s, as->buffer_length);
  /* TODO: Return null instead and fix up all of our callers to
   * handle this correctly. */
  if (as->s == NULL)
    __archive_errx(1, "Out of memory");
  return (as);
}

  struct archive_string *
__archive_strncat(struct archive_string *as, const char *p, size_t n)
{
  size_t s;
  const char *pp;

  /* Like strlen(p), except won't examine positions beyond p[n]. */
  s = 0;
  pp = p;
  while (*pp && s < n) {
    pp++;
    s++;
  }
  return (__archive_string_append(as, p, s));
}

  struct archive_string *
__archive_strappend_char(struct archive_string *as, char c)
{
  return (__archive_string_append(as, &c, 1));
}

  struct archive_string *
__archive_strappend_int(struct archive_string *as, int d, int base)
{
  static const char *digits = "0123457890abcdef";

  if (d < 0) {
    __archive_strappend_char(as, '-');
    d = -d;
  }
  if (d >= base)
    __archive_strappend_int(as, d/base, base);
  __archive_strappend_char(as, digits[d % base]);
  return (as);
}

//--- archive_entry.c
/* Obtain suitable wide-character manipulation functions. */
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#else
static size_t wcslen(const wchar_t *s)
{
  const wchar_t *p = s;
  while (*p != L'\0')
    ++p;
  return p - s;
}
static wchar_t * wcscpy(wchar_t *s1, const wchar_t *s2)
{
  wchar_t *dest = s1;
  while((*s1 = *s2) != L'\0')
    ++s1, ++s2;
  return dest;
}
#define wmemcpy(a,b,i)  (wchar_t *)memcpy((a),(b),(i)*sizeof(wchar_t))
/* Good enough for simple equality testing, but not for sorting. */
#define wmemcmp(a,b,i)  memcmp((a),(b),(i)*sizeof(wchar_t))
#endif

#undef max
#define max(a, b) ((a)>(b)?(a):(b))

                                /*
                                 * Handle wide character (i.e., Unicode) and non-wide character
                                 * strings transparently.
                                 *
                                 */

                                struct aes {
                                  const char *aes_mbs;
                                  char *aes_mbs_alloc;
                                  const wchar_t *aes_wcs;
                                  wchar_t *aes_wcs_alloc;
                                };

struct ae_acl {
  struct ae_acl *next;
  int type;   /* E.g., access or default */
  int tag;   /* E.g., user/group/other/mask */
  int permset;  /* r/w/x bits */
  int id;   /* uid/gid for user/group */
  struct aes name;  /* uname/gname */
};

static void aes_clean(struct aes *);
static void aes_copy(struct aes *dest, struct aes *src);
static const char * aes_get_mbs(struct aes *);
static const wchar_t * aes_get_wcs(struct aes *);
static void aes_set_mbs(struct aes *, const char *mbs);
static void aes_copy_mbs(struct aes *, const char *mbs);
/* static void aes_set_wcs(struct aes *, const wchar_t *wcs); */
static void aes_copy_wcs(struct aes *, const wchar_t *wcs);

static char *  ae_fflagstostr(unsigned long bitset, unsigned long bitclear);
static const wchar_t *ae_wcstofflags(const wchar_t *stringp,
  unsigned long *setp, unsigned long *clrp);
static void append_entry_w(wchar_t **wp, const wchar_t *prefix, int tag,
  const wchar_t *wname, int perm, int id);
static void append_id_w(wchar_t **wp, int id);

static int acl_special(struct archive_entry *entry,
  int type, int permset, int tag);
static struct ae_acl *acl_new_entry(struct archive_entry *entry,
  int type, int permset, int tag, int id);
static void next_field_w(const wchar_t **wp, const wchar_t **start,
  const wchar_t **end, wchar_t *sep);
static int prefix_w(const wchar_t *start, const wchar_t *end,
  const wchar_t *test);


/*
 * Description of an archive entry.
 *
 * Basically, this is a "struct stat" with a few text fields added in.
 *
 * TODO: Add "comment", "charset", and possibly other entries
 * that are supported by "pax interchange" format.  However, GNU, ustar,
 * cpio, and other variants don't support these features, so they're not an
 * excruciatingly high priority right now.
 *
 * TODO: "pax interchange" format allows essentially arbitrary
 * key/value attributes to be attached to any entry.  Supporting
 * such extensions may make this library useful for special
 * applications (e.g., a package manager could attach special
 * package-management attributes to each entry).  There are tricky
 * API issues involved, so this is not going to happen until
 * there's a real demand for it.
 *
 * TODO: Design a good API for handling sparse files.
 */
struct archive_entry {
  /*
   * Note that ae_stat.st_mode & S_IFMT  can be  0!
   *
   * This occurs when the actual file type of the object is not
   * in the archive.  For example, 'tar' archives store
   * hardlinks without marking the type of the underlying
   * object.
   */
  struct stat ae_stat;

  /*
   * Use aes here so that we get transparent mbs<->wcs conversions.
   */
  struct aes ae_fflags_text; /* Text fflags per fflagstostr(3) */
  unsigned long ae_fflags_set;  /* Bitmap fflags */
  unsigned long ae_fflags_clear;
  struct aes ae_gname;  /* Name of owning group */
  struct aes ae_hardlink; /* Name of target for hardlink */
  struct aes ae_pathname; /* Name of entry */
  struct aes ae_symlink;  /* symlink contents */
  struct aes ae_uname;  /* Name of owner */

  struct ae_acl *acl_head;
  struct ae_acl *acl_p;
  int   acl_state; /* See acl_next for details. */
  wchar_t  *acl_text_w;
};

  static void
aes_clean(struct aes *aes)
{
  if (aes->aes_mbs_alloc) {
    free(aes->aes_mbs_alloc);
    aes->aes_mbs_alloc = NULL;
  }
  if (aes->aes_wcs_alloc) {
    free(aes->aes_wcs_alloc);
    aes->aes_wcs_alloc = NULL;
  }
  memset(aes, 0, sizeof(*aes));
}

  static void
aes_copy(struct aes *dest, struct aes *src)
{
  *dest = *src;
  if (src->aes_mbs != NULL) {
    dest->aes_mbs_alloc = strdup(src->aes_mbs);
    dest->aes_mbs = dest->aes_mbs_alloc;
    if (dest->aes_mbs == NULL)
      __archive_errx(1, "No memory for aes_copy()");
  }

  if (src->aes_wcs != NULL) {
    dest->aes_wcs_alloc = (wchar_t*)malloc((wcslen(src->aes_wcs) + 1)
      * sizeof(wchar_t));
    dest->aes_wcs = dest->aes_wcs_alloc;
    if (dest->aes_wcs == NULL)
      __archive_errx(1, "No memory for aes_copy()");
    wcscpy(dest->aes_wcs_alloc, src->aes_wcs);
  }
}

  static const char *
aes_get_mbs(struct aes *aes)
{
  if (aes->aes_mbs == NULL && aes->aes_wcs == NULL)
    return NULL;
  if (aes->aes_mbs == NULL && aes->aes_wcs != NULL) {
    /*
     * XXX Need to estimate the number of byte in the
     * multi-byte form.  Assume that, on average, wcs
     * chars encode to no more than 3 bytes.  There must
     * be a better way... XXX
     */
    int mbs_length = wcslen(aes->aes_wcs) * 3 + 64;
    aes->aes_mbs_alloc = (char*)malloc(mbs_length);
    aes->aes_mbs = aes->aes_mbs_alloc;
    if (aes->aes_mbs == NULL)
      __archive_errx(1, "No memory for aes_get_mbs()");
    wcstombs(aes->aes_mbs_alloc, aes->aes_wcs, mbs_length - 1);
    aes->aes_mbs_alloc[mbs_length - 1] = 0;
  }
  return (aes->aes_mbs);
}

  static const wchar_t *
aes_get_wcs(struct aes *aes)
{
  if (aes->aes_wcs == NULL && aes->aes_mbs == NULL)
    return NULL;
  if (aes->aes_wcs == NULL && aes->aes_mbs != NULL) {
    /*
     * No single byte will be more than one wide character,
     * so this length estimate will always be big enough.
     */
    int wcs_length = strlen(aes->aes_mbs);
    aes->aes_wcs_alloc
      = (wchar_t*)malloc((wcs_length + 1) * sizeof(wchar_t));
    aes->aes_wcs = aes->aes_wcs_alloc;
    if (aes->aes_wcs == NULL)
      __archive_errx(1, "No memory for aes_get_wcs()");
    mbstowcs(aes->aes_wcs_alloc, aes->aes_mbs, wcs_length);
    aes->aes_wcs_alloc[wcs_length] = 0;
  }
  return (aes->aes_wcs);
}

  static void
aes_set_mbs(struct aes *aes, const char *mbs)
{
  if (aes->aes_mbs_alloc) {
    free(aes->aes_mbs_alloc);
    aes->aes_mbs_alloc = NULL;
  }
  if (aes->aes_wcs_alloc) {
    free(aes->aes_wcs_alloc);
    aes->aes_wcs_alloc = NULL;
  }
  aes->aes_mbs = mbs;
  aes->aes_wcs = NULL;
}

  static void
aes_copy_mbs(struct aes *aes, const char *mbs)
{
  if (aes->aes_mbs_alloc) {
    free(aes->aes_mbs_alloc);
    aes->aes_mbs_alloc = NULL;
  }
  if (aes->aes_wcs_alloc) {
    free(aes->aes_wcs_alloc);
    aes->aes_wcs_alloc = NULL;
  }
  aes->aes_mbs_alloc = (char*)malloc((strlen(mbs) + 1) * sizeof(char));
  if (aes->aes_mbs_alloc == NULL)
    __archive_errx(1, "No memory for aes_copy_mbs()");
  strcpy(aes->aes_mbs_alloc, mbs);
  aes->aes_mbs = aes->aes_mbs_alloc;
  aes->aes_wcs = NULL;
}

#if 0
  static void
aes_set_wcs(struct aes *aes, const wchar_t *wcs)
{
  if (aes->aes_mbs_alloc) {
    free(aes->aes_mbs_alloc);
    aes->aes_mbs_alloc = NULL;
  }
  if (aes->aes_wcs_alloc) {
    free(aes->aes_wcs_alloc);
    aes->aes_wcs_alloc = NULL;
  }
  aes->aes_mbs = NULL;
  aes->aes_wcs = wcs;
}
#endif

  static void
aes_copy_wcs(struct aes *aes, const wchar_t *wcs)
{
  if (aes->aes_mbs_alloc) {
    free(aes->aes_mbs_alloc);
    aes->aes_mbs_alloc = NULL;
  }
  if (aes->aes_wcs_alloc) {
    free(aes->aes_wcs_alloc);
    aes->aes_wcs_alloc = NULL;
  }
  aes->aes_mbs = NULL;
  aes->aes_wcs_alloc = (wchar_t*)malloc((wcslen(wcs) + 1) * sizeof(wchar_t));
  if (aes->aes_wcs_alloc == NULL)
    __archive_errx(1, "No memory for aes_copy_wcs()");
  wcscpy(aes->aes_wcs_alloc, wcs);
  aes->aes_wcs = aes->aes_wcs_alloc;
}

  struct archive_entry *
archive_entry_clear(struct archive_entry *entry)
{
  aes_clean(&entry->ae_fflags_text);
  aes_clean(&entry->ae_gname);
  aes_clean(&entry->ae_hardlink);
  aes_clean(&entry->ae_pathname);
  aes_clean(&entry->ae_symlink);
  aes_clean(&entry->ae_uname);
  archive_entry_acl_clear(entry);
  memset(entry, 0, sizeof(*entry));
  return entry;
}

  struct archive_entry *
archive_entry_clone(struct archive_entry *entry)
{
  struct archive_entry *entry2;

  /* Allocate new structure and copy over all of the fields. */
  entry2 = (struct archive_entry*)malloc(sizeof(*entry2));
  if (entry2 == NULL)
    return (NULL);
  memset(entry2, 0, sizeof(*entry2));
  entry2->ae_stat = entry->ae_stat;
  entry2->ae_fflags_set = entry->ae_fflags_set;
  entry2->ae_fflags_clear = entry->ae_fflags_clear;

  aes_copy(&entry2->ae_fflags_text, &entry->ae_fflags_text);
  aes_copy(&entry2->ae_gname, &entry->ae_gname);
  aes_copy(&entry2->ae_hardlink, &entry->ae_hardlink);
  aes_copy(&entry2->ae_pathname, &entry->ae_pathname);
  aes_copy(&entry2->ae_symlink, &entry->ae_symlink);
  aes_copy(&entry2->ae_uname, &entry->ae_uname);

  /* XXX TODO: Copy ACL data over as well. XXX */
  return (entry2);
}

  void
archive_entry_free(struct archive_entry *entry)
{
  archive_entry_clear(entry);
  free(entry);
}

  struct archive_entry *
archive_entry_new(void)
{
  struct archive_entry *entry;

  entry = (struct archive_entry*)malloc(sizeof(*entry));
  if (entry == NULL)
    return (NULL);
  memset(entry, 0, sizeof(*entry));
  return (entry);
}

/*
 * Functions for reading fields from an archive_entry.
 */

  time_t
archive_entry_atime(struct archive_entry *entry)
{
  return (entry->ae_stat.st_atime);
}

  long
archive_entry_atime_nsec(struct archive_entry *entry)
{
  (void)entry; /* entry can be unused here. */
  return (ARCHIVE_STAT_ATIME_NANOS(&entry->ae_stat));
}

  time_t
archive_entry_ctime(struct archive_entry *entry)
{
  return (entry->ae_stat.st_ctime);
}

  long
archive_entry_ctime_nsec(struct archive_entry *entry)
{
  (void)entry; /* entry can be unused here. */
  return (ARCHIVE_STAT_CTIME_NANOS(&entry->ae_stat));
}

  dev_t
archive_entry_dev(struct archive_entry *entry)
{
  return (entry->ae_stat.st_dev);
}

  void
archive_entry_fflags(struct archive_entry *entry,
  unsigned long *set, unsigned long *clear)
{
  *set = entry->ae_fflags_set;
  *clear = entry->ae_fflags_clear;
}

/*
 * Note: if text was provided, this just returns that text.  If you
 * really need the text to be rebuilt in a canonical form, set the
 * text, ask for the bitmaps, then set the bitmaps.  (Setting the
 * bitmaps clears any stored text.)  This design is deliberate: if
 * we're editing archives, we don't want to discard flags just because
 * they aren't supported on the current system.  The bitmap<->text
 * conversions are platform-specific (see below).
 */
  const char *
archive_entry_fflags_text(struct archive_entry *entry)
{
  const char *f;
  char *p;

  f = aes_get_mbs(&entry->ae_fflags_text);
  if (f != NULL)
    return (f);

  if (entry->ae_fflags_set == 0  &&  entry->ae_fflags_clear == 0)
    return (NULL);

  p = ae_fflagstostr(entry->ae_fflags_set, entry->ae_fflags_clear);
  if (p == NULL)
    return (NULL);

  aes_copy_mbs(&entry->ae_fflags_text, p);
  free(p);
  f = aes_get_mbs(&entry->ae_fflags_text);
  return (f);
}

  gid_t
archive_entry_gid(struct archive_entry *entry)
{
  return (entry->ae_stat.st_gid);
}

  const char *
archive_entry_gname(struct archive_entry *entry)
{
  return (aes_get_mbs(&entry->ae_gname));
}

  const wchar_t *
archive_entry_gname_w(struct archive_entry *entry)
{
  return (aes_get_wcs(&entry->ae_gname));
}

  const char *
archive_entry_hardlink(struct archive_entry *entry)
{
  return (aes_get_mbs(&entry->ae_hardlink));
}

  const wchar_t *
archive_entry_hardlink_w(struct archive_entry *entry)
{
  return (aes_get_wcs(&entry->ae_hardlink));
}

  ino_t
archive_entry_ino(struct archive_entry *entry)
{
  return (entry->ae_stat.st_ino);
}

  mode_t
archive_entry_mode(struct archive_entry *entry)
{
  return (entry->ae_stat.st_mode);
}

  time_t
archive_entry_mtime(struct archive_entry *entry)
{
  return (entry->ae_stat.st_mtime);
}

  long
archive_entry_mtime_nsec(struct archive_entry *entry)
{
  (void)entry; /* entry can be unused here. */
  return (ARCHIVE_STAT_MTIME_NANOS(&entry->ae_stat));
}

  const char *
archive_entry_pathname(struct archive_entry *entry)
{
  return (aes_get_mbs(&entry->ae_pathname));
}

  const wchar_t *
archive_entry_pathname_w(struct archive_entry *entry)
{
  return (aes_get_wcs(&entry->ae_pathname));
}

  dev_t
archive_entry_rdev(struct archive_entry *entry)
{
  return (entry->ae_stat.st_rdev);
}

  int64_t
archive_entry_size(struct archive_entry *entry)
{
  return (entry->ae_stat.st_size);
}

  const struct stat *
archive_entry_stat(struct archive_entry *entry)
{
  return (&entry->ae_stat);
}

  const char *
archive_entry_symlink(struct archive_entry *entry)
{
  return (aes_get_mbs(&entry->ae_symlink));
}

  const wchar_t *
archive_entry_symlink_w(struct archive_entry *entry)
{
  return (aes_get_wcs(&entry->ae_symlink));
}

  uid_t
archive_entry_uid(struct archive_entry *entry)
{
  return (entry->ae_stat.st_uid);
}

  const char *
archive_entry_uname(struct archive_entry *entry)
{
  return (aes_get_mbs(&entry->ae_uname));
}

  const wchar_t *
archive_entry_uname_w(struct archive_entry *entry)
{
  return (aes_get_wcs(&entry->ae_uname));
}

/*
 * Functions to set archive_entry properties.
 */

/*
 * Note "copy" not "set" here.  The "set" functions that accept a pointer
 * only store the pointer; they don't copy the underlying object.
 */
  void
archive_entry_copy_stat(struct archive_entry *entry, const struct stat *st)
{
  entry->ae_stat = *st;
}

  void
archive_entry_set_fflags(struct archive_entry *entry,
  unsigned long set, unsigned long clear)
{
  aes_clean(&entry->ae_fflags_text);
  entry->ae_fflags_set = set;
  entry->ae_fflags_clear = clear;
}

  const wchar_t *
archive_entry_copy_fflags_text_w(struct archive_entry *entry,
  const wchar_t *flags)
{
  aes_copy_wcs(&entry->ae_fflags_text, flags);
  return (ae_wcstofflags(flags,
      &entry->ae_fflags_set, &entry->ae_fflags_clear));
}

  void
archive_entry_set_gid(struct archive_entry *entry, gid_t g)
{
  entry->ae_stat.st_gid = g;
}

  void
archive_entry_set_gname(struct archive_entry *entry, const char *name)
{
  aes_set_mbs(&entry->ae_gname, name);
}

  void
archive_entry_copy_gname_w(struct archive_entry *entry, const wchar_t *name)
{
  aes_copy_wcs(&entry->ae_gname, name);
}

  void
archive_entry_set_hardlink(struct archive_entry *entry, const char *target)
{
  aes_set_mbs(&entry->ae_hardlink, target);
}

  void
archive_entry_copy_hardlink(struct archive_entry *entry, const char *target)
{
  aes_copy_mbs(&entry->ae_hardlink, target);
}

  void
archive_entry_copy_hardlink_w(struct archive_entry *entry, const wchar_t *target)
{
  aes_copy_wcs(&entry->ae_hardlink, target);
}

  void
archive_entry_set_atime(struct archive_entry *entry, time_t t, long ns)
{
  (void)ns;
  entry->ae_stat.st_atime = t;
  ARCHIVE_STAT_SET_ATIME_NANOS(&entry->ae_stat, ns);
}

  void
archive_entry_set_ctime(struct archive_entry *entry, time_t t, long ns)
{
  (void)ns;
  entry->ae_stat.st_ctime = t;
  ARCHIVE_STAT_SET_CTIME_NANOS(&entry->ae_stat, ns);
}

/* Set symlink if symlink is already set, else set hardlink. */
  void
archive_entry_set_link(struct archive_entry *entry, const char *target)
{
  if (entry->ae_symlink.aes_mbs != NULL ||
    entry->ae_symlink.aes_wcs != NULL)
    aes_set_mbs(&entry->ae_symlink, target);
  else
    aes_set_mbs(&entry->ae_hardlink, target);
}

  void
archive_entry_set_mode(struct archive_entry *entry, mode_t m)
{
  entry->ae_stat.st_mode = m;
}

  void
archive_entry_set_mtime(struct archive_entry *entry, time_t m, long ns)
{
  (void)ns;
  entry->ae_stat.st_mtime = m;
  ARCHIVE_STAT_SET_MTIME_NANOS(&entry->ae_stat, ns);
}

  void
archive_entry_set_pathname(struct archive_entry *entry, const char *name)
{
  aes_set_mbs(&entry->ae_pathname, name);
}

  void
archive_entry_copy_pathname(struct archive_entry *entry, const char *name)
{
  aes_copy_mbs(&entry->ae_pathname, name);
}

  void
archive_entry_copy_pathname_w(struct archive_entry *entry, const wchar_t *name)
{
  aes_copy_wcs(&entry->ae_pathname, name);
}

  void
archive_entry_set_size(struct archive_entry *entry, int64_t s)
{
  entry->ae_stat.st_size = s;
}

  void
archive_entry_set_symlink(struct archive_entry *entry, const char *linkname)
{
  aes_set_mbs(&entry->ae_symlink, linkname);
}

  void
archive_entry_copy_symlink_w(struct archive_entry *entry, const wchar_t *linkname)
{
  aes_copy_wcs(&entry->ae_symlink, linkname);
}

  void
archive_entry_set_uid(struct archive_entry *entry, uid_t u)
{
  entry->ae_stat.st_uid = u;
}

  void
archive_entry_set_uname(struct archive_entry *entry, const char *name)
{
  aes_set_mbs(&entry->ae_uname, name);
}

  void
archive_entry_copy_uname_w(struct archive_entry *entry, const wchar_t *name)
{
  aes_copy_wcs(&entry->ae_uname, name);
}

/*
 * ACL management.  The following would, of course, be a lot simpler
 * if: 1) the last draft of POSIX.1e were a really thorough and
 * complete standard that addressed the needs of ACL archiving and 2)
 * everyone followed it faithfully.  Alas, neither is true, so the
 * following is a lot more complex than might seem necessary to the
 * uninitiated.
 */

  void
archive_entry_acl_clear(struct archive_entry *entry)
{
  struct ae_acl *ap;

  while (entry->acl_head != NULL) {
    ap = entry->acl_head->next;
    aes_clean(&entry->acl_head->name);
    free(entry->acl_head);
    entry->acl_head = ap;
  }
  if (entry->acl_text_w != NULL) {
    free(entry->acl_text_w);
    entry->acl_text_w = NULL;
  }
  entry->acl_p = NULL;
  entry->acl_state = 0; /* Not counting. */
}

/*
 * Add a single ACL entry to the internal list of ACL data.
 */
  void
archive_entry_acl_add_entry(struct archive_entry *entry,
  int type, int permset, int tag, int id, const char *name)
{
  struct ae_acl *ap;

  if (acl_special(entry, type, permset, tag) == 0)
    return;
  ap = acl_new_entry(entry, type, permset, tag, id);
  if (ap == NULL) {
    /* XXX Error XXX */
    return;
  }
  if (name != NULL  &&  *name != '\0')
    aes_copy_mbs(&ap->name, name);
  else
    aes_clean(&ap->name);
}

/*
 * As above, but with a wide-character name.
 */
  void
archive_entry_acl_add_entry_w(struct archive_entry *entry,
  int type, int permset, int tag, int id, const wchar_t *name)
{
  struct ae_acl *ap;

  if (acl_special(entry, type, permset, tag) == 0)
    return;
  ap = acl_new_entry(entry, type, permset, tag, id);
  if (ap == NULL) {
    /* XXX Error XXX */
    return;
  }
  if (name != NULL  &&  *name != L'\0')
    aes_copy_wcs(&ap->name, name);
  else
    aes_clean(&ap->name);
}

/*
 * If this ACL entry is part of the standard POSIX permissions set,
 * store the permissions in the stat structure and return zero.
 */
  static int
acl_special(struct archive_entry *entry, int type, int permset, int tag)
{
  if (type == ARCHIVE_ENTRY_ACL_TYPE_ACCESS) {
    switch (tag) {
    case ARCHIVE_ENTRY_ACL_USER_OBJ:
      entry->ae_stat.st_mode &= ~0700;
      entry->ae_stat.st_mode |= (permset & 7) << 6;
      return (0);
    case ARCHIVE_ENTRY_ACL_GROUP_OBJ:
      entry->ae_stat.st_mode &= ~0070;
      entry->ae_stat.st_mode |= (permset & 7) << 3;
      return (0);
    case ARCHIVE_ENTRY_ACL_OTHER:
      entry->ae_stat.st_mode &= ~0007;
      entry->ae_stat.st_mode |= permset & 7;
      return (0);
    }
  }
  return (1);
}

/*
 * Allocate and populate a new ACL entry with everything but the
 * name.
 */
  static struct ae_acl *
acl_new_entry(struct archive_entry *entry,
  int type, int permset, int tag, int id)
{
  struct ae_acl *ap;

  if (type != ARCHIVE_ENTRY_ACL_TYPE_ACCESS &&
    type != ARCHIVE_ENTRY_ACL_TYPE_DEFAULT)
    return (NULL);
  if (entry->acl_text_w != NULL) {
    free(entry->acl_text_w);
    entry->acl_text_w = NULL;
  }

  /* XXX TODO: More sanity-checks on the arguments XXX */

  /* If there's a matching entry already in the list, overwrite it. */
  for (ap = entry->acl_head; ap != NULL; ap = ap->next) {
    if (ap->type == type && ap->tag == tag && ap->id == id) {
      ap->permset = permset;
      return (ap);
    }
  }

  /* Add a new entry to the list. */
  ap = (struct ae_acl *)malloc(sizeof(*ap));
  if (ap == NULL)
    return (NULL);
  memset(ap, 0, sizeof(*ap));
  ap->next = entry->acl_head;
  entry->acl_head = ap;
  ap->type = type;
  ap->tag = tag;
  ap->id = id;
  ap->permset = permset;
  return (ap);
}

/*
 * Return a count of entries matching "want_type".
 */
  int
archive_entry_acl_count(struct archive_entry *entry, int want_type)
{
  int count;
  struct ae_acl *ap;

  count = 0;
  ap = entry->acl_head;
  while (ap != NULL) {
    if ((ap->type & want_type) != 0)
      count++;
    ap = ap->next;
  }

  if (count > 0 && ((want_type & ARCHIVE_ENTRY_ACL_TYPE_ACCESS) != 0))
    count += 3;
  return (count);
}

/*
 * Prepare for reading entries from the ACL data.  Returns a count
 * of entries matching "want_type", or zero if there are no
 * non-extended ACL entries of that type.
 */
  int
archive_entry_acl_reset(struct archive_entry *entry, int want_type)
{
  int count, cutoff;

  count = archive_entry_acl_count(entry, want_type);

  /*
   * If the only entries are the three standard ones,
   * then don't return any ACL data.  (In this case,
   * client can just use chmod(2) to set permissions.)
   */
  if ((want_type & ARCHIVE_ENTRY_ACL_TYPE_ACCESS) != 0)
    cutoff = 3;
  else
    cutoff = 0;

  if (count > cutoff)
    entry->acl_state = ARCHIVE_ENTRY_ACL_USER_OBJ;
  else
    entry->acl_state = 0;
  entry->acl_p = entry->acl_head;
  return (count);
}

/*
 * Return the next ACL entry in the list.  Fake entries for the
 * standard permissions and include them in the returned list.
 */

  int
archive_entry_acl_next(struct archive_entry *entry, int want_type, int *type,
  int *permset, int *tag, int *id, const char **name)
{
  *name = NULL;
  *id = -1;

  /*
   * The acl_state is either zero (no entries available), -1
   * (reading from list), or an entry type (retrieve that type
   * from ae_stat.st_mode).
   */
  if (entry->acl_state == 0)
    return (ARCHIVE_WARN);

  /* The first three access entries are special. */
  if ((want_type & ARCHIVE_ENTRY_ACL_TYPE_ACCESS) != 0) {
    switch (entry->acl_state) {
    case ARCHIVE_ENTRY_ACL_USER_OBJ:
      *permset = (entry->ae_stat.st_mode >> 6) & 7;
      *type = ARCHIVE_ENTRY_ACL_TYPE_ACCESS;
      *tag = ARCHIVE_ENTRY_ACL_USER_OBJ;
      entry->acl_state = ARCHIVE_ENTRY_ACL_GROUP_OBJ;
      return (ARCHIVE_OK);
    case ARCHIVE_ENTRY_ACL_GROUP_OBJ:
      *permset = (entry->ae_stat.st_mode >> 3) & 7;
      *type = ARCHIVE_ENTRY_ACL_TYPE_ACCESS;
      *tag = ARCHIVE_ENTRY_ACL_GROUP_OBJ;
      entry->acl_state = ARCHIVE_ENTRY_ACL_OTHER;
      return (ARCHIVE_OK);
    case ARCHIVE_ENTRY_ACL_OTHER:
      *permset = entry->ae_stat.st_mode & 7;
      *type = ARCHIVE_ENTRY_ACL_TYPE_ACCESS;
      *tag = ARCHIVE_ENTRY_ACL_OTHER;
      entry->acl_state = -1;
      entry->acl_p = entry->acl_head;
      return (ARCHIVE_OK);
    default:
      break;
    }
  }

  while (entry->acl_p != NULL && (entry->acl_p->type & want_type) == 0)
    entry->acl_p = entry->acl_p->next;
  if (entry->acl_p == NULL) {
    entry->acl_state = 0;
    return (ARCHIVE_WARN);
  }
  *type = entry->acl_p->type;
  *permset = entry->acl_p->permset;
  *tag = entry->acl_p->tag;
  *id = entry->acl_p->id;
  *name = aes_get_mbs(&entry->acl_p->name);
  entry->acl_p = entry->acl_p->next;
  return (ARCHIVE_OK);
}

/*
 * Generate a text version of the ACL.  The flags parameter controls
 * the style of the generated ACL.
 */
  const wchar_t *
archive_entry_acl_text_w(struct archive_entry *entry, int flags)
{
  int count;
  int length;
  const wchar_t *wname;
  const wchar_t *prefix;
  wchar_t separator;
  struct ae_acl *ap;
  int id;
  wchar_t *wp;

  if (entry->acl_text_w != NULL) {
    free (entry->acl_text_w);
    entry->acl_text_w = NULL;
  }

  separator = L',';
  count = 0;
  length = 0;
  ap = entry->acl_head;
  while (ap != NULL) {
    if ((ap->type & flags) != 0) {
      count++;
      if ((flags & ARCHIVE_ENTRY_ACL_STYLE_MARK_DEFAULT) &&
        (ap->type & ARCHIVE_ENTRY_ACL_TYPE_DEFAULT))
        length += 8; /* "default:" */
      length += 5; /* tag name */
      length += 1; /* colon */
      wname = aes_get_wcs(&ap->name);
      if (wname != NULL)
        length += wcslen(wname);
      length ++; /* colon */
      length += 3; /* rwx */
      length += 1; /* colon */
      length += max(sizeof(uid_t),sizeof(gid_t)) * 3 + 1;
      length ++; /* newline */
    }
    ap = ap->next;
  }

  if (count > 0 && ((flags & ARCHIVE_ENTRY_ACL_TYPE_ACCESS) != 0)) {
    length += 10; /* "user::rwx\n" */
    length += 11; /* "group::rwx\n" */
    length += 11; /* "other::rwx\n" */
  }

  if (count == 0)
    return (NULL);

  /* Now, allocate the string and actually populate it. */
  wp = entry->acl_text_w = (wchar_t*)malloc(length * sizeof(wchar_t));
  if (wp == NULL)
    __archive_errx(1, "No memory to generate the text version of the ACL");
  count = 0;
  if ((flags & ARCHIVE_ENTRY_ACL_TYPE_ACCESS) != 0) {
    append_entry_w(&wp, NULL, ARCHIVE_ENTRY_ACL_USER_OBJ, NULL,
      entry->ae_stat.st_mode & 0700, -1);
    *wp++ = ',';
    append_entry_w(&wp, NULL, ARCHIVE_ENTRY_ACL_GROUP_OBJ, NULL,
      entry->ae_stat.st_mode & 0070, -1);
    *wp++ = ',';
    append_entry_w(&wp, NULL, ARCHIVE_ENTRY_ACL_OTHER, NULL,
      entry->ae_stat.st_mode & 0007, -1);
    count += 3;

    ap = entry->acl_head;
    while (ap != NULL) {
      if ((ap->type & ARCHIVE_ENTRY_ACL_TYPE_ACCESS) != 0) {
        wname = aes_get_wcs(&ap->name);
        *wp++ = separator;
        if (flags & ARCHIVE_ENTRY_ACL_STYLE_EXTRA_ID)
          id = ap->id;
        else
          id = -1;
        append_entry_w(&wp, NULL, ap->tag, wname,
          ap->permset, id);
        count++;
      }
      ap = ap->next;
    }
  }


  if ((flags & ARCHIVE_ENTRY_ACL_TYPE_DEFAULT) != 0) {
    if (flags & ARCHIVE_ENTRY_ACL_STYLE_MARK_DEFAULT)
      prefix = L"default:";
    else
      prefix = NULL;
    ap = entry->acl_head;
    count = 0;
    while (ap != NULL) {
      if ((ap->type & ARCHIVE_ENTRY_ACL_TYPE_DEFAULT) != 0) {
        wname = aes_get_wcs(&ap->name);
        if (count > 0)
          *wp++ = separator;
        if (flags & ARCHIVE_ENTRY_ACL_STYLE_EXTRA_ID)
          id = ap->id;
        else
          id = -1;
        append_entry_w(&wp, prefix, ap->tag,
          wname, ap->permset, id);
        count ++;
      }
      ap = ap->next;
    }
  }

  return (entry->acl_text_w);
}

  static void
append_id_w(wchar_t **wp, int id)
{
  if (id > 9)
    append_id_w(wp, id / 10);
  *(*wp)++ = L"0123456789"[id % 10];
}

  static void
append_entry_w(wchar_t **wp, const wchar_t *prefix, int tag,
  const wchar_t *wname, int perm, int id)
{
  if (prefix != NULL) {
    wcscpy(*wp, prefix);
    *wp += wcslen(*wp);
  }
  switch (tag) {
  case ARCHIVE_ENTRY_ACL_USER_OBJ:
    wname = NULL;
    id = -1;
    /* FALL THROUGH */
  case ARCHIVE_ENTRY_ACL_USER:
    wcscpy(*wp, L"user");
    break;
  case ARCHIVE_ENTRY_ACL_GROUP_OBJ:
    wname = NULL;
    id = -1;
    /* FALL THROUGH */
  case ARCHIVE_ENTRY_ACL_GROUP:
    wcscpy(*wp, L"group");
    break;
  case ARCHIVE_ENTRY_ACL_MASK:
    wcscpy(*wp, L"mask");
    wname = NULL;
    id = -1;
    break;
  case ARCHIVE_ENTRY_ACL_OTHER:
    wcscpy(*wp, L"other");
    wname = NULL;
    id = -1;
    break;
  }
  *wp += wcslen(*wp);
  *(*wp)++ = L':';
  if (wname != NULL) {
    wcscpy(*wp, wname);
    *wp += wcslen(*wp);
  }
  *(*wp)++ = L':';
  *(*wp)++ = (perm & 0444) ? L'r' : L'-';
  *(*wp)++ = (perm & 0222) ? L'w' : L'-';
  *(*wp)++ = (perm & 0111) ? L'x' : L'-';
  if (id != -1) {
    *(*wp)++ = L':';
    append_id_w(wp, id);
  }
  **wp = L'\0';
}

/*
 * Parse a textual ACL.  This automatically recognizes and supports
 * extensions described above.  The 'type' argument is used to
 * indicate the type that should be used for any entries not
 * explicitly marked as "default:".
 */
  int
__archive_entry_acl_parse_w(struct archive_entry *entry,
  const wchar_t *text, int default_type)
{
  int type, tag, permset, id;
  const wchar_t *start, *end;
  const wchar_t *name_start, *name_end;
  wchar_t sep;
  wchar_t *namebuff;
  int namebuff_length;

  name_start = name_end = NULL;
  namebuff = NULL;
  namebuff_length = 0;

  while (text != NULL  &&  *text != L'\0') {
    next_field_w(&text, &start, &end, &sep);
    if (sep != L':')
      goto fail;

    /*
     * Solaris extension:  "defaultuser::rwx" is the
     * default ACL corresponding to "user::rwx", etc.
     */
    if (end-start > 7  && wmemcmp(start, L"default", 7) == 0) {
      type = ARCHIVE_ENTRY_ACL_TYPE_DEFAULT;
      start += 7;
    } else
      type = default_type;

    if (prefix_w(start, end, L"user")) {
      next_field_w(&text, &start, &end, &sep);
      if (sep != L':')
        goto fail;
      if (end > start) {
        tag = ARCHIVE_ENTRY_ACL_USER;
        name_start = start;
        name_end = end;
      } else
        tag = ARCHIVE_ENTRY_ACL_USER_OBJ;
    } else if (prefix_w(start, end, L"group")) {
      next_field_w(&text, &start, &end, &sep);
      if (sep != L':')
        goto fail;
      if (end > start) {
        tag = ARCHIVE_ENTRY_ACL_GROUP;
        name_start = start;
        name_end = end;
      } else
        tag = ARCHIVE_ENTRY_ACL_GROUP_OBJ;
    } else if (prefix_w(start, end, L"other")) {
      next_field_w(&text, &start, &end, &sep);
      if (sep != L':')
        goto fail;
      if (end > start)
        goto fail;
      tag = ARCHIVE_ENTRY_ACL_OTHER;
    } else if (prefix_w(start, end, L"mask")) {
      next_field_w(&text, &start, &end, &sep);
      if (sep != L':')
        goto fail;
      if (end > start)
        goto fail;
      tag = ARCHIVE_ENTRY_ACL_MASK;
    } else
      goto fail;

    next_field_w(&text, &start, &end, &sep);
    permset = 0;
    while (start < end) {
      switch (*start++) {
      case 'r': case 'R':
        permset |= ARCHIVE_ENTRY_ACL_READ;
        break;
      case 'w': case 'W':
        permset |= ARCHIVE_ENTRY_ACL_WRITE;
        break;
      case 'x': case 'X':
        permset |= ARCHIVE_ENTRY_ACL_EXECUTE;
        break;
      case '-':
        break;
      default:
        goto fail;
      }
    }

    /*
     * Support star-compatible numeric UID/GID extension.
     * This extension adds a ":" followed by the numeric
     * ID so that "group:groupname:rwx", for example,
     * becomes "group:groupname:rwx:999", where 999 is the
     * numeric GID.  This extension makes it possible, for
     * example, to correctly restore ACLs on a system that
     * might have a damaged passwd file or be disconnected
     * from a central NIS server.  This extension is compatible
     * with POSIX.1e draft 17.
     */
    if (sep == L':' && (tag == ARCHIVE_ENTRY_ACL_USER ||
        tag == ARCHIVE_ENTRY_ACL_GROUP)) {
      next_field_w(&text, &start, &end, &sep);

      id = 0;
      while (start < end  && *start >= '0' && *start <= '9') {
        if (id > (INT_MAX / 10))
          id = INT_MAX;
        else {
          id *= 10;
          id += *start - '0';
          start++;
        }
      }
    } else
          id = -1; /* No id specified. */

    /* Skip any additional entries. */
    while (sep == L':') {
      next_field_w(&text, &start, &end, &sep);
    }

    /* Add entry to the internal list. */
    if (name_end == name_start) {
      archive_entry_acl_add_entry_w(entry, type, permset,
        tag, id, NULL);
    } else {
      if (namebuff_length <= name_end - name_start) {
        if (namebuff != NULL)
          free(namebuff);
        namebuff_length = name_end - name_start + 256;
        namebuff =
          (wchar_t*)malloc(namebuff_length * sizeof(wchar_t));
        if (namebuff == NULL)
          goto fail;
      }
      wmemcpy(namebuff, name_start, name_end - name_start);
      namebuff[name_end - name_start] = L'\0';
      archive_entry_acl_add_entry_w(entry, type,
        permset, tag, id, namebuff);
    }
  }
  if (namebuff != NULL)
    free(namebuff);
  return (ARCHIVE_OK);

fail:
  if (namebuff != NULL)
    free(namebuff);
  return (ARCHIVE_WARN);
}

/*
 * Match "[:whitespace:]*(.*)[:whitespace:]*[:,\n]".  *wp is updated
 * to point to just after the separator.  *start points to the first
 * character of the matched text and *end just after the last
 * character of the matched identifier.  In particular *end - *start
 * is the length of the field body, not including leading or trailing
 * whitespace.
 */
  static void
next_field_w(const wchar_t **wp, const wchar_t **start,
  const wchar_t **end, wchar_t *sep)
{
  /* Skip leading whitespace to find start of field. */
  while (**wp == L' ' || **wp == L'\t' || **wp == L'\n') {
    (*wp)++;
  }
  *start = *wp;

  /* Scan for the separator. */
  while (**wp != L'\0' && **wp != L',' && **wp != L':' &&
    **wp != L'\n') {
    (*wp)++;
  }
  *sep = **wp;

  /* Trim trailing whitespace to locate end of field. */
  *end = *wp - 1;
  while (**end == L' ' || **end == L'\t' || **end == L'\n') {
    (*end)--;
  }
  (*end)++;

  /* Adjust scanner location. */
  if (**wp != L'\0')
    (*wp)++;
}

  static int
prefix_w(const wchar_t *start, const wchar_t *end, const wchar_t *test)
{
  if (start == end)
    return (0);

  if (*start++ != *test++)
    return (0);

  while (start < end  &&  *start++ == *test++)
    ;

  if (start < end)
    return (0);

  return (1);
}


/*
 * Following code is modified from UC Berkeley sources, and
 * is subject to the following copyright notice.
 */

/*-
 * Copyright (c) 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static struct flag {
  const char *name;
  const wchar_t *wname;
  unsigned long  set;
  unsigned long  clear;
} flags[] = {
  /* Preferred (shorter) names per flag first, all prefixed by "no" */
#ifdef SF_APPEND
    { "nosappnd", L"nosappnd",  SF_APPEND, 0 },
    { "nosappend", L"nosappend",  SF_APPEND, 0 },
#endif
#ifdef  EXT2_APPEND_FL    /* 'a' */
    { "nosappnd", L"nosappnd",  EXT2_APPEND_FL, 0 },
    { "nosappend", L"nosappend",  EXT2_APPEND_FL, 0 },
#endif
#ifdef SF_ARCHIVED
    { "noarch", L"noarch",  SF_ARCHIVED, 0 },
    { "noarchived", L"noarchived",        SF_ARCHIVED, 0 },
#endif
#ifdef SF_IMMUTABLE
    { "noschg", L"noschg",  SF_IMMUTABLE, 0 },
    { "noschange", L"noschange",  SF_IMMUTABLE, 0 },
    { "nosimmutable", L"nosimmutable", SF_IMMUTABLE, 0 },
#endif
#ifdef EXT2_IMMUTABLE_FL   /* 'i' */
    { "noschg", L"noschg",  EXT2_IMMUTABLE_FL, 0 },
    { "noschange", L"noschange",  EXT2_IMMUTABLE_FL, 0 },
    { "nosimmutable", L"nosimmutable", EXT2_IMMUTABLE_FL, 0 },
#endif
#ifdef SF_NOUNLINK
    { "nosunlnk", L"nosunlnk",  SF_NOUNLINK, 0 },
    { "nosunlink", L"nosunlink",  SF_NOUNLINK, 0 },
#endif
#ifdef SF_SNAPSHOT
    { "nosnapshot", L"nosnapshot", SF_SNAPSHOT, 0 },
#endif
#ifdef UF_APPEND
    { "nouappnd", L"nouappnd",  UF_APPEND, 0 },
    { "nouappend", L"nouappend",  UF_APPEND, 0 },
#endif
#ifdef UF_IMMUTABLE
    { "nouchg", L"nouchg",  UF_IMMUTABLE, 0 },
    { "nouchange", L"nouchange",  UF_IMMUTABLE, 0 },
    { "nouimmutable", L"nouimmutable", UF_IMMUTABLE, 0 },
#endif
#ifdef UF_NODUMP
    { "nodump", L"nodump",  0,  UF_NODUMP},
#endif
#ifdef EXT2_NODUMP_FL    /* 'd' */
    { "nodump", L"nodump",  0,  EXT2_NODUMP_FL},
#endif
#ifdef UF_OPAQUE
    { "noopaque", L"noopaque",  UF_OPAQUE, 0 },
#endif
#ifdef UF_NOUNLINK
    { "nouunlnk", L"nouunlnk",  UF_NOUNLINK, 0 },
    { "nouunlink", L"nouunlink",  UF_NOUNLINK, 0 },
#endif
#ifdef EXT2_COMPR_FL    /* 'c' */
    { "nocompress", L"nocompress",        EXT2_COMPR_FL, 0 },
#endif

#ifdef EXT2_NOATIME_FL    /* 'A' */
    { "noatime", L"noatime",  0,  EXT2_NOATIME_FL},
#endif
    { NULL,  NULL,   0,  0 }
};

/*
 * fflagstostr --
 * Convert file flags to a comma-separated string.  If no flags
 * are set, return the empty string.
 */
  char *
ae_fflagstostr(unsigned long bitset, unsigned long bitclear)
{
  char *string, *dp;
  const char *sp;
  unsigned long bits;
  struct flag *flag;
  int length;

  bits = bitset | bitclear;
  length = 0;
  for (flag = flags; flag->name != NULL; flag++)
    if (bits & (flag->set | flag->clear)) {
      length += strlen(flag->name) + 1;
      bits &= ~(flag->set | flag->clear);
    }

  if (length == 0)
    return (NULL);
  string = (char*)malloc(length);
  if (string == NULL)
    return (NULL);

  dp = string;
  for (flag = flags; flag->name != NULL; flag++) {
    if (bitset & flag->set || bitclear & flag->clear) {
      sp = flag->name + 2;
    } else if (bitset & flag->clear  ||  bitclear & flag->set) {
      sp = flag->name;
    } else
      continue;
    bitset &= ~(flag->set | flag->clear);
    bitclear &= ~(flag->set | flag->clear);
    if (dp > string)
      *dp++ = ',';
    while ((*dp++ = *sp++) != '\0')
      ;
    dp--;
  }

  *dp = '\0';
  return (string);
}

/*
 * wcstofflags --
 * Take string of arguments and return file flags.  This
 * version works a little differently than strtofflags(3).
 * In particular, it always tests every token, skipping any
 * unrecognized tokens.  It returns a pointer to the first
 * unrecognized token, or NULL if every token was recognized.
 * This version is also const-correct and does not modify the
 * provided string.
 */
  const wchar_t *
ae_wcstofflags(const wchar_t *s, unsigned long *setp, unsigned long *clrp)
{
  const wchar_t *start, *end;
  struct flag *flag;
  unsigned long set, clear;
  const wchar_t *failed;

  set = clear = 0;
  start = s;
  failed = NULL;
  /* Find start of first token. */
  while (*start == L'\t'  ||  *start == L' '  ||  *start == L',')
    start++;
  while (*start != L'\0') {
    /* Locate end of token. */
    end = start;
    while (*end != L'\0'  &&  *end != L'\t'  &&
      *end != L' '  &&  *end != L',')
      end++;
    for (flag = flags; flag->wname != NULL; flag++) {
      if (wmemcmp(start, flag->wname, end - start) == 0) {
        /* Matched "noXXXX", so reverse the sense. */
        clear |= flag->set;
        set |= flag->clear;
        break;
      } else if (wmemcmp(start, flag->wname + 2, end - start)
        == 0) {
        /* Matched "XXXX", so don't reverse. */
        set |= flag->set;
        clear |= flag->clear;
        break;
      }
    }
    /* Ignore unknown flag names. */
    if (flag->wname == NULL  &&  failed == NULL)
      failed = start;

    /* Find start of next token. */
    start = end;
    while (*start == L'\t'  ||  *start == L' '  ||  *start == L',')
      start++;

  }

  if (setp)
    *setp = set;
  if (clrp)
    *clrp = clear;

  /* Return location of first failure. */
  return (failed);
}

//--- archive_string_sprintf.c
/*
 * Like 'vsprintf', but ensures the target is big enough, resizing if
 * necessary.
 */
  void
__archive_string_vsprintf(struct archive_string *as, const char *fmt,
  va_list ap)
{
  char long_flag;
  intmax_t s; /* Signed integer temp. */
  uintmax_t u; /* Unsigned integer temp. */
  const char *p, *p2;

  __archive_string_ensure(as, 64);

  if (fmt == NULL) {
    as->s[0] = 0;
    return;
  }

  long_flag = '\0';
  for (p = fmt; *p != '\0'; p++) {
    const char *saved_p = p;

    if (*p != '%') {
      archive_strappend_char(as, *p);
      continue;
    }

    p++;

    switch(*p) {
    case 'j':
      long_flag = 'j';
      p++;
      break;
    case 'l':
      long_flag = 'l';
      p++;
      break;
    }

    switch (*p) {
    case '%':
      __archive_strappend_char(as, '%');
      break;
    case 'c':
      s = va_arg(ap, int);
      __archive_strappend_char(as, s);
      break;
    case 'd':
      switch(long_flag) {
      case 'j': s = va_arg(ap, intmax_t); break;
      case 'l': s = va_arg(ap, long); break;
      default:  s = va_arg(ap, int); break;
      }
      archive_strappend_int(as, s, 10);
      break;
    case 's':
      p2 = va_arg(ap, char *);
      archive_strcat(as, p2);
      break;
    case 'o': case 'u': case 'x': case 'X':
      /* Common handling for unsigned integer formats. */
      switch(long_flag) {
      case 'j': u = va_arg(ap, uintmax_t); break;
      case 'l': u = va_arg(ap, unsigned long); break;
      default:  u = va_arg(ap, unsigned int); break;
      }
      /* Format it in the correct base. */
      switch (*p) {
      case 'o': archive_strappend_int(as, u, 8); break;
      case 'u': archive_strappend_int(as, u, 10); break;
      default: archive_strappend_int(as, u, 16); break;
      }
      break;
    default:
      /* Rewind and print the initial '%' literally. */
      p = saved_p;
      archive_strappend_char(as, *p);
    }
  }
}

//----------------------------------------------------------------------
class cmCPackTGZ_Data
{
public:
  cmCPackTGZ_Data(cmCPackTGZGenerator* gen) :
    Name(0), OutputStream(0), Generator(gen) {}
  const char *Name;
  std::ostream* OutputStream;
  cmCPackTGZGenerator* Generator;
};

//----------------------------------------------------------------------
int cmCPackTGZGenerator::TGZ_Open(struct archive *a, void *client_data)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  (void)a;
  mydata->OutputStream = new cmGeneratedFileStream(mydata->Name);
  if ( *mydata->OutputStream &&
    mydata->Generator->GenerateHeader(mydata->OutputStream))
    {
    return (ARCHIVE_OK);
    }
  else
    {
    return (ARCHIVE_FATAL);
    }
}

//----------------------------------------------------------------------
ssize_t cmCPackTGZGenerator::TGZ_Write(struct archive *a, void *client_data, void *buff, size_t n)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  (void)a;
  mydata->OutputStream->write(reinterpret_cast<const char*>(buff), n);
  if ( !*mydata->OutputStream )
    {
    return 0;
    }
  return n;
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::TGZ_Close(struct archive *a, void *client_data)
{
  cmCPackTGZ_Data *mydata = (cmCPackTGZ_Data*)client_data;

  (void)a;
  delete mydata->OutputStream;
  return (0);
}

//----------------------------------------------------------------------
int cmCPackTGZGenerator::CompressFiles(const char* outFileName, const char* toplevel,
  const std::vector<std::string>& files)
{
  std::cout << "Toplevel: " << toplevel << std::endl;
  cmCPackTGZ_Data mydata(this);
  struct archive *a;
  struct archive_entry *entry;
  struct stat st;
  char buff[8192];
  int len;
  int fd;

  a = archive_write_new();
  mydata.Name = outFileName;
  archive_write_set_compression_gzip(a);
  archive_write_set_format_ustar(a);
  archive_write_open(a, &mydata, cmCPackTGZGenerator::TGZ_Open,
    cmCPackTGZGenerator::TGZ_Write, cmCPackTGZGenerator::TGZ_Close);
  std::vector<std::string>::const_iterator fileIt;
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    std::string fname = cmSystemTools::RelativePath(toplevel, fileIt->c_str());
    const char* filename = fileIt->c_str();
    stat(filename, &st);
    entry = archive_entry_new();
    archive_entry_copy_stat(entry, &st);
    archive_entry_set_pathname(entry, fname.c_str());
    archive_write_header(a, entry);
    fd = open(filename, O_RDONLY);
    len = read(fd, buff, sizeof(buff));
    while ( len > 0 )
      {
      archive_write_data(a, buff, len);
      len = read(fd, buff, sizeof(buff));
      }
    archive_entry_free(entry);
    }
  archive_write_finish(a);
  return 1;
}

//--- archive_write_set_format_ustar.c
struct ustar {
  uint64_t entry_bytes_remaining;
  uint64_t entry_padding;
  char  written;
};

/*
 * Define structure of POSIX 'ustar' tar header.
 */
struct archive_entry_header_ustar {
  char name[100];
  char mode[6];
  char mode_padding[2];
  char uid[6];
  char uid_padding[2];
  char gid[6];
  char gid_padding[2];
  char size[11];
  char size_padding[1];
  char mtime[11];
  char mtime_padding[1];
  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6]; /* For POSIX: "ustar\0" */
  char version[2]; /* For POSIX: "00" */
  char uname[32];
  char gname[32];
  char rdevmajor[6];
  char rdevmajor_padding[2];
  char rdevminor[6];
  char rdevminor_padding[2];
  char prefix[155];
  char padding[12];
};

/*
 * A filled-in copy of the header for initialization.
 */
static const struct archive_entry_header_ustar template_header = {
    { "" },    /* name */
    { '0','0','0','0','0','0' }, { ' ', '\0' }, /* mode, space-null termination. */
    { '0','0','0','0','0','0' }, { ' ', '\0' }, /* uid, space-null termination. */
    { '0','0','0','0','0','0' }, { ' ', '\0' }, /* gid, space-null termination. */
    { '0','0','0','0','0','0','0','0','0','0','0' }, { ' ' }, /* size, space termination. */
    { '0','0','0','0','0','0','0','0','0','0','0' }, { ' ' }, /* mtime, space termination. */
    { ' ',' ',' ',' ',' ',' ',' ',' ' },   /* Initial checksum value. */
    { '0' },   /* default: regular file */
    { "" },    /* linkname */
    { 'u','s','t','a','r' },   /* magic */
    { '0', '0' },   /* version */
    { "" },    /* uname */
    { "" },    /* gname */
    {'0','0','0','0','0','0'}, { ' ', '\0' }, /* rdevmajor, space-null termination */
    {'0','0','0','0','0','0'}, { ' ', '\0' }, /* rdevminor, space-null termination */
    { "" },    /* prefix */
    { "" }    /* padding */
};

static int archive_write_ustar_data(struct archive *a, const void *buff,
  size_t s);
static int archive_write_ustar_finish(struct archive *);
static int archive_write_ustar_finish_entry(struct archive *);
static int archive_write_ustar_header(struct archive *,
  struct archive_entry *entry);
static int format_256(int64_t, char *, int);
static int format_number(int64_t, char *, int size, int max, int strict);
static int format_octal(int64_t, char *, int);
static int write_nulls(struct archive *a, size_t);

/*
 * Set output format to 'ustar' format.
 */
  int
archive_write_set_format_ustar(struct archive *a)
{
  struct ustar *ustar;

  /* If someone else was already registered, unregister them. */
  if (a->format_finish != NULL)
    (a->format_finish)(a);

  ustar = (struct ustar*)malloc(sizeof(*ustar));
  if (ustar == NULL) {
    archive_set_error(a, ENOMEM, "Can't allocate ustar data");
    return (ARCHIVE_FATAL);
  }
  memset(ustar, 0, sizeof(*ustar));
  a->format_data = ustar;

  a->pad_uncompressed = 1; /* Mimic gtar in this respect. */
  a->format_write_header = archive_write_ustar_header;
  a->format_write_data = archive_write_ustar_data;
  a->format_finish = archive_write_ustar_finish;
  a->format_finish_entry = archive_write_ustar_finish_entry;
  a->archive_format = ARCHIVE_FORMAT_TAR_USTAR;
  a->archive_format_name = "POSIX ustar";
  return (ARCHIVE_OK);
}

  static int
archive_write_ustar_header(struct archive *a, struct archive_entry *entry)
{
  char buff[512];
  int ret;
  struct ustar *ustar;

  ustar = (struct ustar*)a->format_data;
  ustar->written = 1;

  /* Only regular files (not hardlinks) have data. */
  if (archive_entry_hardlink(entry) != NULL ||
    archive_entry_symlink(entry) != NULL ||
    !S_ISREG(archive_entry_mode(entry)))
    archive_entry_set_size(entry, 0);

  ret = __archive_write_format_header_ustar(a, buff, entry, -1, 1);
  if (ret != ARCHIVE_OK)
    return (ret);
  ret = (a->compression_write)(a, buff, 512);
  if (ret != ARCHIVE_OK)
    return (ret);

  ustar->entry_bytes_remaining = archive_entry_size(entry);
  ustar->entry_padding = 0x1ff & (- ustar->entry_bytes_remaining);
  return (ARCHIVE_OK);
}

/*
 * Format a basic 512-byte "ustar" header.
 *
 * Returns -1 if format failed (due to field overflow).
 * Note that this always formats as much of the header as possible.
 * If "strict" is set to zero, it will extend numeric fields as
 * necessary (overwriting terminators or using base-256 extensions).
 *
 * This is exported so that other 'tar' formats can use it.
 */
  int
__archive_write_format_header_ustar(struct archive *a, char buff[512],
  struct archive_entry *entry, int tartype, int strict)
{
  unsigned int checksum;
  struct archive_entry_header_ustar *h;
  int i, ret;
  size_t copy_length;
  const char *p, *pp;
  const struct stat *st;
  int mytartype;

  ret = 0;
  mytartype = -1;
  /*
   * The "template header" already includes the "ustar"
   * signature, various end-of-field markers and other required
   * elements.
   */
  memcpy(buff, &template_header, 512);

  h = (struct archive_entry_header_ustar *)buff;

  /*
   * Because the block is already null-filled, and strings
   * are allowed to exactly fill their destination (without null),
   * I use memcpy(dest, src, strlen()) here a lot to copy strings.
   */

  pp = archive_entry_pathname(entry);
  if (strlen(pp) <= sizeof(h->name))
    memcpy(h->name, pp, strlen(pp));
  else {
    /* Store in two pieces, splitting at a '/'. */
    p = strchr(pp + strlen(pp) - sizeof(h->name) - 1, '/');
    /*
     * If there is no path separator, or the prefix or
     * remaining name are too large, return an error.
     */
    if (!p) {
      archive_set_error(a, ENAMETOOLONG,
        "Pathname too long");
      ret = ARCHIVE_WARN;
    } else if (p  > pp + sizeof(h->prefix)) {
      archive_set_error(a, ENAMETOOLONG,
        "Pathname too long");
      ret = ARCHIVE_WARN;
    } else {
      /* Copy prefix and remainder to appropriate places */
      memcpy(h->prefix, pp, p - pp);
      memcpy(h->name, p + 1, pp + strlen(pp) - p - 1);
    }
  }

  p = archive_entry_hardlink(entry);
  if (p != NULL)
    mytartype = '1';
  else
    p = archive_entry_symlink(entry);
  if (p != NULL && p[0] != '\0') {
    copy_length = strlen(p);
    if (copy_length > sizeof(h->linkname)) {
      archive_set_error(a, ENAMETOOLONG,
        "Link contents too long");
      ret = ARCHIVE_WARN;
      copy_length = sizeof(h->linkname);
    }
    memcpy(h->linkname, p, copy_length);
  }

  p = archive_entry_uname(entry);
  if (p != NULL && p[0] != '\0') {
    copy_length = strlen(p);
    if (copy_length > sizeof(h->uname)) {
      archive_set_error(a, ARCHIVE_ERRNO_MISC,
        "Username too long");
      ret = ARCHIVE_WARN;
      copy_length = sizeof(h->uname);
    }
    memcpy(h->uname, p, copy_length);
  }

  p = archive_entry_gname(entry);
  if (p != NULL && p[0] != '\0') {
    copy_length = strlen(p);
    if (strlen(p) > sizeof(h->gname)) {
      archive_set_error(a, ARCHIVE_ERRNO_MISC,
        "Group name too long");
      ret = ARCHIVE_WARN;
      copy_length = sizeof(h->gname);
    }
    memcpy(h->gname, p, copy_length);
  }

  st = archive_entry_stat(entry);

  if (format_number(st->st_mode & 07777, h->mode, sizeof(h->mode), 8, strict)) {
    archive_set_error(a, ERANGE, "Numeric mode too large");
    ret = ARCHIVE_WARN;
  }

  if (format_number(st->st_uid, h->uid, sizeof(h->uid), 8, strict)) {
    archive_set_error(a, ERANGE, "Numeric user ID too large");
    ret = ARCHIVE_WARN;
  }

  if (format_number(st->st_gid, h->gid, sizeof(h->gid), 8, strict)) {
    archive_set_error(a, ERANGE, "Numeric group ID too large");
    ret = ARCHIVE_WARN;
  }

  if (format_number(st->st_size, h->size, sizeof(h->size), 12, strict)) {
    archive_set_error(a, ERANGE, "File size out of range");
    ret = ARCHIVE_WARN;
  }

  if (format_number(st->st_mtime, h->mtime, sizeof(h->mtime), 12, strict)) {
    archive_set_error(a, ERANGE,
      "File modification time too large");
    ret = ARCHIVE_WARN;
  }

#if defined(S_ISBLK)
  if (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)) {
    if (format_number(major(st->st_rdev), h->rdevmajor,
        sizeof(h->rdevmajor), 8, strict)) {
      archive_set_error(a, ERANGE,
        "Major device number too large");
      ret = ARCHIVE_WARN;
    }

    if (format_number(minor(st->st_rdev), h->rdevminor,
        sizeof(h->rdevminor), 8, strict)) {
      archive_set_error(a, ERANGE,
        "Minor device number too large");
      ret = ARCHIVE_WARN;
    }
  }
#endif

  if (tartype >= 0) {
    h->typeflag[0] = tartype;
  } else if (mytartype >= 0) {
    h->typeflag[0] = mytartype;
  } else {
    switch (st->st_mode & S_IFMT) {
    case S_IFREG: h->typeflag[0] = '0' ; break;
#if defined(S_IFLNK)
    case S_IFLNK: h->typeflag[0] = '2' ; break;
#endif
    case S_IFCHR: h->typeflag[0] = '3' ; break;
#if defined(S_IFBLK)
    case S_IFBLK: h->typeflag[0] = '4' ; break;
#endif
    case S_IFDIR: h->typeflag[0] = '5' ; break;
#if defined(S_IFIFO)
    case S_IFIFO: h->typeflag[0] = '6' ; break;
#endif
#if defined(S_IFSOCK)
    case S_IFSOCK:
                  archive_set_error(a, ARCHIVE_ERRNO_FILE_FORMAT,
                    "tar format cannot archive socket");
                  ret = ARCHIVE_WARN;
                  break;
#endif
    default:
                  archive_set_error(a, ARCHIVE_ERRNO_FILE_FORMAT,
                    "tar format cannot archive this (mode=0%lo)",
                    (unsigned long)st->st_mode);
                  ret = ARCHIVE_WARN;
    }
  }

  checksum = 0;
  for (i = 0; i < 512; i++)
    checksum += 255 & (unsigned int)buff[i];
  h->checksum[6] = '\0'; /* Can't be pre-set in the template. */
  /* h->checksum[7] = ' '; */ /* This is pre-set in the template. */
  format_octal(checksum, h->checksum, 6);
  return (ret);
}

/*
 * Format a number into a field, with some intelligence.
 */
  static int
format_number(int64_t v, char *p, int s, int maxsize, int strict)
{
  int64_t limit;

  limit = ((int64_t)1 << (s*3));

  /* "Strict" only permits octal values with proper termination. */
  if (strict)
    return (format_octal(v, p, s));

  /*
   * In non-strict mode, we allow the number to overwrite one or
   * more bytes of the field termination.  Even old tar
   * implementations should be able to handle this with no
   * problem.
   */
  if (v >= 0) {
    while (s <= maxsize) {
      if (v < limit)
        return (format_octal(v, p, s));
      s++;
      limit <<= 3;
    }
  }

  /* Base-256 can handle any number, positive or negative. */
  return (format_256(v, p, maxsize));
}

/*
 * Format a number into the specified field using base-256.
 */
  static int
format_256(int64_t v, char *p, int s)
{
  p += s;
  while (s-- > 0) {
    *--p = (char)(v & 0xff);
    v >>= 8;
  }
  *p |= 0x80; /* Set the base-256 marker bit. */
  return (0);
}

/*
 * Format a number into the specified field.
 */
  static int
format_octal(int64_t v, char *p, int s)
{
  int len;

  len = s;

  /* Octal values can't be negative, so use 0. */
  if (v < 0) {
    while (len-- > 0)
      *p++ = '0';
    return (-1);
  }

  p += s;  /* Start at the end and work backwards. */
  while (s-- > 0) {
    *--p = '0' + (v & 7);
    v >>= 3;
  }

  if (v == 0)
    return (0);

  /* If it overflowed, fill field with max value. */
  while (len-- > 0)
    *p++ = '7';

  return (-1);
}

  static int
archive_write_ustar_finish(struct archive *a)
{
  struct ustar *ustar;
  int r;

  r = ARCHIVE_OK;
  ustar = (struct ustar*)a->format_data;
  /*
   * Suppress end-of-archive if nothing else was ever written.
   * This fixes a problem where setting one format, then another
   * ends up writing a gratuitous end-of-archive marker.
   */
  if (ustar->written && a->compression_write != NULL)
    r = write_nulls(a, 512*2);
  free(ustar);
  a->format_data = NULL;
  return (r);
}

  static int
archive_write_ustar_finish_entry(struct archive *a)
{
  struct ustar *ustar;
  int ret;

  ustar = (struct ustar*) a->format_data;
  ret = write_nulls(a,
    ustar->entry_bytes_remaining + ustar->entry_padding);
  ustar->entry_bytes_remaining = ustar->entry_padding = 0;
  return (ret);
}

  static int
write_nulls(struct archive *a, size_t padding)
{
  int ret, to_write;

  while (padding > 0) {
    to_write = padding < a->null_length ? padding : a->null_length;
    ret = (a->compression_write)(a, a->nulls, to_write);
    if (ret != ARCHIVE_OK)
      return (ret);
    padding -= to_write;
  }
  return (ARCHIVE_OK);
}

  static int
archive_write_ustar_data(struct archive *a, const void *buff, size_t s)
{
  struct ustar *ustar;
  int ret;

  ustar = (struct ustar*)a->format_data;
  if (s > ustar->entry_bytes_remaining)
    s = ustar->entry_bytes_remaining;
  ret = (a->compression_write)(a, buff, s);
  ustar->entry_bytes_remaining -= s;
  return (ret);
}

//--- archive_write_set_compression_gzip.c

struct private_data {
  z_stream  stream;
  int64_t   total_in;
  unsigned char *compressed;
  size_t   compressed_buffer_size;
  unsigned long  crc;
};


/*
 * Yuck.  zlib.h is not const-correct, so I need this one bit
 * of ugly hackery to convert a const * pointer to a non-const pointer.
 */
#define SET_NEXT_IN(st,src)     \
  (st)->stream.next_in = (Bytef*)(void *)(uintptr_t)(const void *)(src)

static int archive_compressor_gzip_finish(struct archive *);
static int archive_compressor_gzip_init(struct archive *);
static int archive_compressor_gzip_write(struct archive *, const void *,
  size_t);
static int drive_compressor(struct archive *, struct private_data *,
  int finishing);


/*
 * Allocate, initialize and return a archive object.
 */
  int
archive_write_set_compression_gzip(struct archive *a)
{
  __archive_check_magic(a, ARCHIVE_WRITE_MAGIC, ARCHIVE_STATE_NEW, "archive_write_set_compression_gzip");
  a->compression_init = &archive_compressor_gzip_init;
  a->compression_code = ARCHIVE_COMPRESSION_GZIP;
  a->compression_name = "gzip";
  return (ARCHIVE_OK);
}

/*
 * Setup callback.
 */
  static int
archive_compressor_gzip_init(struct archive *a)
{
  int ret;
  struct private_data *state;
  time_t t;

  a->compression_code = ARCHIVE_COMPRESSION_GZIP;
  a->compression_name = "gzip";

  if (a->client_opener != NULL) {
    ret = (a->client_opener)(a, a->client_data);
    if (ret != ARCHIVE_OK)
      return (ret);
  }

  state = (struct private_data *)malloc(sizeof(*state));
  if (state == NULL) {
    archive_set_error(a, ENOMEM,
      "Can't allocate data for compression");
    return (ARCHIVE_FATAL);
  }
  memset(state, 0, sizeof(*state));

  state->compressed_buffer_size = a->bytes_per_block;
  state->compressed = (unsigned char*)malloc(state->compressed_buffer_size);
  state->crc = crc32(0L, NULL, 0);

  if (state->compressed == NULL) {
    archive_set_error(a, ENOMEM,
      "Can't allocate data for compression buffer");
    free(state);
    return (ARCHIVE_FATAL);
  }

  state->stream.next_out = state->compressed;
  state->stream.avail_out = state->compressed_buffer_size;

  /* Prime output buffer with a gzip header. */
  t = time(NULL);
  state->compressed[0] = 0x1f; /* GZip signature bytes */
  state->compressed[1] = 0x8b;
  state->compressed[2] = 0x08; /* "Deflate" compression */
  state->compressed[3] = 0; /* No options */
  state->compressed[4] = (t)&0xff;  /* Timestamp */
  state->compressed[5] = (t>>8)&0xff;
  state->compressed[6] = (t>>16)&0xff;
  state->compressed[7] = (t>>24)&0xff;
  state->compressed[8] = 0; /* No deflate options */
  state->compressed[9] = 3; /* OS=Unix */
  state->stream.next_out += 10;
  state->stream.avail_out -= 10;

  a->compression_write = archive_compressor_gzip_write;
  a->compression_finish = archive_compressor_gzip_finish;

  /* Initialize compression library. */
  ret = deflateInit2(&(state->stream),
    Z_DEFAULT_COMPRESSION,
    Z_DEFLATED,
    -15 /* < 0 to suppress zlib header */,
    8,
    Z_DEFAULT_STRATEGY);

  if (ret == Z_OK) {
    a->compression_data = state;
    return (0);
  }

  /* Library setup failed: clean up. */
  archive_set_error(a, ARCHIVE_ERRNO_MISC, "Internal error "
    "initializing compression library");
  free(state->compressed);
  free(state);

  /* Override the error message if we know what really went wrong. */
  switch (ret) {
  case Z_STREAM_ERROR:
    archive_set_error(a, ARCHIVE_ERRNO_MISC,
      "Internal error initializing "
      "compression library: invalid setup parameter");
    break;
  case Z_MEM_ERROR:
    archive_set_error(a, ENOMEM, "Internal error initializing "
      "compression library");
    break;
  case Z_VERSION_ERROR:
    archive_set_error(a, ARCHIVE_ERRNO_MISC,
      "Internal error initializing "
      "compression library: invalid library version");
    break;
  }

  return (ARCHIVE_FATAL);
}

/*
 * Write data to the compressed stream.
 */
  static int
archive_compressor_gzip_write(struct archive *a, const void *buff,
  size_t length)
{
  struct private_data *state;
  int ret;

  state = (struct private_data*)a->compression_data;
  if (a->client_writer == NULL) {
    archive_set_error(a, ARCHIVE_ERRNO_PROGRAMMER,
      "No write callback is registered?  "
      "This is probably an internal programming error.");
    return (ARCHIVE_FATAL);
  }

  /* Update statistics */
  state->crc = crc32(state->crc, (const Bytef*)buff, length);
  state->total_in += length;

  /* Compress input data to output buffer */
  SET_NEXT_IN(state, buff);
  state->stream.avail_in = length;
  if ((ret = drive_compressor(a, state, 0)) != ARCHIVE_OK)
    return (ret);

  a->file_position += length;
  return (ARCHIVE_OK);
}


/*
 * Finish the compression...
 */
  static int
archive_compressor_gzip_finish(struct archive *a)
{
  ssize_t block_length, target_block_length, bytes_written;
  int ret;
  struct private_data *state;
  unsigned tocopy;
  unsigned char trailer[8];

  state = (struct private_data*)a->compression_data;
  ret = 0;
  if (a->client_writer == NULL) {
    archive_set_error(a, ARCHIVE_ERRNO_PROGRAMMER,
      "No write callback is registered?  "
      "This is probably an internal programming error.");
    ret = ARCHIVE_FATAL;
    goto cleanup;
  }

  /* By default, always pad the uncompressed data. */
  if (a->pad_uncompressed) {
    tocopy = a->bytes_per_block -
      (state->total_in % a->bytes_per_block);
    while (tocopy > 0 && tocopy < (unsigned)a->bytes_per_block) {
      SET_NEXT_IN(state, a->nulls);
      state->stream.avail_in = tocopy < a->null_length ?
        tocopy : a->null_length;
      state->crc = crc32(state->crc, a->nulls,
        state->stream.avail_in);
      state->total_in += state->stream.avail_in;
      tocopy -= state->stream.avail_in;
      ret = drive_compressor(a, state, 0);
      if (ret != ARCHIVE_OK)
        goto cleanup;
    }
  }

  /* Finish compression cycle */
  if (((ret = drive_compressor(a, state, 1))) != ARCHIVE_OK)
    goto cleanup;

  /* Build trailer: 4-byte CRC and 4-byte length. */
  trailer[0] = (state->crc)&0xff;
  trailer[1] = (state->crc >> 8)&0xff;
  trailer[2] = (state->crc >> 16)&0xff;
  trailer[3] = (state->crc >> 24)&0xff;
  trailer[4] = (state->total_in)&0xff;
  trailer[5] = (state->total_in >> 8)&0xff;
  trailer[6] = (state->total_in >> 16)&0xff;
  trailer[7] = (state->total_in >> 24)&0xff;

  /* Add trailer to current block. */
  tocopy = 8;
  if (tocopy > state->stream.avail_out)
    tocopy = state->stream.avail_out;
  memcpy(state->stream.next_out, trailer, tocopy);
  state->stream.next_out += tocopy;
  state->stream.avail_out -= tocopy;

  /* If it overflowed, flush and start a new block. */
  if (tocopy < 8) {
    bytes_written = (a->client_writer)(a, a->client_data,
      state->compressed, state->compressed_buffer_size);
    if (bytes_written <= 0) {
      ret = ARCHIVE_FATAL;
      goto cleanup;
    }
    a->raw_position += bytes_written;
    state->stream.next_out = state->compressed;
    state->stream.avail_out = state->compressed_buffer_size;
    memcpy(state->stream.next_out, trailer + tocopy, 8-tocopy);
    state->stream.next_out += 8-tocopy;
    state->stream.avail_out -= 8-tocopy;
  }

  /* Optionally, pad the final compressed block. */
  block_length = state->stream.next_out - state->compressed;


  /* Tricky calculation to determine size of last block. */
  target_block_length = block_length;
  if (a->bytes_in_last_block <= 0)
    /* Default or Zero: pad to full block */
    target_block_length = a->bytes_per_block;
  else
    /* Round length to next multiple of bytes_in_last_block. */
    target_block_length = a->bytes_in_last_block *
      ( (block_length + a->bytes_in_last_block - 1) /
        a->bytes_in_last_block);
  if (target_block_length > a->bytes_per_block)
    target_block_length = a->bytes_per_block;
  if (block_length < target_block_length) {
    memset(state->stream.next_out, 0,
      target_block_length - block_length);
    block_length = target_block_length;
  }

  /* Write the last block */
  bytes_written = (a->client_writer)(a, a->client_data,
    state->compressed, block_length);
  if (bytes_written <= 0) {
    ret = ARCHIVE_FATAL;
    goto cleanup;
  }
  a->raw_position += bytes_written;

  /* Cleanup: shut down compressor, release memory, etc. */
cleanup:
  switch (deflateEnd(&(state->stream))) {
  case Z_OK:
    break;
  default:
    archive_set_error(a, ARCHIVE_ERRNO_MISC,
      "Failed to clean up compressor");
    ret = ARCHIVE_FATAL;
  }
  free(state->compressed);
  free(state);

  /* Close the output */
  if (a->client_closer != NULL)
    (a->client_closer)(a, a->client_data);

  return (ret);
}

/*
 * Utility function to push input data through compressor,
 * writing full output blocks as necessary.
 *
 * Note that this handles both the regular write case (finishing ==
 * false) and the end-of-archive case (finishing == true).
 */
  static int
drive_compressor(struct archive *a, struct private_data *state, int finishing)
{
  ssize_t bytes_written;
  int ret;

  for (;;) {
    if (state->stream.avail_out == 0) {
      bytes_written = (a->client_writer)(a, a->client_data,
        state->compressed, state->compressed_buffer_size);
      if (bytes_written <= 0) {
        /* TODO: Handle this write failure */
        return (ARCHIVE_FATAL);
      } else if ((size_t)bytes_written < state->compressed_buffer_size) {
        /* Short write: Move remaining to
         * front of block and keep filling */
        memmove(state->compressed,
          state->compressed + bytes_written,
          state->compressed_buffer_size - bytes_written);
      }
      a->raw_position += bytes_written;
      state->stream.next_out
        = state->compressed +
        state->compressed_buffer_size - bytes_written;
      state->stream.avail_out = bytes_written;
    }

    ret = deflate(&(state->stream),
      finishing ? Z_FINISH : Z_NO_FLUSH );

    switch (ret) {
    case Z_OK:
      /* In non-finishing case, check if compressor
       * consumed everything */
      if (!finishing && state->stream.avail_in == 0)
        return (ARCHIVE_OK);
      /* In finishing case, this return always means
       * there's more work */
      break;
    case Z_STREAM_END:
      /* This return can only occur in finishing case. */
      return (ARCHIVE_OK);
    default:
      /* Any other return value indicates an error. */
      archive_set_error(a, ARCHIVE_ERRNO_MISC,
        "GZip compression failed");
      return (ARCHIVE_FATAL);
    }
  }
}

