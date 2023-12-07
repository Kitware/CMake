/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileLock.h"

#include <cassert>
#include <utility>

#include "cmFileLockResult.h"

// Common implementation

cmFileLock::cmFileLock(cmFileLock&& other) noexcept
{
  this->File = other.File;
  other.File = (decltype(other.File))-1;
  this->Filename = std::move(other.Filename);
}

cmFileLock::~cmFileLock()
{
  if (!this->Filename.empty()) {
    const cmFileLockResult result = this->Release();
    static_cast<void>(result);
    assert(result.IsOk());
  }
}

cmFileLock& cmFileLock::operator=(cmFileLock&& other) noexcept
{
  this->File = other.File;
  other.File = (decltype(other.File))-1;
  this->Filename = std::move(other.Filename);

  return *this;
}

cmFileLockResult cmFileLock::Lock(const std::string& filename,
                                  unsigned long timeout)
{
  if (filename.empty()) {
    // Error is internal since all the directories and file must be created
    // before actual lock called.
    return cmFileLockResult::MakeInternal();
  }

  if (!this->Filename.empty()) {
    // Error is internal since double-lock must be checked in class
    // cmFileLockPool by the cmFileLock::IsLocked method.
    return cmFileLockResult::MakeInternal();
  }

  this->Filename = filename;
  cmFileLockResult result = this->OpenFile();
  if (result.IsOk()) {
    if (timeout == static_cast<unsigned long>(-1)) {
      result = this->LockWithoutTimeout();
    } else {
      result = this->LockWithTimeout(timeout);
    }
  }

  if (!result.IsOk()) {
    this->Filename.clear();
  }

  return result;
}

bool cmFileLock::IsLocked(const std::string& filename) const
{
  return filename == this->Filename;
}

#if defined(_WIN32)
// NOLINTNEXTLINE(bugprone-suspicious-include)
#  include "cmFileLockWin32.cxx"
#else
// NOLINTNEXTLINE(bugprone-suspicious-include)
#  include "cmFileLockUnix.cxx"
#endif
