#include <functional>
#include <iostream>
#include <memory>

#include <cm3p/uv.h>

#include "cmGetPipes.h"
#include "cmUVHandlePtr.h"

static bool testBool()
{
  cm::uv_async_ptr async;
  cm::uv_handle_ptr handle;
  cm::uv_idle_ptr idle;
  cm::uv_pipe_ptr pipe;
  cm::uv_process_ptr process;
  cm::uv_signal_ptr signal;
  cm::uv_stream_ptr stream;
  cm::uv_timer_ptr timer;
  cm::uv_tty_ptr tty;
  return !async && !handle && !idle && !pipe && !process && !signal &&
    !stream && !timer && !tty;
}

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
  idle.start(cb);
  uv_run(loop, UV_RUN_DEFAULT);

  if (!idled) {
    std::cerr << "uv_idle_ptr did not trigger callback" << std::endl;
    return false;
  }

  idled = false;

  idle.start(cb);
  idle.stop();
  uv_run(loop, UV_RUN_DEFAULT);

  if (idled) {
    std::cerr << "uv_idle_ptr::stop did not stop callback" << std::endl;
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

  timed = false;
  timer.start(cb, 10, 0);
  timer.stop();
  uv_run(loop, UV_RUN_DEFAULT);

  if (timed) {
    std::cerr << "uv_timer_ptr::stop did not stop callback" << std::endl;
    return false;
  }

  return true;
}

static bool testWriteCallback()
{
  int pipe[] = { -1, -1 };
  if (cmGetPipes(pipe) < 0) {
    std::cout << "cmGetPipes() returned an error" << std::endl;
    return false;
  }

  cm::uv_loop_ptr loop;
  loop.init();

  cm::uv_pipe_ptr pipeRead;
  pipeRead.init(*loop, 0);
  uv_pipe_open(pipeRead, pipe[0]);

  cm::uv_pipe_ptr pipeWrite;
  pipeWrite.init(*loop, 0);
  uv_pipe_open(pipeWrite, pipe[1]);

  char c = '.';
  uv_buf_t buf = uv_buf_init(&c, sizeof(c));
  int status = -1;
  auto cb = std::make_shared<std::function<void(int)>>(
    [&status](int s) { status = s; });

  // Test getting a callback after the write is done.
  cm::uv_write(pipeWrite, &buf, 1, cb);
  uv_run(loop, UV_RUN_DEFAULT);
  if (status != 0) {
    std::cout << "cm::uv_write non-zero status: " << status << std::endl;
    return false;
  }

  // Test deleting the callback before it is made.
  status = -1;
  cm::uv_write(pipeWrite, &buf, 1, cb);
  cb.reset();
  uv_run(loop, UV_RUN_DEFAULT);
  if (status != -1) {
    std::cout << "cm::uv_write callback incorrectly called with status: "
              << status << std::endl;
    return false;
  }

  return true;
}

int testUVHandlePtr(int, char** const)
{
  bool passed = true;
  passed = testBool() && passed;
  passed = testIdle() && passed;
  passed = testTimer() && passed;
  passed = testWriteCallback() && passed;
  return passed ? 0 : -1;
}
