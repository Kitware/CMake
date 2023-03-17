/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Status.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#  include "Status.hxx.in"
#endif

#include <cerrno>
#include <iostream>

#ifdef _WIN32
#  include <windows.h>
#endif

int testStatus(int, char*[])
{
  bool res = true;
  {
    kwsys::Status status;
    if (status.GetKind() != kwsys::Status::Kind::Success) {
      std::cerr << "Status default constructor does not produce Success\n";
      res = false;
    }

    status = kwsys::Status::Success();
    if (status.GetKind() != kwsys::Status::Kind::Success) {
      std::cerr << "Status Success constructor does not produce Success\n";
      res = false;
    }
    if (!status.IsSuccess()) {
      std::cerr << "Status Success gives false IsSuccess\n";
      res = false;
    }
    if (!status) {
      std::cerr << "Status Success kind is not true\n";
      res = false;
    }
    if (status.GetPOSIX() != 0) {
      std::cerr << "Status Success kind does not return POSIX 0\n";
      res = false;
    }
#ifdef _WIN32
    if (status.GetWindows() != 0) {
      std::cerr << "Status Success kind does not return Windows 0\n";
      res = false;
    }
#endif
    if (status.GetString() != "Success") {
      std::cerr << "Status Success kind does not return \"Success\" string\n";
      res = false;
    }

    status = kwsys::Status::POSIX(EINVAL);
    if (status.GetKind() != kwsys::Status::Kind::POSIX) {
      std::cerr << "Status POSIX constructor does not produce POSIX\n";
      res = false;
    }
    if (status.IsSuccess()) {
      std::cerr << "Status POSIX gives true IsSuccess\n";
      res = false;
    }
    if (status) {
      std::cerr << "Status POSIX kind is not false\n";
      res = false;
    }
    if (status.GetPOSIX() != EINVAL) {
      std::cerr << "Status POSIX kind does not preserve POSIX value\n";
      res = false;
    }
#ifdef _WIN32
    if (status.GetWindows() != 0) {
      std::cerr << "Status POSIX kind does not return Windows 0\n";
      res = false;
    }
#endif
    if (status.GetString().empty()) {
      std::cerr << "Status POSIX kind returns empty string\n";
      res = false;
    }
    errno = ENOENT;
    status = kwsys::Status::POSIX_errno();
    if (status.GetPOSIX() != ENOENT) {
      std::cerr << "Status POSIX_errno did not use errno\n";
      res = false;
    }
    errno = 0;

#ifdef _WIN32
    status = kwsys::Status::Windows(ERROR_INVALID_PARAMETER);
    if (status.GetKind() != kwsys::Status::Kind::Windows) {
      std::cerr << "Status Windows constructor does not produce Windows\n";
      res = false;
    }
    if (status.IsSuccess()) {
      std::cerr << "Status Windows gives true IsSuccess\n";
      res = false;
    }
    if (status) {
      std::cerr << "Status Windows kind is not false\n";
      res = false;
    }
    if (status.GetWindows() != ERROR_INVALID_PARAMETER) {
      std::cerr << "Status Windows kind does not preserve Windows value\n";
      res = false;
    }
    if (status.GetPOSIX() != 0) {
      std::cerr << "Status Windows kind does not return POSIX 0\n";
      res = false;
    }
    if (status.GetString().empty()) {
      std::cerr << "Status Windows kind returns empty string\n";
      res = false;
    }

    SetLastError(ERROR_FILE_NOT_FOUND);
    status = kwsys::Status::Windows_GetLastError();
    if (status.GetWindows() != ERROR_FILE_NOT_FOUND) {
      std::cerr << "Status Windows_GetLastError did not use GetLastError()\n";
      res = false;
    }
    SetLastError(ERROR_SUCCESS);
#endif
  }
  return res ? 0 : 1;
}
