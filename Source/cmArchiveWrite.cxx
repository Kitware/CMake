/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmArchiveWrite.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>

#include <cm/algorithm>

#include <cm3p/archive.h>
#include <cm3p/archive_entry.h>

#include "cmsys/Directory.hxx"
#include "cmsys/Encoding.hxx"
#include "cmsys/FStream.hxx"

#include "cm_get_date.h"

#include "cmLocale.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#ifndef __LA_SSIZE_T
#  define __LA_SSIZE_T la_ssize_t
#endif

static std::string cm_archive_error_string(struct archive* a)
{
  const char* e = archive_error_string(a);
  return e ? e : "unknown error";
}

static void cm_archive_entry_copy_pathname(struct archive_entry* e,
                                           const std::string& dest)
{
#if cmsys_STL_HAS_WSTRING
  archive_entry_copy_pathname_w(e, cmsys::Encoding::ToWide(dest).c_str());
#else
  archive_entry_copy_pathname(e, dest.c_str());
#endif
}

static void cm_archive_entry_copy_sourcepath(struct archive_entry* e,
                                             const std::string& file)
{
#if cmsys_STL_HAS_WSTRING
  archive_entry_copy_sourcepath_w(e, cmsys::Encoding::ToWide(file).c_str());
#else
  archive_entry_copy_sourcepath(e, file.c_str());
#endif
}

class cmArchiveWrite::Entry
{
  struct archive_entry* Object;

public:
  Entry()
    : Object(archive_entry_new())
  {
  }
  ~Entry() { archive_entry_free(this->Object); }
  Entry(const Entry&) = delete;
  Entry& operator=(const Entry&) = delete;
  operator struct archive_entry*() { return this->Object; }
};

struct cmArchiveWrite::Callback
{
  // archive_write_callback
  static __LA_SSIZE_T Write(struct archive* /*unused*/, void* cd,
                            const void* b, size_t n)
  {
    cmArchiveWrite* self = static_cast<cmArchiveWrite*>(cd);
    if (self->Stream.write(static_cast<const char*>(b),
                           static_cast<std::streamsize>(n))) {
      return static_cast<__LA_SSIZE_T>(n);
    }
    return static_cast<__LA_SSIZE_T>(-1);
  }
};

