/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCPackDebGenerator.h"

#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>

#include <limits.h> // USHRT_MAX

// NOTE:
// A debian package .deb is simply an 'ar' archive. The only subtle difference
// is that debian uses the BSD ar style archive whereas most Linux distro have
// a GNU ar.
// See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=161593 for more info
// Therefore we provide our own implementation of a BSD-ar:
static int ar_append(const char*archive,const std::vector<std::string>& files);

//----------------------------------------------------------------------
cmCPackDebGenerator::cmCPackDebGenerator()
{
}

//----------------------------------------------------------------------
cmCPackDebGenerator::~cmCPackDebGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackDebGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackDebGenerator::CompressFiles(const char* outFileName,
  const char* toplevel,
  const std::vector<std::string>& files)
{
  this->ReadListFile("CPackDeb.cmake");
  const char* cmakeExecutable = this->GetOption("CMAKE_COMMAND");

  // debian-binary file
  std::string dbfilename;
  dbfilename = toplevel;
  dbfilename += "/debian-binary";
    { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(dbfilename.c_str());
    out << "2.0";
    out << std::endl; // required for valid debian package
    }

  // control file
  std::string ctlfilename;
  ctlfilename = toplevel;
  ctlfilename += "/control";

  // debian policy enforce lower case for package name
  // mandatory entries:
  std::string debian_pkg_name = cmsys::SystemTools::LowerCase( 
                                this->GetOption("CPACK_DEBIAN_PACKAGE_NAME") );
  const char* debian_pkg_version = 
                               this->GetOption("CPACK_DEBIAN_PACKAGE_VERSION");
  const char* debian_pkg_section = 
                               this->GetOption("CPACK_DEBIAN_PACKAGE_SECTION");
  const char* debian_pkg_priority = 
                              this->GetOption("CPACK_DEBIAN_PACKAGE_PRIORITY");
  const char* debian_pkg_arch = 
                          this->GetOption("CPACK_DEBIAN_PACKAGE_ARCHITECTURE");
  const char* maintainer =  this->GetOption("CPACK_DEBIAN_PACKAGE_MAINTAINER");
  const char* desc =       this->GetOption("CPACK_DEBIAN_PACKAGE_DESCRIPTION");

  // optional entries
  const char* debian_pkg_dep = this->GetOption("CPACK_DEBIAN_PACKAGE_DEPENDS");
  const char* debian_pkg_rec = 
                            this->GetOption("CPACK_DEBIAN_PACKAGE_RECOMMENDS");
  const char* debian_pkg_sug = 
                              this->GetOption("CPACK_DEBIAN_PACKAGE_SUGGESTS");

    { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(ctlfilename.c_str());
    out << "Package: " << debian_pkg_name << "\n";
    out << "Version: " << debian_pkg_version << "\n";
    out << "Section: " << debian_pkg_section << "\n";
    out << "Priority: " << debian_pkg_priority << "\n";
    out << "Architecture: " << debian_pkg_arch << "\n";
    if(debian_pkg_dep)
      {
      out << "Depends: " << debian_pkg_dep << "\n";
      }
    if(debian_pkg_rec)
      {
      out << "Recommends: " << debian_pkg_rec << "\n";
      }
    if(debian_pkg_sug)
      {
      out << "Suggests: " << debian_pkg_sug << "\n";
      }
    unsigned long totalSize = 0;
    {
      std::string dirName = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
      dirName += '/';
      for (std::vector<std::string>::const_iterator fileIt = files.begin();
              fileIt != files.end(); ++ fileIt )
        {
        totalSize += cmSystemTools::FileLength(fileIt->c_str());
        }
    }
    out << "Installed-Size: " << totalSize << "\n";
    out << "Maintainer: " << maintainer << "\n";
    out << "Description: " << desc << "\n";
    out << std::endl;
    }

  std::string cmd;
  cmd = "\"";
  cmd += cmakeExecutable;
  cmd += "\" -E tar cfz data.tar.gz ";

  // now add all directories which have to be compressed
  // collect all top level install dirs for that
  // e.g. /opt/bin/foo, /usr/bin/bar and /usr/bin/baz would give /usr and /opt
  int topLevelLength = strlen(toplevel);
  std::set<std::string> installDirs;
  for (std::vector<std::string>::const_iterator fileIt = files.begin(); 
       fileIt != files.end(); ++ fileIt )
    {
    std::string::size_type slashPos = fileIt->find('/', topLevelLength+1);
    std::string relativeDir = fileIt->substr(topLevelLength, 
                                             slashPos - topLevelLength);
    if (installDirs.find(relativeDir) == installDirs.end())
      {
      installDirs.insert(relativeDir);
      cmd += " .";
      cmd += relativeDir;
      }
    }

  std::string output;
  int retVal = -1;
  int res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);

  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/Deb.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << cmd.c_str() << std::endl
      << "# Working directory: " << toplevel << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running tar command: "
      << cmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  std::string md5filename;
  md5filename = toplevel;
  md5filename += "/md5sums";

    { // the scope is needed for cmGeneratedFileStream
    cmGeneratedFileStream out(md5filename.c_str());
    std::vector<std::string>::const_iterator fileIt;
    std::string topLevelWithTrailingSlash = toplevel;
    topLevelWithTrailingSlash += '/';
    for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
      {
      cmd = "\"";
      cmd += cmakeExecutable;
      cmd += "\" -E md5sum \"";
      cmd += *fileIt;
      cmd += "\"";
      //std::string output;
      //int retVal = -1;
      res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
        &retVal, toplevel, this->GeneratorVerbose, 0);
      // debian md5sums entries are like this:
      // 014f3604694729f3bf19263bac599765  usr/bin/ccmake
      // thus strip the full path (with the trailing slash)
      cmSystemTools::ReplaceString(output, 
                                   topLevelWithTrailingSlash.c_str(), "");
      out << output;
      }
    // each line contains a eol. 
    // Do not end the md5sum file with yet another (invalid)
    }


  cmd = "\"";
  cmd += cmakeExecutable;
  cmd += "\" -E tar cfz control.tar.gz ./control ./md5sums";
  const char* controlExtra = 
    this->GetOption("CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA");
  if( controlExtra )
    { 
    std::vector<std::string> controlExtraList;
    cmSystemTools::ExpandListArgument(controlExtra, controlExtraList);
    for(std::vector<std::string>::iterator i = 
          controlExtraList.begin(); i != controlExtraList.end(); ++i)
      {
      std::string filenamename = 
        cmsys::SystemTools::GetFilenameName(i->c_str());
      std::string localcopy = toplevel;
      localcopy += "/";
      localcopy += filenamename;
      // if we can copy the file, it means it does exist, let's add it:
      if( cmsys::SystemTools::CopyFileIfDifferent(
            i->c_str(), localcopy.c_str()) )
        {
        // debian is picky and need relative to ./ path in the tar.gz
        cmd += " ./";
        cmd += filenamename;
        }
      }
    }
  res = cmSystemTools::RunSingleCommand(cmd.c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);

  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/Deb.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << cmd.c_str() << std::endl
      << "# Working directory: " << toplevel << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running tar command: "
      << cmd.c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  // ar -r your-package-name.deb debian-binary control.tar.gz data.tar.gz
  // since debian packages require BSD ar (most Linux distros and even
  // FreeBSD and NetBSD ship GNU ar) we use a copy of OpenBSD ar here.
  std::vector<std::string> arFiles;
  std::string topLevelString = toplevel;
  topLevelString += "/";
  arFiles.push_back(topLevelString + "debian-binary");
  arFiles.push_back(topLevelString + "control.tar.gz");
  arFiles.push_back(topLevelString + "data.tar.gz");
  res = ar_append(outFileName, arFiles);
  if ( res!=0 )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/Deb.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Problem creating archive using: " << res << std::endl;
    return 0;
    }

  return 1;
}

