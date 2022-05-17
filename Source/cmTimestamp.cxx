/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#if !defined(_WIN32) && !defined(__sun) && !defined(__OpenBSD__)
// POSIX APIs are needed
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _POSIX_C_SOURCE 200809L
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__QNX__)
// For isascii
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#  define _XOPEN_SOURCE 700
#endif

#include "cmTimestamp.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <utility>

#ifdef __MINGW32__
#  include <libloaderapi.h>
#endif

#include <cm3p/uv.h>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

std::string cmTimestamp::CurrentTime(const std::string& formatString,
                                     bool utcFlag) const
{
  // get current time with microsecond resolution
  uv_timeval64_t timeval;
  uv_gettimeofday(&timeval);
  auto currentTimeT = static_cast<time_t>(timeval.tv_sec);
  auto microseconds = static_cast<uint32_t>(timeval.tv_usec);

  // check for override via SOURCE_DATE_EPOCH for reproducible builds
  std::string source_date_epoch;
  cmSystemTools::GetEnv("SOURCE_DATE_EPOCH", source_date_epoch);
  if (!source_date_epoch.empty()) {
    std::istringstream iss(source_date_epoch);
    iss >> currentTimeT;
    if (iss.fail() || !iss.eof()) {
      cmSystemTools::Error("Cannot parse SOURCE_DATE_EPOCH as integer");
      exit(27);
    }
    // SOURCE_DATE_EPOCH has only a resolution in the seconds range
    microseconds = 0;
  }
  if (currentTimeT == static_cast<time_t>(-1)) {
    return std::string();
  }

  return this->CreateTimestampFromTimeT(currentTimeT, microseconds,
                                        formatString, utcFlag);
}

std::string cmTimestamp::FileModificationTime(const char* path,
                                              const std::string& formatString,
                                              bool utcFlag) const
{
  std::string real_path =
    cmSystemTools::GetRealPathResolvingWindowsSubst(path);

  if (!cmsys::SystemTools::FileExists(real_path)) {
    return std::string();
  }

  // use libuv's implementation of stat(2) to get the file information
  time_t mtime = 0;
  uint32_t microseconds = 0;
  uv_fs_t req;
  if (uv_fs_stat(nullptr, &req, real_path.c_str(), nullptr) == 0) {
    mtime = static_cast<time_t>(req.statbuf.st_mtim.tv_sec);
    // tv_nsec has nanosecond resolution, but we truncate it to microsecond
    // resolution in order to be consistent with cmTimestamp::CurrentTime()
    microseconds = static_cast<uint32_t>(req.statbuf.st_mtim.tv_nsec / 1000);
  }
  uv_fs_req_cleanup(&req);

  return this->CreateTimestampFromTimeT(mtime, microseconds, formatString,
                                        utcFlag);
}

std::string cmTimestamp::CreateTimestampFromTimeT(time_t timeT,
                                                  std::string formatString,
                                                  bool utcFlag) const
{
  return this->CreateTimestampFromTimeT(timeT, 0, std::move(formatString),
                                        utcFlag);
}

std::string cmTimestamp::CreateTimestampFromTimeT(time_t timeT,
                                                  const uint32_t microseconds,
                                                  std::string formatString,
                                                  bool utcFlag) const
{
  if (formatString.empty()) {
    formatString = "%Y-%m-%dT%H:%M:%S";
    if (utcFlag) {
      formatString += "Z";
    }
  }

  struct tm timeStruct;
  memset(&timeStruct, 0, sizeof(timeStruct));

  struct tm* ptr = nullptr;
  if (utcFlag) {
    ptr = gmtime(&timeT);
  } else {
    ptr = localtime(&timeT);
  }

  if (ptr == nullptr) {
    return std::string();
  }

  timeStruct = *ptr;

  std::string result;
  for (std::string::size_type i = 0; i < formatString.size(); ++i) {
    char c1 = formatString[i];
    char c2 = (i + 1 < formatString.size()) ? formatString[i + 1]
                                            : static_cast<char>(0);

    if (c1 == '%' && c2 != 0) {
      result +=
        this->AddTimestampComponent(c2, timeStruct, timeT, microseconds);
      ++i;
    } else {
      result += c1;
    }
  }

  return result;
}

