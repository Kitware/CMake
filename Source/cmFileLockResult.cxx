/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileLockResult.h"

#include <cerrno>
#include <cstring>

#ifdef _WIN32
#  include <Windows.h>
#endif

cmFileLockResult cmFileLockResult::MakeOk()
{
  return { OK, 0 };
}

cmFileLockResult cmFileLockResult::MakeSystem()
{
#if defined(_WIN32)
  const Error lastError = GetLastError();
#else
  const Error lastError = errno;
#endif
  return { SYSTEM, lastError };
}

cmFileLockResult cmFileLockResult::MakeTimeout()
{
  return { TIMEOUT, 0 };
}

cmFileLockResult cmFileLockResult::MakeAlreadyLocked()
{
  return { ALREADY_LOCKED, 0 };
}

cmFileLockResult cmFileLockResult::MakeInternal()
{
  return { INTERNAL, 0 };
}

cmFileLockResult cmFileLockResult::MakeNoFunction()
{
  return { NO_FUNCTION, 0 };
}

bool cmFileLockResult::IsOk() const
{
  return this->Type == OK;
}

std::string cmFileLockResult::GetOutputMessage() const
{
  switch (this->Type) {
    case OK:
      return "0";
    case SYSTEM:
#if defined(_WIN32)
    {
#  define WINMSG_BUF_LEN (1024)
      char winmsg[WINMSG_BUF_LEN];
      DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
      if (FormatMessageA(flags, nullptr, this->ErrorValue,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPSTR)winmsg, WINMSG_BUF_LEN, nullptr)) {
        const std::string message = winmsg;
        return message;
      } else {
        return "Internal error (FormatMessageA failed)";
      }
    }
#else
      return strerror(this->ErrorValue);
#endif
    case TIMEOUT:
      return "Timeout reached";
    case ALREADY_LOCKED:
      return "File already locked";
    case NO_FUNCTION:
      return "'GUARD FUNCTION' not used in function definition";
    case INTERNAL:
    default:
      return "Internal error";
  }
}

cmFileLockResult::cmFileLockResult(ErrorType typeValue, Error errorValue)
  : Type(typeValue)
  , ErrorValue(errorValue)
{
}