// The following code is taken from OpenBSD ar:
// http://www.openbsd.org/cgi-bin/cvsweb/src/usr.bin/ar/
// It has been slightly modified:
// -return error codes instead exit() in functions
// -use the stdio file I/O functions instead the file descriptor based ones
// -merged into one cxx file
// -no additional options supported
// The coding style hasn't been modified.

/*-
 * Copyright (c) 1990, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARMAG           "!<arch>\n"        /* ar "magic number" */
#define SARMAG          8                  /* strlen(ARMAG); */

#define AR_EFMT1        "#1/"              /* extended format #1 */
#define ARFMAG          "`\n"

/* Header format strings. */
#define HDR1            "%s%-13d%-12ld%-6u%-6u%-8o%-10lld%2s"
#define HDR2             "%-16.16s%-12ld%-6u%-6u%-8o%-10lld%2s"

struct ar_hdr {
  char ar_name[16];                        /* name */
  char ar_date[12];                        /* modification time */
  char ar_uid[6];                          /* user id */
  char ar_gid[6];                          /* group id */
  char ar_mode[8];                         /* octal file permissions */
  char ar_size[10];                        /* size in bytes */
  char ar_fmag[2];                         /* consistency check */
};

/* Set up file copy. */
#define SETCF(from, fromname, to, toname, pad) { \
        cf.rFile = from; \
        cf.rname = fromname; \
        cf.wFile = to; \
        cf.wname = toname; \
        cf.flags = pad; \
}

/* File copy structure. */
typedef struct {
        FILE* rFile;                       /* read file descriptor */
        const char *rname;                 /* read name */
        FILE* wFile;                       /* write file descriptor */
        const char *wname;                 /* write name */
#define NOPAD        0x00                  /* don't pad */
#define WPAD        0x02                   /* pad on writes */
        unsigned int flags;                       /* pad flags */
} CF;

