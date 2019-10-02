/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetPipes.h"

#include <fcntl.h>

#include "cm_uv.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <io.h>

int cmGetPipes(int* fds)
{
  SECURITY_ATTRIBUTES attr;
  HANDLE readh, writeh;
  attr.nLength = sizeof(attr);
  attr.lpSecurityDescriptor = nullptr;
  attr.bInheritHandle = FALSE;
  if (!CreatePipe(&readh, &writeh, &attr, 0))
    return uv_translate_sys_error(GetLastError());
  fds[0] = _open_osfhandle((intptr_t)readh, 0);
  fds[1] = _open_osfhandle((intptr_t)writeh, 0);
  if (fds[0] == -1 || fds[1] == -1) {
    CloseHandle(readh);
    CloseHandle(writeh);
    return uv_translate_sys_error(GetLastError());
  }
  return 0;
}
#else
#  include <cerrno>

#  include <unistd.h>

int cmGetPipes(int* fds)
{
  if (pipe(fds) == -1) {
    return uv_translate_sys_error(errno);
  }

  if (fcntl(fds[0], F_SETFD, FD_CLOEXEC) == -1 ||
      fcntl(fds[1], F_SETFD, FD_CLOEXEC) == -1) {
    close(fds[0]);
    close(fds[1]);
    return uv_translate_sys_error(errno);
  }
  return 0;
}
#endif
