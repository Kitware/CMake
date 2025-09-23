/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmFileLock.h"

class cmFileLockResult;

class cmFileLockPool
{
public:
  cmFileLockPool();
  ~cmFileLockPool();

  cmFileLockPool(cmFileLockPool const&) = delete;
  cmFileLockPool& operator=(cmFileLockPool const&) = delete;

  //@{
  /**
   * @brief Function scope control.
   */
  void PushFunctionScope();
  void PopFunctionScope();
  //@}

  //@{
  /**
   * @brief File scope control.
   */
  void PushFileScope();
  void PopFileScope();
  //@}

  //@{
  /**
   * @brief Lock the file in given scope.
   * @param timeoutSec Lock timeout. If -1 try until success or fatal error.
   */
  cmFileLockResult LockFunctionScope(std::string const& filename,
                                     unsigned long timeoutSec);
  cmFileLockResult LockFileScope(std::string const& filename,
                                 unsigned long timeoutSec);
  cmFileLockResult LockProcessScope(std::string const& filename,
                                    unsigned long timeoutSec);
  //@}

  /**
   * @brief Unlock the file explicitly.
   */
  cmFileLockResult Release(std::string const& filename);

private:
  bool IsAlreadyLocked(std::string const& filename) const;

  class ScopePool
  {
  public:
    ScopePool();
    ~ScopePool();

    ScopePool(ScopePool const&) = delete;
    ScopePool(ScopePool&&) noexcept;
    ScopePool& operator=(ScopePool const&) = delete;
    ScopePool& operator=(ScopePool&&) noexcept;

    cmFileLockResult Lock(std::string const& filename,
                          unsigned long timeoutSec);
    cmFileLockResult Release(std::string const& filename);
    bool IsAlreadyLocked(std::string const& filename) const;

  private:
    using List = std::vector<cmFileLock>;

    List Locks;
  };

  using List = std::vector<ScopePool>;

  List FunctionScopes;
  List FileScopes;
  ScopePool ProcessScope;
};
