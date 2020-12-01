/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileTime.h"

#include <ctime>
#include <string>

// Use a platform-specific API to get file times efficiently.
#if !defined(_WIN32) || defined(__CYGWIN__)
#  include "cm_sys_stat.h"
#else
#  include <windows.h>

#  include "cmsys/Encoding.hxx"
#endif

bool cmFileTime::Load(std::string const& fileName)
{
#if !defined(_WIN32) || defined(__CYGWIN__)
  // POSIX version.  Use the stat function.
  struct stat fst;
  if (::stat(fileName.c_str(), &fst) != 0) {
    return false;
  }
#  if CMake_STAT_HAS_ST_MTIM
  // Nanosecond resolution
  this->Time = fst.st_mtim.tv_sec * UtPerS + fst.st_mtim.tv_nsec;
#  elif CMake_STAT_HAS_ST_MTIMESPEC
  // Nanosecond resolution
  this->Time = fst.st_mtimespec.tv_sec * UtPerS + fst.st_mtimespec.tv_nsec;
#  else
  // Second resolution
  this->Time = fst.st_mtime * UtPerS;
#  endif
#else
  // Windows version.  Get the modification time from extended file attributes.
  WIN32_FILE_ATTRIBUTE_DATA fdata;
  if (!GetFileAttributesExW(cmsys::Encoding::ToWide(fileName).c_str(),
                            GetFileExInfoStandard, &fdata)) {
    return false;
  }

  // Copy the file time to the output location.
  using uint64 = unsigned long long;

  this->Time = static_cast<TimeType>(
    (uint64(fdata.ftLastWriteTime.dwHighDateTime) << 32) +
    fdata.ftLastWriteTime.dwLowDateTime);
#endif
  return true;
}
