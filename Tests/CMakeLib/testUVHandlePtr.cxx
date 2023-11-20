#include <iostream>

#include <cm3p/uv.h>

#include "cmUVHandlePtr.h"

static bool testIdle()
{
  bool idled = false;

  cm::uv_loop_ptr loop;
  loop.init();

  auto cb = [](uv_idle_t* handle) {
    auto idledPtr = static_cast<bool*>(handle->data);
    *idledPtr = true;
    uv_idle_stop(handle);
  };

  cm::uv_idle_ptr idle;
  idle.init(*loop, &idled);
  uv_idle_start(idle, cb);
  uv_run(loop, UV_RUN_DEFAULT);

  if (!idled) {
    std::cerr << "uv_idle_ptr did not trigger callback" << std::endl;
    return false;
  }

  return true;
}

static bool testTimer()
{
  bool timed = false;

  cm::uv_loop_ptr loop;
  loop.init();

  auto cb = [](uv_timer_t* handle) {
    auto timedPtr = static_cast<bool*>(handle->data);
    *timedPtr = true;
    uv_timer_stop(handle);
  };

  cm::uv_timer_ptr timer;
  timer.init(*loop, &timed);
  timer.start(cb, 10, 0);
  uv_run(loop, UV_RUN_DEFAULT);

  if (!timed) {
    std::cerr << "uv_timer_ptr did not trigger callback" << std::endl;
    return false;
  }

  return true;
}

int testUVHandlePtr(int, char** const)
{
  bool passed = true;
  passed = testIdle() && passed;
  passed = testTimer() && passed;
  return passed ? 0 : -1;
}
