/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <cerrno> // errno
#include <cstdio> // SEEK_SET

#include <fcntl.h>
#include <unistd.h>

#include "cmFileLock.h"
#include "cmSystemTools.h"

cmFileLock::cmFileLock() = default;

cmFileLockResult cmFileLock::Release()
{
  if (this->Filename.empty()) {
    return cmFileLockResult::MakeOk();
  }
  const int lockResult = this->LockFile(F_SETLK, F_UNLCK);

  this->Filename = "";

  ::close(this->File);
  this->File = -1;

  if (lockResult == 0) {
    return cmFileLockResult::MakeOk();
  }
  return cmFileLockResult::MakeSystem();
}

cmFileLockResult cmFileLock::OpenFile()
{
  this->File = ::open(this->Filename.c_str(), O_RDWR);
  if (this->File == -1) {
    return cmFileLockResult::MakeSystem();
  }
  return cmFileLockResult::MakeOk();
}

cmFileLockResult cmFileLock::LockWithoutTimeout()
{
  if (this->LockFile(F_SETLKW, F_WRLCK) == -1) {
    return cmFileLockResult::MakeSystem();
  }
  return cmFileLockResult::MakeOk();
}

cmFileLockResult cmFileLock::LockWithTimeout(unsigned long seconds)
{
  while (true) {
    if (this->LockFile(F_SETLK, F_WRLCK) == -1) {
      if (errno != EACCES && errno != EAGAIN) {
        return cmFileLockResult::MakeSystem();
      }
    } else {
      return cmFileLockResult::MakeOk();
    }
    if (seconds == 0) {
      return cmFileLockResult::MakeTimeout();
    }
    --seconds;
    cmSystemTools::Delay(1000);
  }
}

int cmFileLock::LockFile(int cmd, int type) const
{
  struct ::flock lock;
  lock.l_start = 0;
  lock.l_len = 0;                         // lock all bytes
  lock.l_pid = 0;                         // unused (for F_GETLK only)
  lock.l_type = static_cast<short>(type); // exclusive lock
  lock.l_whence = SEEK_SET;
  return ::fcntl(this->File, cmd, &lock);
}
