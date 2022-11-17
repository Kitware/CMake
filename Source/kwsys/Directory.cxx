/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Directory.hxx)

#include KWSYS_HEADER(Configure.hxx)

#include KWSYS_HEADER(Encoding.hxx)

#include KWSYS_HEADER(SystemTools.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#  include "Configure.hxx.in"
#  include "Directory.hxx.in"
#  include "Encoding.hxx.in"
#endif

#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h>

#  include <ctype.h>
#  include <fcntl.h>
#  include <io.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

namespace KWSYS_NAMESPACE {

class DirectoryInternals
{
public:
  struct FileData
  {
    std::string Name;
#if defined(_WIN32) && !defined(__CYGWIN__)
    WIN32_FIND_DATAW FindData;
#endif
    FileData(std::string name
#if defined(_WIN32) && !defined(__CYGWIN__)
             ,
             WIN32_FIND_DATAW data
#endif
             )
      : Name(std::move(name))
#if defined(_WIN32) && !defined(__CYGWIN__)
      , FindData(std::move(data))
#endif
    {
    }
  };
  // Array of Files
  std::vector<FileData> Files;

  // Path to Open'ed directory
  std::string Path;
};

Directory::Directory()
{
  this->Internal = new DirectoryInternals;
}

Directory::Directory(Directory&& other)
{
  this->Internal = other.Internal;
  other.Internal = nullptr;
}

Directory& Directory::operator=(Directory&& other)
{
  std::swap(this->Internal, other.Internal);
  return *this;
}

Directory::~Directory()
{
  delete this->Internal;
}

unsigned long Directory::GetNumberOfFiles() const
{
  return static_cast<unsigned long>(this->Internal->Files.size());
}

const char* Directory::GetFile(unsigned long dindex) const
{
  return this->Internal->Files[dindex].Name.c_str();
}

std::string const& Directory::GetFileName(std::size_t i) const
{
  return this->Internal->Files[i].Name;
}

std::string Directory::GetFilePath(std::size_t i) const
{
  std::string abs = this->Internal->Path;
  if (!abs.empty() && abs.back() != '/') {
    abs += '/';
  }
  abs += this->Internal->Files[i].Name;
  return abs;
}

bool Directory::FileIsDirectory(std::size_t i) const
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  auto const& data = this->Internal->Files[i].FindData;
  return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
  std::string const& path = this->GetFilePath(i);
  return kwsys::SystemTools::FileIsDirectory(path);
#endif
}

bool Directory::FileIsSymlink(std::size_t i) const
{
  std::string const& path = this->GetFilePath(i);
#if defined(_WIN32) && !defined(__CYGWIN__)
  auto const& data = this->Internal->Files[i].FindData;
  return kwsys::SystemTools::FileIsSymlinkWithAttr(
    Encoding::ToWindowsExtendedPath(path), data.dwFileAttributes);
#else
  return kwsys::SystemTools::FileIsSymlink(path);
#endif
}

const char* Directory::GetPath() const
{
  return this->Internal->Path.c_str();
}

void Directory::Clear()
{
  this->Internal->Path.resize(0);
  this->Internal->Files.clear();
}

} // namespace KWSYS_NAMESPACE

// First Windows platforms

#if defined(_WIN32) && !defined(__CYGWIN__)