cmArchiveWrite::cmArchiveWrite(std::ostream& os, Compress c,
                               std::string const& format, int compressionLevel,
                               int numThreads)
  : Stream(os)
  , Archive(archive_write_new())
  , Disk(archive_read_disk_new())
  , Format(format)
{
  // Upstream fixed an issue with their integer parsing in 3.4.0
  // which would cause spurious errors to be raised from `strtoull`.

  if (numThreads < 1) {
    int upperLimit = (numThreads == 0) ? std::numeric_limits<int>::max()
                                       : std::abs(numThreads);

    numThreads =
      cm::clamp<int>(std::thread::hardware_concurrency(), 1, upperLimit);
  }

  std::string sNumThreads = std::to_string(numThreads);

  switch (c) {
    case CompressNone:
      if (archive_write_add_filter_none(this->Archive) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_add_filter_none: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
      break;
    case CompressCompress:
      if (archive_write_add_filter_compress(this->Archive) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_add_filter_compress: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
      break;
    case CompressGZip: {
      if (archive_write_add_filter_gzip(this->Archive) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_add_filter_gzip: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
      std::string source_date_epoch;
      cmSystemTools::GetEnv("SOURCE_DATE_EPOCH", source_date_epoch);
      if (!source_date_epoch.empty()) {
        // We're not able to specify an arbitrary timestamp for gzip.
        // The next best thing is to omit the timestamp entirely.
        if (archive_write_set_filter_option(this->Archive, "gzip", "timestamp",
                                            nullptr) != ARCHIVE_OK) {
          this->Error = cmStrCat("archive_write_set_filter_option: ",
                                 cm_archive_error_string(this->Archive));
          return;
        }
      }
    } break;
    case CompressBZip2:
      if (archive_write_add_filter_bzip2(this->Archive) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_add_filter_bzip2: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
      break;
    case CompressLZMA:
      if (archive_write_add_filter_lzma(this->Archive) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_add_filter_lzma: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
      break;
    case CompressXZ:
      if (archive_write_add_filter_xz(this->Archive) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_add_filter_xz: ",
                               cm_archive_error_string(this->Archive));
        return;
      }

#if ARCHIVE_VERSION_NUMBER >= 3004000

#  ifdef _AIX
      // FIXME: Using more than 2 threads creates an empty archive.
      // Enforce this limit pending further investigation.
      if (numThreads > 2) {
        numThreads = 2;
        sNumThreads = std::to_string(numThreads);
      }
#  endif
      if (archive_write_set_filter_option(this->Archive, "xz", "threads",
                                          sNumThreads.c_str()) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_compressor_xz_options: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
#endif

      break;
    case CompressZstd:
      if (archive_write_add_filter_zstd(this->Archive) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_add_filter_zstd: ",
                               cm_archive_error_string(this->Archive));
        return;
      }

#if ARCHIVE_VERSION_NUMBER >= 3006000
      if (archive_write_set_filter_option(this->Archive, "zstd", "threads",
                                          sNumThreads.c_str()) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_compressor_zstd_options: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
#endif
      break;
  }

  if (compressionLevel != 0) {
    std::string compressionLevelStr = std::to_string(compressionLevel);
    std::string archiveFilterName;
    switch (c) {
      case CompressNone:
      case CompressCompress:
        break;
      case CompressGZip:
        archiveFilterName = "gzip";
        break;
      case CompressBZip2:
        archiveFilterName = "bzip2";
        break;
      case CompressLZMA:
        archiveFilterName = "lzma";
        break;
      case CompressXZ:
        archiveFilterName = "xz";
        break;
      case CompressZstd:
        archiveFilterName = "zstd";
        break;
    }
    if (!archiveFilterName.empty()) {
      if (archive_write_set_filter_option(
            this->Archive, archiveFilterName.c_str(), "compression-level",
            compressionLevelStr.c_str()) != ARCHIVE_OK) {
        this->Error = cmStrCat("archive_write_set_filter_option: ",
                               cm_archive_error_string(this->Archive));
        return;
      }
    }
  }

#if !defined(_WIN32) || defined(__CYGWIN__)
  if (archive_read_disk_set_standard_lookup(this->Disk) != ARCHIVE_OK) {
    this->Error = cmStrCat("archive_read_disk_set_standard_lookup: ",
                           cm_archive_error_string(this->Archive));
    return;
  }
#endif

  if (archive_write_set_format_by_name(this->Archive, format.c_str()) !=
      ARCHIVE_OK) {
    this->Error = cmStrCat("archive_write_set_format_by_name: ",
                           cm_archive_error_string(this->Archive));
    return;
  }

  // do not pad the last block!!
  if (archive_write_set_bytes_in_last_block(this->Archive, 1)) {
    this->Error = cmStrCat("archive_write_set_bytes_in_last_block: ",
                           cm_archive_error_string(this->Archive));
    return;
  }
}

bool cmArchiveWrite::Open()
{
  if (!this->Error.empty()) {
    return false;
  }
  if (archive_write_open(
        this->Archive, this, nullptr,
        reinterpret_cast<archive_write_callback*>(&Callback::Write),
        nullptr) != ARCHIVE_OK) {
    this->Error =
      cmStrCat("archive_write_open: ", cm_archive_error_string(this->Archive));
    return false;
  }
  return true;
}

cmArchiveWrite::~cmArchiveWrite()
{
  archive_read_free(this->Disk);
  archive_write_free(this->Archive);
}

bool cmArchiveWrite::Add(std::string path, size_t skip, const char* prefix,
                         bool recursive)
{
  if (!path.empty() && path.back() == '/') {
    path.erase(path.size() - 1);
  }
  this->AddPath(path.c_str(), skip, prefix, recursive);
  return this->Okay();
}

bool cmArchiveWrite::AddPath(const char* path, size_t skip, const char* prefix,
                             bool recursive)
{
  if (strcmp(path, ".") != 0 ||
      (this->Format != "zip" && this->Format != "7zip")) {
    if (!this->AddFile(path, skip, prefix)) {
      return false;
    }
  }
  if ((!cmSystemTools::FileIsDirectory(path) || !recursive) ||
      cmSystemTools::FileIsSymlink(path)) {
    return true;
  }
  cmsys::Directory d;
  if (d.Load(path)) {
    std::string next = cmStrCat(path, '/');
    if (next == "./" && (this->Format == "zip" || this->Format == "7zip")) {
      next.clear();
    }
    std::string::size_type end = next.size();
    unsigned long n = d.GetNumberOfFiles();
    for (unsigned long i = 0; i < n; ++i) {
      const char* file = d.GetFile(i);
      if (strcmp(file, ".") != 0 && strcmp(file, "..") != 0) {
        next.erase(end);
        next += file;
        if (!this->AddPath(next.c_str(), skip, prefix)) {
          return false;
        }
      }
    }
  }
  return true;
}

bool cmArchiveWrite::AddFile(const char* file, size_t skip, const char* prefix)
{
  this->Error = "";
  // Skip the file if we have no name for it.  This may happen on a
  // top-level directory, which does not need to be included anyway.
  if (skip >= strlen(file)) {
    return true;
  }
  const char* out = file + skip;

  cmLocaleRAII localeRAII;
  static_cast<void>(localeRAII);

  // Meta-data.
  std::string dest = cmStrCat(prefix ? prefix : "", out);
  if (this->Verbose) {
    std::cout << dest << "\n";
  }
  Entry e;
  cm_archive_entry_copy_sourcepath(e, file);
  cm_archive_entry_copy_pathname(e, dest);
  if (archive_read_disk_entry_from_file(this->Disk, e, -1, nullptr) !=
      ARCHIVE_OK) {
    this->Error = cmStrCat("Unable to read from file '", file,
                           "': ", cm_archive_error_string(this->Disk));
    return false;
  }
  if (!this->MTime.empty()) {
    time_t now;
    time(&now);
    time_t t = cm_get_date(now, this->MTime.c_str());
    if (t == -1) {
      this->Error = cmStrCat("unable to parse mtime '", this->MTime, '\'');
      return false;
    }
    archive_entry_set_mtime(e, t, 0);
  } else {
    std::string source_date_epoch;
    cmSystemTools::GetEnv("SOURCE_DATE_EPOCH", source_date_epoch);
    if (!source_date_epoch.empty()) {
      std::istringstream iss(source_date_epoch);
      time_t epochTime;
      iss >> epochTime;
      if (iss.eof() && !iss.fail()) {
        // Set all of the file times to the epoch time to handle archive
        // formats that include creation/access time.
        archive_entry_set_mtime(e, epochTime, 0);
        archive_entry_set_atime(e, epochTime, 0);
        archive_entry_set_ctime(e, epochTime, 0);
        archive_entry_set_birthtime(e, epochTime, 0);
      }
    }
  }

  // manages the uid/guid of the entry (if any)
  if (this->Uid.IsSet() && this->Gid.IsSet()) {
    archive_entry_set_uid(e, this->Uid.Get());
    archive_entry_set_gid(e, this->Gid.Get());
  }

  if (!this->Uname.empty() && !this->Gname.empty()) {
    archive_entry_set_uname(e, this->Uname.c_str());
    archive_entry_set_gname(e, this->Gname.c_str());
  }

  // manages the permissions
  if (this->Permissions.IsSet()) {
    archive_entry_set_perm(e, this->Permissions.Get());
  }

  if (this->PermissionsMask.IsSet()) {
    int perm = archive_entry_perm(e);
    archive_entry_set_perm(e, perm & this->PermissionsMask.Get());
  }

  // Clear acl and xattr fields not useful for distribution.
  archive_entry_acl_clear(e);
  archive_entry_xattr_clear(e);
  archive_entry_set_fflags(e, 0, 0);

  if (this->Format == "pax" || this->Format == "paxr") {
    // Sparse files are a GNU tar extension.
    // Do not use them in standard tar files.
    archive_entry_sparse_clear(e);
  }

  if (archive_write_header(this->Archive, e) != ARCHIVE_OK) {
    this->Error = cmStrCat("archive_write_header: ",
                           cm_archive_error_string(this->Archive));
    return false;
  }

  // do not copy content of symlink
  if (!archive_entry_symlink(e)) {
    // Content.
    if (size_t size = static_cast<size_t>(archive_entry_size(e))) {
      return this->AddData(file, size);
    }
  }
  return true;
}

bool cmArchiveWrite::AddData(const char* file, size_t size)
{
  cmsys::ifstream fin(file, std::ios::in | std::ios::binary);
  if (!fin) {
    this->Error = cmStrCat("Error opening \"", file,
                           "\": ", cmSystemTools::GetLastSystemError());
    return false;
  }

  char buffer[16384];
  size_t nleft = size;
  while (nleft > 0) {
    using ssize_type = std::streamsize;
    size_t const nnext = nleft > sizeof(buffer) ? sizeof(buffer) : nleft;
    ssize_type const nnext_s = static_cast<ssize_type>(nnext);
    fin.read(buffer, nnext_s);
    // Some stream libraries (older HPUX) return failure at end of
    // file on the last read even if some data were read.  Check
    // gcount instead of trusting the stream error status.
    if (static_cast<size_t>(fin.gcount()) != nnext) {
      break;
    }
    if (archive_write_data(this->Archive, buffer, nnext) != nnext_s) {
      this->Error = cmStrCat("archive_write_data: ",
                             cm_archive_error_string(this->Archive));
      return false;
    }
    nleft -= nnext;
  }
  if (nleft > 0) {
    this->Error = cmStrCat("Error reading \"", file,
                           "\": ", cmSystemTools::GetLastSystemError());
    return false;
  }
  return true;
}
