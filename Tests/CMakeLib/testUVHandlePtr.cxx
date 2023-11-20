#include <iostream>

#include <cm3p/uv.h>

#include "cmUVHandlePtr.h"

static bool testIdle()
{
  bool idled = false;

  cm::uv_loop_ptr loop;
  loop.init();

  cm::uv_idle_ptr idle;
  idle.init(*loop, &idled);
  uv_idle_start(idle, [](uv_idle_t* handle) {
    auto idledPtr = static_cast<bool*>(handle->data);
    *idledPtr = true;
    uv_idle_stop(handle);
  });
  uv_run(loop, UV_RUN_DEFAULT);

  if (!idled) {
    std::cerr << "uv_idle_ptr did not trigger callback" << std::endl;
    return false;
  }

  return true;
}

int testUVHandlePtr(int, char** const)
{
  bool passed = true;
  passed = testIdle() && passed;
  return passed ? 0 : -1;
}
