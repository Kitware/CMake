/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#if defined(_WIN32)
#  include <memory>
using HANDLE = void*;
#endif

class cmFileLockResult;

/**
 * @brief Cross-platform file locking.
 * @details Under the hood this class use 'fcntl' for Unix-like platforms and
 * 'LockFileEx'/'UnlockFileEx' for Win32 platform. Locks are exclusive and
 * advisory.
 */
class cmFileLock
{
public:
  cmFileLock();
  ~cmFileLock();

  cmFileLock(cmFileLock const&) = delete;
  cmFileLock(cmFileLock&&) noexcept;
  cmFileLock& operator=(cmFileLock const&) = delete;
  cmFileLock& operator=(cmFileLock&&) noexcept;

  /**
   * @brief Lock the file.
   * @param timeoutSec Lock timeout. If -1 try until success or fatal error.
   */
  cmFileLockResult Lock(std::string const& filename, unsigned long timeoutSec);

  /**
   * @brief Unlock the file.
   */
  cmFileLockResult Release();

  /**
   * @brief Check file is locked by this class.
   * @details This function helps to find double locks (deadlocks) and to do
   * explicit unlocks.
   */
  bool IsLocked(std::string const& filename) const;

private:
  cmFileLockResult OpenFile();
  cmFileLockResult LockWithoutTimeout();
  cmFileLockResult LockWithTimeout(unsigned long timeoutSec);

#if defined(_WIN32)
  HANDLE File = (HANDLE)-1;
  std::unique_ptr<struct _OVERLAPPED> Overlapped;
  int LockFile(int flags);
#else
  int File = -1;
  int LockFile(int cmd, int type) const;
#endif

  std::string Filename;
};