namespace KWSYS_NAMESPACE {

Status Directory::Load(std::string const& name, std::string* errorMessage)
{
  this->Clear();
  HANDLE srchHandle;
  char* buf;
  size_t bufLength;
  size_t n = name.size();
  if (name.back() == '/' || name.back() == '\\') {
    bufLength = n + 1 + 1;
    buf = new char[bufLength];
    snprintf(buf, bufLength, "%s*", name.c_str());
  } else {
    // Make sure the slashes in the wildcard suffix are consistent with the
    // rest of the path
    bufLength = n + 2 + 1;
    buf = new char[bufLength];
    if (name.find('\\') != std::string::npos) {
      snprintf(buf, bufLength, "%s\\*", name.c_str());
    } else {
      snprintf(buf, bufLength, "%s/*", name.c_str());
    }
  }
  WIN32_FIND_DATAW data; // data of current file

  // Now put them into the file array
  srchHandle =
    FindFirstFileW(Encoding::ToWindowsExtendedPath(buf).c_str(), &data);
  delete[] buf;

  if (srchHandle == INVALID_HANDLE_VALUE) {
    Status status = Status::POSIX_errno();
    if (errorMessage) {
      *errorMessage = status.GetString();
    }
    return status;
  }

  // Loop through names
  do {
    this->Internal->Files.emplace_back(Encoding::ToNarrow(data.cFileName),
                                       data);
  } while (FindNextFileW(srchHandle, &data));
  this->Internal->Path = name;
  if (!FindClose(srchHandle)) {
    Status status = Status::POSIX_errno();
    if (errorMessage) {
      *errorMessage = status.GetString();
    }
    return status;
  }
  return Status::Success();
}

unsigned long Directory::GetNumberOfFilesInDirectory(const std::string& name,
                                                     std::string* errorMessage)
{
  HANDLE srchHandle;
  char* buf;
  size_t bufLength;
  size_t n = name.size();
  if (name.back() == '/') {
    bufLength = n + 1 + 1;
    buf = new char[n + 1 + 1];
    snprintf(buf, bufLength, "%s*", name.c_str());
  } else {
    bufLength = n + 2 + 1;
    buf = new char[n + 2 + 1];
    snprintf(buf, bufLength, "%s/*", name.c_str());
  }
  WIN32_FIND_DATAW data; // data of current file

  // Now put them into the file array
  srchHandle = FindFirstFileW(Encoding::ToWide(buf).c_str(), &data);
  delete[] buf;

  if (srchHandle == INVALID_HANDLE_VALUE) {
    if (errorMessage) {
      if (unsigned int errorId = GetLastError()) {
        LPSTR message = nullptr;
        DWORD size = FormatMessageA(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
          nullptr, errorId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPSTR)&message, 0, nullptr);
        *errorMessage = std::string(message, size);
        LocalFree(message);
      } else {
        *errorMessage = "Unknown error.";
      }
    }
    return 0;
  }

  // Loop through names
  unsigned long count = 0;
  do {
    count++;
  } while (FindNextFileW(srchHandle, &data));
  FindClose(srchHandle);
  return count;
}

} // namespace KWSYS_NAMESPACE

#else

// Now the POSIX style directory access

#  include <sys/types.h>

#  include <dirent.h>
#  include <errno.h>
#  include <string.h>

// PGI with glibc has trouble with dirent and large file support:
//  http://www.pgroup.com/userforum/viewtopic.php?
//  p=1992&sid=f16167f51964f1a68fe5041b8eb213b6
// Work around the problem by mapping dirent the same way as readdir.
#  if defined(__PGI) && defined(__GLIBC__)
#    define kwsys_dirent_readdir dirent
#    define kwsys_dirent_readdir64 dirent64
#    define kwsys_dirent kwsys_dirent_lookup(readdir)
#    define kwsys_dirent_lookup(x) kwsys_dirent_lookup_delay(x)
#    define kwsys_dirent_lookup_delay(x) kwsys_dirent_##x
#  else
#    define kwsys_dirent dirent
#  endif

namespace KWSYS_NAMESPACE {

Status Directory::Load(std::string const& name, std::string* errorMessage)
{
  this->Clear();

  errno = 0;
  DIR* dir = opendir(name.c_str());

  if (!dir) {
    if (errorMessage != nullptr) {
      *errorMessage = std::string(strerror(errno));
    }
    return Status::POSIX_errno();
  }

  errno = 0;
  for (kwsys_dirent* d = readdir(dir); d; d = readdir(dir)) {
    this->Internal->Files.emplace_back(d->d_name);
  }
  if (errno != 0) {
    if (errorMessage != nullptr) {
      *errorMessage = std::string(strerror(errno));
    }
    return Status::POSIX_errno();
  }

  this->Internal->Path = name;
  closedir(dir);
  return Status::Success();
}

unsigned long Directory::GetNumberOfFilesInDirectory(const std::string& name,
                                                     std::string* errorMessage)
{
  errno = 0;
  DIR* dir = opendir(name.c_str());

  if (!dir) {
    if (errorMessage != nullptr) {
      *errorMessage = std::string(strerror(errno));
    }
    return 0;
  }

  unsigned long count = 0;
  for (kwsys_dirent* d = readdir(dir); d; d = readdir(dir)) {
    count++;
  }
  if (errno != 0) {
    if (errorMessage != nullptr) {
      *errorMessage = std::string(strerror(errno));
    }
    return false;
  }

  closedir(dir);
  return count;
}

} // namespace KWSYS_NAMESPACE

#endif
