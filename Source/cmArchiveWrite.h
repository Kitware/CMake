/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2010 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmArchiveWrite_h
#define cmArchiveWrite_h

#include "cmStandardIncludes.h"

#if !defined(CMAKE_BUILD_WITH_CMAKE)
# error "cmArchiveWrite not allowed during bootstrap build!"
#endif

/** \class cmArchiveWrite
 * \brief Wrapper around libarchive for writing.
 *
 */
class cmArchiveWrite
{
  typedef void (cmArchiveWrite::* safe_bool)();
  void safe_bool_true() {}
public:
  /** Compression type.  */
  enum Compress
  {
    CompressNone,
    CompressCompress,
    CompressGZip,
    CompressBZip2,
    CompressLZMA,
    CompressXZ
  };

  /** Construct with output stream to which to write archive.  */
  cmArchiveWrite(std::ostream& os, Compress c = CompressNone,
    std::string const& format = "paxr");

  ~cmArchiveWrite();

  /**
   * Add a path (file or directory) to the archive.  Directories are
   * added recursively.  The "path" must be readable on disk, either
   * full path or relative to current working directory.  The "skip"
   * value indicates how many leading bytes from the input path to
   * skip.  The remaining part of the input path is appended to the
   * "prefix" value to construct the final name in the archive.
   */
  bool Add(std::string path,
     size_t skip = 0,
     const char* prefix = 0,
     bool recursive = true);

  /** Returns true if there has been no error.  */
  operator safe_bool() const
    { return this->Okay()? &cmArchiveWrite::safe_bool_true : 0; }

  /** Returns true if there has been an error.  */
  bool operator!() const { return !this->Okay(); }

  /** Return the error string; empty if none.  */
  std::string GetError() const { return this->Error; }

  // TODO: More general callback instead of hard-coding calls to
  // std::cout.
  void SetVerbose(bool v) { this->Verbose = v; }

  void SetMTime(std::string const& t) { this->MTime = t; }

  //! Sets the permissions of the added files/folders
  //! @note set to -1 to use the default permissions
  long int SetPermissions(long int permissions_)
    {
    std::swap(this->permissions, permissions_);
    return permissions_;
    }

  //! Sets the permissions mask of files/folders
  //!
  //! The permissions will be copied from the existing file
  //! or folder. The mask will then be applied to unset
  //! some of them
  long int SetPermissionsMask(long int permissionsMask_)
    {
    std::swap(this->permissionsMask, permissionsMask_);
    return permissionsMask_;
    }

  //! Sets the UID to be used in the tar file
  //! @return the previous UID
  //! @note set to -1 to disable the UID overriding
  long int SetUID( long int uid_ )
    {
    std::swap(this->uid, uid_);
    return uid_;
    }

  std::string SetUNAME(std::string uname_)
    {
    std::swap(this->uname, uname_);
    return uname_;
    }

  //! Sets the UID to be used in the tar file
  //! @return the previous UID
  long int SetGID( long int gid_ )
    {
    std::swap(this->gid, gid_);
    return gid_;
    }

  std::string SetGNAME(std::string gname_)
    {
    std::swap(this->gname, gname_);
    return gname_;
    }

private:
  bool Okay() const { return this->Error.empty(); }
  bool AddPath(const char* path, size_t skip, const char* prefix,
               bool recursive = true);
  bool AddFile(const char* file, size_t skip, const char* prefix);
  bool AddData(const char* file, size_t size);

  struct Callback;
  friend struct Callback;

  class Entry;

  std::ostream& Stream;
  struct archive* Archive;
  struct archive* Disk;
  bool Verbose;
  std::string Format;
  std::string Error;
  std::string MTime;

  //! UID of the user in the tar file. Set to -1
  //! to disable overriding
  long int uid;

  //! GUID of the user in the tar file
  long int gid;

  //! UNAME/GNAME of the user (does not override UID/GID)
  //!@{
  std::string uname;
  std::string gname;
  //!@}

  //! Permissions on files/folders
  long int permissions;
  long int permissionsMask;
};

#endif
