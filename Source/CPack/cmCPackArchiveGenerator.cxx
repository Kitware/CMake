/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackArchiveGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"
#include <errno.h>

#include <cmsys/SystemTools.hxx>
#include <cmlibarchive/libarchive/archive.h>
#include <cmlibarchive/libarchive/archive_entry.h>


//----------------------------------------------------------------------
cmCPackArchiveGenerator::cmCPackArchiveGenerator(CompressType t,
  ArchiveType at)
{
  this->Compress = t;
  this->Archive = at;
}

//----------------------------------------------------------------------
cmCPackArchiveGenerator::~cmCPackArchiveGenerator()
{
}

static const size_t cmCPackTGZ_Data_BlockSize = 16384;

// make this an anonymous namespace so that archive.h does not
// have to be included in the .h file for this class
namespace
{
bool SetArchiveType(struct archive* a,
                     cmCPackArchiveGenerator::CompressType ct,
                     cmCPackArchiveGenerator::ArchiveType at)
{
  int res = 0;
  // pick the archive type
  switch(at)
    {
    case cmCPackArchiveGenerator::TAR:
      // maybe this:
      res = archive_write_set_format_pax_restricted(a);
      break;
    case cmCPackArchiveGenerator::ZIP:
      res = archive_write_set_format_zip(a);
      break;
    }
  if(res != ARCHIVE_OK)
    {
    return false;
    }

  // pick a compression type
  switch(ct)
    {
    case cmCPackArchiveGenerator::GZIP:
      res = archive_write_set_compression_gzip(a);
      break;
    case cmCPackArchiveGenerator::BZIP2:
      res = archive_write_set_compression_bzip2(a);
      break;
    case cmCPackArchiveGenerator::COMPRESS:
      res = archive_write_set_compression_compress(a);
      break;
    case cmCPackArchiveGenerator::LZMA:
      res = archive_write_set_compression_lzma(a);
      break;
    case cmCPackArchiveGenerator::NONE:
    default:
      res = archive_write_set_compression_none(a);
    } 
  if(res != ARCHIVE_OK)
    {
    return false;
    }
  // do not pad the last block!!
  res = archive_write_set_bytes_in_last_block(a, 1);
  if(res != ARCHIVE_OK)
    {
    return false;
    }
  
  return true;
}
  
struct StreamData
{
  StreamData(cmGeneratedFileStream* gfs,
    cmCPackArchiveGenerator* ag) 
    { 
      this->GeneratedFileStream = gfs;
      this->Generator = ag;
    }
  cmGeneratedFileStream* GeneratedFileStream;
  cmCPackArchiveGenerator* Generator;
};


extern "C"
{
int OpenArchive(struct archive *, void *client_data)
{
  struct StreamData *data = (StreamData*)client_data;
  if(data->GeneratedFileStream && 
     *data->GeneratedFileStream)
    {
    if(data->Generator->
       GenerateHeader(data->GeneratedFileStream))
      {
      return ARCHIVE_OK;
      }
    }
  return (ARCHIVE_FATAL);
}
  
__LA_SSIZE_T WriteArchive(struct archive *,
                          void *client_data, 
                          const void *buff,
                          size_t n)
{
  struct StreamData *data = (StreamData*)client_data;
  data->GeneratedFileStream->
    write(reinterpret_cast<const char*>(buff),n);
  if(!data->GeneratedFileStream->bad())
    {
    return n;
    }
  return 0;
}


int CloseArchive(struct archive *, void *client_data)
{
  struct StreamData *data = (StreamData*)client_data;
  if(data->GeneratedFileStream->Close())
    {
    delete data->GeneratedFileStream;
    return ARCHIVE_OK;
    }
  return ARCHIVE_FATAL;
}
} //extern C
} // anon name space


//----------------------------------------------------------------------
int cmCPackArchiveGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  return this->Superclass::InitializeInternal();
}

int cmCPackArchiveGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  int res = ARCHIVE_OK;
#define CHECK_ARCHIVE_ERROR(res, msg)           \
  if(res != ARCHIVE_OK)                         \
    {\
    cmCPackLogger(cmCPackLog::LOG_ERROR, msg      \
                  << archive_error_string(a)      \
                  << cmSystemTools::GetLastSystemError()  \
                  << " " << res                   \
                  << "\n");                       \
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                << (toplevel ? toplevel : "(NULL)") << std::endl);
  // create a new archive
  struct archive* a = archive_write_new();
  // Set the compress and archive types for the archive
  SetArchiveType(a, this->Compress, this->Archive);
  // Open binary stream
  cmGeneratedFileStream* gf = new cmGeneratedFileStream;
  gf->Open(outFileName, false, true);
  StreamData data(gf, this);
  // pass callbacks to archive_write_open to handle stream
  res = archive_write_open(a,
                           &data,
                           OpenArchive,
                           WriteArchive, 
                           CloseArchive);
  CHECK_ARCHIVE_ERROR(res, "archive_write_open:");
  // create a new disk struct
  struct archive* disk = archive_read_disk_new();
#if !defined(_WIN32) || defined(__CYGWIN__)
  res = archive_read_disk_set_standard_lookup(disk);
#endif
  CHECK_ARCHIVE_ERROR(res, "archive_read_disk_set_standard_lookup:");
  std::vector<std::string>::const_iterator fileIt;
  std::string dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(toplevel);
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    // create a new entry for each file
    struct archive_entry *entry = archive_entry_new();
    // Get the relative path to the file
    std::string rp = cmSystemTools::RelativePath(toplevel, fileIt->c_str());
    // Set the name of the entry to the file name
    archive_entry_set_pathname(entry, rp.c_str());  
    res = archive_read_disk_entry_from_file(disk, entry, -1, 0);
    CHECK_ARCHIVE_ERROR(res, "archive_read_disk_entry_from_file:");
    // write  entry header
    res = archive_write_header(a, entry);
    CHECK_ARCHIVE_ERROR(res, "archive_write_header:");
    // the entry size can be 0 if it is a symlink
    if(archive_entry_size(entry) > 0)
      {
      // now copy contents of file into archive a
      FILE* file = fopen(fileIt->c_str(), "rb");
      if(!file)
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem with fopen(): " 
                      << fileIt->c_str()
                      << strerror(errno)
                      << std::endl);
        return 0;
        }
      char buff[cmCPackTGZ_Data_BlockSize];
      size_t len = fread(buff, 1, sizeof(buff), file);
      while (len > 0)
        {
        size_t wlen = archive_write_data(a, buff, len);
        if(wlen != len)
          {
          cmCPackLogger(cmCPackLog::LOG_ERROR, "archive_write_data(): " 
                        << "tried to write " << len  << "\n"
                        << "write " << wlen << "\n");
          return 0;
          }
        len = fread(buff, 1, sizeof(buff), file);
        }
      // close the file and free the entry
      fclose(file);
      }
    archive_entry_free(entry);
    }
  cmSystemTools::ChangeDirectory(dir.c_str());
  // close the archive and finish the write
  archive_write_close(a);
  archive_write_finish(a);
  archive_read_finish(disk);
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::GenerateHeader(std::ostream*)
{
  return 1;
}
