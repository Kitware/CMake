/*-
 * Copyright (c) 2003-2007 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
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
#include "test.h"
__FBSDID("$FreeBSD: src/lib/libarchive/test/test_read_format_mtree.c,v 1.4 2008/09/18 04:13:36 kientzle Exp $");

/* Single entry with a hardlink. */
static unsigned char archive[] = {
    "#mtree\n"
    "file type=file uid=18 mode=0123 size=3\n"
    "dir type=dir\n"
    " file\\040with\\040space type=file uid=18\n"
    " ..\n"
    "file\\040with\\040space type=file\n"
    "dir2 type=dir\n"
    " dir3a type=dir\n"
    "  indir3a type=file\n"
    "dir2/fullindir2 type=file mode=0777\n"
    "  ..\n"
    " indir2 type=file\n"
    " dir3b type=dir\n"
    "  indir3b type=file\n"
    "  ..\n"
    " ..\n"
    "notindir type=file\n"
    "dir2/fullindir2 mode=0644\n"
};

DEFINE_TEST(test_read_format_mtree)
{
    char buff[16];
    struct archive_entry *ae;
    struct archive *a;
    FILE *f;

    /*
     * An access error occurred on some platform when mtree
     * format handling open a directory. It is for through
     * the routine which open a directory that we create
     * "dir" and "dir2" directories.
     */
    assertMakeDir("dir", 0775);
    assertMakeDir("dir2", 0775);

    assert((a = archive_read_new()) != NULL);
    assertEqualIntA(a, ARCHIVE_OK,
        archive_read_support_compression_all(a));
    assertEqualIntA(a, ARCHIVE_OK,
        archive_read_support_format_all(a));
    assertEqualIntA(a, ARCHIVE_OK,
        archive_read_open_memory(a, archive, sizeof(archive)));

    /*
     * Read "file", whose data is available on disk.
     */
    f = fopen("file", "wb");
    assert(f != NULL);
    assertEqualInt(3, fwrite("hi\n", 1, 3, f));
    fclose(f);
    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualInt(archive_format(a), ARCHIVE_FORMAT_MTREE);
    assertEqualString(archive_entry_pathname(ae), "file");
    assertEqualInt(archive_entry_uid(ae), 18);
    assertEqualInt(AE_IFREG, archive_entry_filetype(ae));
    assertEqualInt(archive_entry_mode(ae), AE_IFREG | 0123);
    assertEqualInt(archive_entry_size(ae), 3);
    assertEqualInt(3, archive_read_data(a, buff, 3));
    assertEqualMem(buff, "hi\n", 3);

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir");
    assertEqualInt(AE_IFDIR, archive_entry_filetype(ae));

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir/file with space");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "file with space");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir2");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir2/dir3a");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir2/dir3a/indir3a");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir2/fullindir2");
    assertEqualInt(archive_entry_mode(ae), AE_IFREG | 0644);

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir2/indir2");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir2/dir3b");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "dir2/dir3b/indir3b");

    assertEqualIntA(a, ARCHIVE_OK, archive_read_next_header(a, &ae));
    assertEqualString(archive_entry_pathname(ae), "notindir");

    assertEqualIntA(a, ARCHIVE_EOF, archive_read_next_header(a, &ae));
    assertEqualInt(ARCHIVE_OK, archive_read_close(a));
#if ARCHIVE_VERSION_NUMBER < 2000000
    archive_read_finish(a);
#else
    assertEqualInt(ARCHIVE_OK, archive_read_finish(a));
#endif
}


