/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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
  cmFileLockResult LockFunctionScope(const std::string& filename,
                                     unsigned long timeoutSec);
  cmFileLockResult LockFileScope(const std::string& filename,
                                 unsigned long timeoutSec);
  cmFileLockResult LockProcessScope(const std::string& filename,
                                    unsigned long timeoutSec);
  //@}

  /**
   * @brief Unlock the file explicitly.
   */
  cmFileLockResult Release(const std::string& filename);

private:
  bool IsAlreadyLocked(const std::string& filename) const;

  class ScopePool
  {
  public:
    ScopePool();
    ~ScopePool();

    ScopePool(ScopePool const&) = delete;
    ScopePool(ScopePool&&) noexcept;
    ScopePool& operator=(ScopePool const&) = delete;
    ScopePool& operator=(ScopePool&&) noexcept;

    cmFileLockResult Lock(const std::string& filename,
                          unsigned long timeoutSec);
    cmFileLockResult Release(const std::string& filename);
    bool IsAlreadyLocked(const std::string& filename) const;

  private:
    using List = std::vector<cmFileLock>;

    List Locks;
  };

  using List = std::vector<ScopePool>;

  List FunctionScopes;
  List FileScopes;
  ScopePool ProcessScope;
};
