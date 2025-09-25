/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmFileLockPool.h"

#include <algorithm>
#include <cassert>
#include <utility>

#include "cmFileLock.h"
#include "cmFileLockResult.h"

cmFileLockPool::cmFileLockPool() = default;

cmFileLockPool::~cmFileLockPool() = default;

void cmFileLockPool::PushFunctionScope()
{
  this->FunctionScopes.emplace_back();
}

void cmFileLockPool::PopFunctionScope()
{
  assert(!this->FunctionScopes.empty());
  this->FunctionScopes.pop_back();
}

void cmFileLockPool::PushFileScope()
{
  this->FileScopes.emplace_back();
}

void cmFileLockPool::PopFileScope()
{
  assert(!this->FileScopes.empty());
  this->FileScopes.pop_back();
}

cmFileLockResult cmFileLockPool::LockFunctionScope(std::string const& filename,
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

cmFileLockResult cmFileLockPool::LockFileScope(std::string const& filename,
                                               unsigned long timeoutSec)
{
  if (this->IsAlreadyLocked(filename)) {
    return cmFileLockResult::MakeAlreadyLocked();
  }
  assert(!this->FileScopes.empty());
  return this->FileScopes.back().Lock(filename, timeoutSec);
}

cmFileLockResult cmFileLockPool::LockProcessScope(std::string const& filename,
                                                  unsigned long timeoutSec)
{
  if (this->IsAlreadyLocked(filename)) {
    return cmFileLockResult::MakeAlreadyLocked();
  }
  return this->ProcessScope.Lock(filename, timeoutSec);
}

cmFileLockResult cmFileLockPool::Release(std::string const& filename)
{
  for (auto& funcScope : this->FunctionScopes) {
    cmFileLockResult const result = funcScope.Release(filename);
    if (!result.IsOk()) {
      return result;
    }
  }

  for (auto& fileScope : this->FileScopes) {
    cmFileLockResult const result = fileScope.Release(filename);
    if (!result.IsOk()) {
      return result;
    }
  }

  return this->ProcessScope.Release(filename);
}

bool cmFileLockPool::IsAlreadyLocked(std::string const& filename) const
{
  for (auto const& funcScope : this->FunctionScopes) {
    bool const result = funcScope.IsAlreadyLocked(filename);
    if (result) {
      return true;
    }
  }

  for (auto const& fileScope : this->FileScopes) {
    bool const result = fileScope.IsAlreadyLocked(filename);
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

cmFileLockResult cmFileLockPool::ScopePool::Lock(std::string const& filename,
                                                 unsigned long timeoutSec)
{
  cmFileLock lock;
  cmFileLockResult const result = lock.Lock(filename, timeoutSec);
  if (result.IsOk()) {
    this->Locks.push_back(std::move(lock));
    return cmFileLockResult::MakeOk();
  }
  return result;
}

cmFileLockResult cmFileLockPool::ScopePool::Release(
  std::string const& filename)
{
  for (auto& lock : this->Locks) {
    if (lock.IsLocked(filename)) {
      return lock.Release();
    }
  }
  return cmFileLockResult::MakeOk();
}

bool cmFileLockPool::ScopePool::IsAlreadyLocked(
  std::string const& filename) const
{
  return std::any_of(this->Locks.begin(), this->Locks.end(),
                     [&filename](cmFileLock const& lock) -> bool {
                       return lock.IsLocked(filename);
                     });
}
