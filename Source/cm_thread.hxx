/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef CM_THREAD_HXX
#define CM_THREAD_HXX

#include "cmConfigure.h" // IWYU pragma: keep
#include "cm_uv.h"

namespace cm {

class shared_mutex
{
  uv_rwlock_t _M_;

public:
  shared_mutex() { uv_rwlock_init(&_M_); }
  ~shared_mutex() { uv_rwlock_destroy(&_M_); }

  shared_mutex(shared_mutex const&) = delete;
  shared_mutex& operator=(shared_mutex const&) = delete;

  void lock() { uv_rwlock_wrlock(&_M_); }
  void unlock() { uv_rwlock_wrunlock(&_M_); }

  void lock_shared() { uv_rwlock_rdlock(&_M_); }
  void unlock_shared() { uv_rwlock_rdunlock(&_M_); }
};

template <typename T>
class shared_lock
{
  T& _mutex;

public:
  shared_lock(T& m)
    : _mutex(m)
  {
    _mutex.lock_shared();
  }

  ~shared_lock() { _mutex.unlock_shared(); }

  shared_lock(shared_lock const&) = delete;
  shared_lock& operator=(shared_lock const&) = delete;
};
}

#endif
