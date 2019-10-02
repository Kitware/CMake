/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileTimes.h"

#include <utility>

#include <cm/memory>

#include "cm_sys_stat.h"

#if defined(_WIN32)
#  include <windows.h>

#  include "cmSystemTools.h"
#else
#  include <utime.h>
#endif

#if defined(_WIN32) &&                                                        \
  (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__MINGW32__))
#  include <io.h>
#endif

#ifdef _WIN32
class cmFileTimes::WindowsHandle
{
public:
  WindowsHandle(HANDLE h)
    : handle_(h)
  {
  }
  ~WindowsHandle()
  {
    if (this->handle_ != INVALID_HANDLE_VALUE) {
      CloseHandle(this->handle_);
    }
  }
  explicit operator bool() const
  {
    return this->handle_ != INVALID_HANDLE_VALUE;
  }
  bool operator!() const { return this->handle_ == INVALID_HANDLE_VALUE; }
  operator HANDLE() const { return this->handle_; }

private:
  HANDLE handle_;
};
#endif

class cmFileTimes::Times
{
public:
#if defined(_WIN32) && !defined(__CYGWIN__)
  FILETIME timeCreation;
  FILETIME timeLastAccess;
  FILETIME timeLastWrite;
#else
  struct utimbuf timeBuf;
#endif
};

cmFileTimes::cmFileTimes() = default;
cmFileTimes::cmFileTimes(std::string const& fileName)
{
  Load(fileName);
}
cmFileTimes::~cmFileTimes() = default;

bool cmFileTimes::Load(std::string const& fileName)
{
  std::unique_ptr<Times> ptr;
  if (IsValid()) {
    // Invalidate this and re-use times
    ptr.swap(this->times);
  } else {
    ptr = cm::make_unique<Times>();
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  cmFileTimes::WindowsHandle handle =
    CreateFileW(cmSystemTools::ConvertToWindowsExtendedPath(fileName).c_str(),
                GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS, 0);
  if (!handle) {
    return false;
  }
  if (!GetFileTime(handle, &ptr->timeCreation, &ptr->timeLastAccess,
                   &ptr->timeLastWrite)) {
    return false;
  }
#else
  struct stat st;
  if (stat(fileName.c_str(), &st) < 0) {
    return false;
  }
  ptr->timeBuf.actime = st.st_atime;
  ptr->timeBuf.modtime = st.st_mtime;
#endif
  // Accept times
  this->times = std::move(ptr);
  return true;
}

bool cmFileTimes::Store(std::string const& fileName) const
{
  if (!IsValid()) {
    return false;
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  cmFileTimes::WindowsHandle handle = CreateFileW(
    cmSystemTools::ConvertToWindowsExtendedPath(fileName).c_str(),
    FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
  if (!handle) {
    return false;
  }
  return SetFileTime(handle, &this->times->timeCreation,
                     &this->times->timeLastAccess,
                     &this->times->timeLastWrite) != 0;
#else
  return utime(fileName.c_str(), &this->times->timeBuf) >= 0;
#endif
}

bool cmFileTimes::Copy(std::string const& fromFile, std::string const& toFile)
{
  cmFileTimes fileTimes;
  return (fileTimes.Load(fromFile) && fileTimes.Store(toFile));
}
