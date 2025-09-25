/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <cm/memory>

#include <windows.h>

#include "cmFileLock.h"
#include "cmSystemTools.h"

static unsigned long const LOCK_LEN = static_cast<unsigned long>(-1);

cmFileLock::cmFileLock()
  : Overlapped(cm::make_unique<OVERLAPPED>())
{
  ZeroMemory(this->Overlapped.get(), sizeof(*this->Overlapped));
}

cmFileLockResult cmFileLock::Release()
{
  if (this->Filename.empty()) {
    return cmFileLockResult::MakeOk();
  }
  const DWORD reserved = 0;
  ZeroMemory(this->Overlapped.get(), sizeof(*this->Overlapped));

  const BOOL unlockResult =
    UnlockFileEx(File, reserved, LOCK_LEN, LOCK_LEN, this->Overlapped.get());

  this->Filename = "";

  CloseHandle(this->File);

  this->File = INVALID_HANDLE_VALUE;

  if (unlockResult) {
    return cmFileLockResult::MakeOk();
  } else {
    return cmFileLockResult::MakeSystem();
  }
}

cmFileLockResult cmFileLock::OpenFile()
{
  const DWORD access = GENERIC_READ | GENERIC_WRITE;
  const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  const PSECURITY_ATTRIBUTES security = nullptr;
  const DWORD attr = 0;
  const HANDLE templ = nullptr;
  this->File = CreateFileW(
    cmSystemTools::ConvertToWindowsExtendedPath(this->Filename).c_str(),
    access, shareMode, security, OPEN_EXISTING, attr, templ);
  if (this->File == INVALID_HANDLE_VALUE) {
    return cmFileLockResult::MakeSystem();
  } else {
    return cmFileLockResult::MakeOk();
  }
}

cmFileLockResult cmFileLock::LockWithoutTimeout()
{
  cmFileLockResult lock_result = cmFileLockResult::MakeOk();
  if (!this->LockFile(LOCKFILE_EXCLUSIVE_LOCK)) {
    lock_result = cmFileLockResult::MakeSystem();
  }
  CloseHandle(this->Overlapped->hEvent);
  return lock_result;
}

cmFileLockResult cmFileLock::LockWithTimeout(unsigned long seconds)
{
  cmFileLockResult lock_result = cmFileLockResult::MakeOk();
  const DWORD flags = LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY;
  bool in_time = true;
  while (in_time && !this->LockFile(flags)) {
    switch (GetLastError()) {
      case ERROR_INVALID_HANDLE:
        lock_result = cmFileLockResult::MakeSystem();
        break;
      case ERROR_LOCK_VIOLATION:
        if (seconds == 0) {
          in_time = false;
          lock_result = cmFileLockResult::MakeTimeout();
          continue;
        }
        --seconds;
        cmSystemTools::Delay(1000);
        continue;
      case ERROR_IO_PENDING:
        switch (
          WaitForSingleObject(this->Overlapped->hEvent, seconds * 1000)) {
          case WAIT_OBJECT_0:
            break;
          case WAIT_TIMEOUT:
            lock_result = cmFileLockResult::MakeTimeout();
            break;
          default:
            lock_result = cmFileLockResult::MakeSystem();
            break;
        }
        break;
      default:
        lock_result = cmFileLockResult::MakeSystem();
        break;
    }
  }
  CloseHandle(this->Overlapped->hEvent);
  return lock_result;
}

int cmFileLock::LockFile(int flags)
{
  const DWORD reserved = 0;

  this->Overlapped->hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (this->Overlapped->hEvent == nullptr) {
    return false;
  }
  return LockFileEx(this->File, flags, reserved, LOCK_LEN, LOCK_LEN,
                    this->Overlapped.get());
}