time_t cmTimestamp::CreateUtcTimeTFromTm(struct tm& tm) const
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
  return _mkgmtime(&tm);
#else
  // From Linux timegm() manpage.

  std::string tz_old;
  bool const tz_was_set = cmSystemTools::GetEnv("TZ", tz_old);
  tz_old = "TZ=" + tz_old;

  // The standard says that "TZ=" or "TZ=[UNRECOGNIZED_TZ]" means UTC.
  // It seems that "TZ=" does NOT work, at least under Windows
  // with neither MSVC nor MinGW, so let's use explicit "TZ=UTC"

  cmSystemTools::PutEnv("TZ=UTC");

  tzset();

  time_t result = mktime(&tm);

#  ifndef CMAKE_BOOTSTRAP
  if (tz_was_set) {
    cmSystemTools::PutEnv(tz_old);
  } else {
    cmSystemTools::UnsetEnv("TZ");
  }
#  else
  // No UnsetEnv during bootstrap.  This is good enough for CMake itself.
  cmSystemTools::PutEnv(tz_old);
  static_cast<void>(tz_was_set);
#  endif

  tzset();

  return result;
#endif
}

std::string cmTimestamp::AddTimestampComponent(
  char flag, struct tm& timeStruct, const time_t timeT,
  const uint32_t microseconds) const
{
  std::string formatString = cmStrCat('%', flag);

  switch (flag) {
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'd':
    case 'H':
    case 'I':
    case 'j':
    case 'm':
    case 'M':
    case 'S':
    case 'U':
    case 'V':
    case 'w':
    case 'y':
    case 'Y':
    case '%':
      break;
    case 's': // Seconds since UNIX epoch (midnight 1-jan-1970)
    {
      // Build a time_t for UNIX epoch and subtract from the input "timeT":
      struct tm tmUnixEpoch;
      memset(&tmUnixEpoch, 0, sizeof(tmUnixEpoch));
      tmUnixEpoch.tm_mday = 1;
      tmUnixEpoch.tm_year = 1970 - 1900;

      const time_t unixEpoch = this->CreateUtcTimeTFromTm(tmUnixEpoch);
      if (unixEpoch == -1) {
        cmSystemTools::Error(
          "Error generating UNIX epoch in string(TIMESTAMP ...) or "
          "file(TIMESTAMP ...). Please, file a bug report against CMake");
        return std::string();
      }

      return std::to_string(static_cast<long int>(difftime(timeT, unixEpoch)));
    }
    case 'f': // microseconds
    {
      // clip number to 6 digits and pad with leading zeros
      std::string microsecs = std::to_string(microseconds % 1000000);
      return std::string(6 - microsecs.length(), '0') + microsecs;
    }
    default: {
      return formatString;
    }
  }

  char buffer[16];

#ifdef __MINGW32__
  /* See a bug in MinGW: https://sourceforge.net/p/mingw-w64/bugs/793/. A work
   * around is to try to use strftime() from ucrtbase.dll. */
  using T = size_t(__cdecl*)(char*, size_t, const char*, const struct tm*);
  auto loadUcrtStrftime = []() -> T {
    auto handle =
      LoadLibraryExA("ucrtbase.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (handle) {
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
      return reinterpret_cast<T>(GetProcAddress(handle, "strftime"));
#  pragma GCC diagnostic pop
    }
    return nullptr;
  };
  static T ucrtStrftime = loadUcrtStrftime();

  if (ucrtStrftime) {
    size_t size =
      ucrtStrftime(buffer, sizeof(buffer), formatString.c_str(), &timeStruct);
    return std::string(buffer, size);
  }
#endif

  size_t size =
    strftime(buffer, sizeof(buffer), formatString.c_str(), &timeStruct);

  return std::string(buffer, size);
}