/* misc.c */

static const char * ar_rname(const char *path)
{
  const char *ind = strrchr(path, '/');
  return (ind ) ? ind + 1 : path;
}

/* archive.c */

typedef struct ar_hdr HDR;
static char ar_hb[sizeof(HDR) + 1];        /* real header */

static int ar_already_written;

/* copy_ar --
 *      Copy size bytes from one file to another - taking care to handle the
 *      extra byte (for odd size files) when reading archives and writing an
 *      extra byte if necessary when adding files to archive.  The length of
 *      the object is the long name plus the object itself; the variable
 *      already_written gets set if a long name was written.
 *
 *      The padding is really unnecessary, and is almost certainly a remnant
 *      of early archive formats where the header included binary data which
 *      a PDP-11 required to start on an even byte boundary.  (Or, perhaps,
 *      because 16-bit word addressed copies were faster?)  Anyhow, it should
 *      have been ripped out long ago.
 */
static int copy_ar(CF *cfp, off_t size)
{
  static char pad = '\n';
  off_t sz = size;
  size_t nr, nw;
  char buf[8*1024];

  if (sz == 0)
    return 0;

  FILE* from = cfp->rFile;
  FILE* to = cfp->wFile;
  while (sz && 
        (nr = fread(buf, 1, sz < static_cast<off_t>(sizeof(buf)) 
                    ? static_cast<size_t>(sz) : sizeof(buf), from ))
               > 0) {
    sz -= nr;
    for (size_t off = 0; off < nr; nr -= off, off += nw)
      if ((nw = fwrite(buf + off, 1, nr, to)) < nr)
        return -1;
    }
  if (sz)
    return -2;

  if (cfp->flags & WPAD && (size + ar_already_written) & 1 
      && fwrite(&pad, 1, 1, to) != 1)
    return -4;

  return 0;
}

/* put_arobj --  Write an archive member to a file. */
static int put_arobj(CF *cfp, struct stat *sb)
{
  int result = 0;
  struct ar_hdr *hdr;

 /* If passed an sb structure, reading a file from disk.  Get stat(2)
  * information, build a name and construct a header.  (Files are named
  * by their last component in the archive.) */
  const char* name = ar_rname(cfp->rname);
  (void)stat(cfp->rname, sb);

 /* If not truncating names and the name is too long or contains
  * a space, use extended format 1.   */
  unsigned int lname = strlen(name);
  uid_t uid = sb->st_uid;
  gid_t gid = sb->st_gid;
  if (uid > USHRT_MAX) {
    uid = USHRT_MAX;
    }
  if (gid > USHRT_MAX) {
    gid = USHRT_MAX;
    }
  if (lname > sizeof(hdr->ar_name) || strchr(name, ' '))
    (void)sprintf(ar_hb, HDR1, AR_EFMT1, lname,
                  (long int)sb->st_mtime, uid, gid, sb->st_mode,
                  (long long)sb->st_size + lname, ARFMAG);
    else {
      lname = 0;
      (void)sprintf(ar_hb, HDR2, name, 
                    (long int)sb->st_mtime, uid, gid, sb->st_mode, 
                    (long long)sb->st_size, ARFMAG);
      }
    off_t size = sb->st_size;

  if (fwrite(ar_hb, 1, sizeof(HDR), cfp->wFile) != sizeof(HDR))
    return -1;

  if (lname) {
    if (fwrite(name, 1, lname, cfp->wFile) != lname)
      return -2;
    ar_already_written = lname;
    }
  result = copy_ar(cfp, size);
  ar_already_written = 0;
  return result;
}

/* append.c */

/* append --
 *      Append files to the archive - modifies original archive or creates
 *      a new archive if named archive does not exist. 
 */
static int ar_append(const char* archive,const std::vector<std::string>& files)
{
  int eval = 0;
  FILE* aFile = fopen(archive, "wb+");
  if (aFile!=NULL) {
    fwrite(ARMAG, SARMAG, 1, aFile);
    if (fseek(aFile, 0, SEEK_END) != -1) {
      CF cf;
      struct stat sb;
      /* Read from disk, write to an archive; pad on write. */
      SETCF(NULL, 0, aFile, archive, WPAD);
      for(std::vector<std::string>::const_iterator fileIt = files.begin();
          fileIt!=files.end(); ++fileIt) {
        const char* filename = fileIt->c_str();
        FILE* file = fopen(filename, "rb");
        if (file == NULL) {
          eval = -1;
          continue;
          }
        cf.rFile = file;
        cf.rname = filename;
        int result = put_arobj(&cf, &sb);
        (void)fclose(file);
        if (result!=0) {
          eval = -2;
          break;
          }
        }
      }
      else {
        eval = -3;
      }
    fclose(aFile);
    }
  else {
    eval = -4;
    }
  return eval;
}
