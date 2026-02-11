/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmStdIoInit.h"

#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <fcntl.h>

#ifdef _WIN32
#  include <windows.h>

#  include <io.h> // for _close, _dup2, _get_osfhandle

#  include "cm_fileno.hxx"
#else
#  include <unistd.h>
#endif

#include "cmStdIoStream.h"

namespace cm {
namespace StdIo {

namespace {

#ifdef _WIN32
void InitStdPipe(int stdFd, DWORD nStdHandle, FILE* stream,
                 wchar_t const* mode)
{
  if (cm_fileno(stream) >= 0) {
    return;
  }
  _close(stdFd);
  _wfreopen(L"NUL", mode, stream);
  int fd = cm_fileno(stream);
  if (fd < 0) {
    perror("failed to open NUL for missing stdio pipe");
    abort();
  }
  if (fd != stdFd) {
    _dup2(fd, stdFd);
  }
  SetStdHandle(nStdHandle, reinterpret_cast<HANDLE>(_get_osfhandle(fd)));
}
#else
void InitStdPipe(int fd)
{
  if (fcntl(fd, F_GETFD) != -1 || errno != EBADF) {
    return;
  }

  int f = open("/dev/null", fd == STDIN_FILENO ? O_RDONLY : O_WRONLY);
  if (f == -1) {
    perror("failed to open /dev/null for missing stdio pipe");
    abort();
  }
  if (f != fd) {
    dup2(f, fd);
    close(f);
  }
}
#endif

struct InitStdPipes
{
  InitStdPipes()
  {
#ifdef _WIN32
    InitStdPipe(0, STD_INPUT_HANDLE, stdin, L"rb");
    InitStdPipe(1, STD_OUTPUT_HANDLE, stdout, L"wb");
    InitStdPipe(2, STD_ERROR_HANDLE, stderr, L"wb");
#else
    InitStdPipe(STDIN_FILENO);
    InitStdPipe(STDOUT_FILENO);
    InitStdPipe(STDERR_FILENO);
#endif
  }
};

} // anonymous namespace

class Globals
{
public:
  std::ios::Init InitIos;
  InitStdPipes InitPipes;
  IStream StdIn{ std::cin, stdin };
  OStream StdOut{ std::cout, stdout };
  OStream StdErr{ std::cerr, stderr };

  Globals();

#ifdef _WIN32
  static BOOL WINAPI CtrlHandler(DWORD /*dwCtrlType*/)
  {
    Get().StdErr.Destroy();
    Get().StdOut.Destroy();
    Get().StdIn.Destroy();
    return FALSE;
  }
#endif

  static Globals& Get();
};

Globals::Globals()
{
#ifdef _WIN32
  // On Windows, setlocale offers a ".<code-page>" syntax to select the
  // user's locale with a specific character set.  We always use UTF-8.
  std::setlocale(LC_CTYPE, ".UTF-8");

  SetConsoleCtrlHandler(CtrlHandler, TRUE);
#else
  // On non-Windows platforms, we select the user's locale.
  std::setlocale(LC_CTYPE, "");
#endif
}

Globals& Globals::Get()
{
  static Globals globals;
  return globals;
}

Init::Init()
{
  Globals::Get();
}

IStream& In()
{
  return Globals::Get().StdIn;
}

OStream& Out()
{
  return Globals::Get().StdOut;
}

OStream& Err()
{
  return Globals::Get().StdErr;
}

}
}
