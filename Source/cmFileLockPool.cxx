/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileLockPool.h"

#include <cassert>
#include <utility>

#include "cmFileLock.h"
#include "cmFileLockResult.h"

cmFileLockPool::cmFileLockPool() = default;

cmFileLockPool::~cmFileLockPool() = default;

void cmFileLockPool::PushFunctionScope()
{
  this->FunctionScopes.push_back(ScopePool());
}

void cmFileLockPool::PopFunctionScope()
{
  assert(!this->FunctionScopes.empty());
  this->FunctionScopes.pop_back();
}

void cmFileLockPool::PushFileScope()
{
  this->FileScopes.push_back(ScopePool());
}

void cmFileLockPool::PopFileScope()
{
  assert(!this->FileScopes.empty());
  this->FileScopes.pop_back();
}

cmFileLockResult cmFileLockPool::LockFunctionScope(const std::string& filename,
                                                   unsigned long timeoutSec)
{
  if (this->IsAlreadyLocked(filename)) {
    return cmFileLockResult::MakeAlreadyLocked();
  }
  if (this->FunctionScopes.empty()) {
    return cmFileLockResult::MakeNoFunction();
  }
  return this->FunctionScopes.back().Lock(filename, timeoutSec);
}

cmFileLockResult cmFileLockPool::LockFileScope(const std::string& filename,
                                               unsigned long timeoutSec)
{
  if (this->IsAlreadyLocked(filename)) {
    return cmFileLockResult::MakeAlreadyLocked();
  }
  assert(!this->FileScopes.empty());
  return this->FileScopes.back().Lock(filename, timeoutSec);
}

cmFileLockResult cmFileLockPool::LockProcessScope(const std::string& filename,
                                                  unsigned long timeoutSec)
{
  if (this->IsAlreadyLocked(filename)) {
    return cmFileLockResult::MakeAlreadyLocked();
  }
  return this->ProcessScope.Lock(filename, timeoutSec);
}

cmFileLockResult cmFileLockPool::Release(const std::string& filename)
{
  for (auto& funcScope : this->FunctionScopes) {
    const cmFileLockResult result = funcScope.Release(filename);
    if (!result.IsOk()) {
      return result;
    }
  }

  for (auto& fileScope : this->FileScopes) {
    const cmFileLockResult result = fileScope.Release(filename);
    if (!result.IsOk()) {
      return result;
    }
  }

  return this->ProcessScope.Release(filename);
}

bool cmFileLockPool::IsAlreadyLocked(const std::string& filename) const
{
  for (auto const& funcScope : this->FunctionScopes) {
    const bool result = funcScope.IsAlreadyLocked(filename);
    if (result) {
      return true;
    }
  }

  for (auto const& fileScope : this->FileScopes) {
    const bool result = fileScope.IsAlreadyLocked(filename);
    if (result) {
      return true;
    }
  }

  return this->ProcessScope.IsAlreadyLocked(filename);
}

cmFileLockPool::ScopePool::ScopePool() = default;

cmFileLockPool::ScopePool::~ScopePool() = default;

cmFileLockPool::ScopePool::ScopePool(ScopePool&&) noexcept = default;

cmFileLockPool::ScopePool& cmFileLockPool::ScopePool::operator=(
  ScopePool&& other) noexcept
{
  if (this != &other) {
    this->Locks = std::move(other.Locks);
  }

  return *this;
}

cmFileLockResult cmFileLockPool::ScopePool::Lock(const std::string& filename,
                                                 unsigned long timeoutSec)
{
  cmFileLock lock;
  const cmFileLockResult result = lock.Lock(filename, timeoutSec);
  if (result.IsOk()) {
    this->Locks.push_back(std::move(lock));
    return cmFileLockResult::MakeOk();
  }
  return result;
}

cmFileLockResult cmFileLockPool::ScopePool::Release(
  const std::string& filename)
{
  for (auto& lock : this->Locks) {
    if (lock.IsLocked(filename)) {
      return lock.Release();
    }
  }
  return cmFileLockResult::MakeOk();
}

bool cmFileLockPool::ScopePool::IsAlreadyLocked(
  const std::string& filename) const
{
  for (auto const& lock : this->Locks) {
    if (lock.IsLocked(filename)) {
      return true;
    }
  }
  return false;
}
