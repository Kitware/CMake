/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDebuggerPipeConnection.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace cmDebugger {

struct write_req_t
{
  uv_write_t req;
  uv_buf_t buf;
};

cmDebuggerPipeBase::cmDebuggerPipeBase(std::string name)
  : PipeName(std::move(name))
{
  Loop.init();
  LoopExit.init(
    *Loop, [](uv_async_t* handle) { uv_stop((uv_loop_t*)handle->data); },
    Loop);
  WriteEvent.init(
    *Loop,
    [](uv_async_t* handle) {
      auto* conn = static_cast<cmDebuggerPipeBase*>(handle->data);
      conn->WriteInternal();
    },
    this);
  PipeClose.init(
    *Loop,
    [](uv_async_t* handle) {
      auto* conn = static_cast<cmDebuggerPipeBase*>(handle->data);
      if (conn->Pipe.get()) {
        conn->Pipe->data = nullptr;
        conn->Pipe.reset();
      }
    },
    this);
}

void cmDebuggerPipeBase::WaitForConnection()
{
  std::unique_lock<std::mutex> lock(Mutex);
  Connected.wait(lock, [this] { return isOpen() || FailedToOpen; });
  if (FailedToOpen) {
    throw std::runtime_error("Failed to open debugger connection.");
  }
}

void cmDebuggerPipeBase::close()
{
  std::unique_lock<std::mutex> lock(Mutex);

  CloseConnection();
  PipeClose.send();
  lock.unlock();
  ReadReady.notify_all();
}

size_t cmDebuggerPipeBase::read(void* buffer, size_t n)
{
  std::unique_lock<std::mutex> lock(Mutex);
  ReadReady.wait(lock, [this] { return !isOpen() || !ReadBuffer.empty(); });

  if (!isOpen() && ReadBuffer.empty()) {
    return 0;
  }

  auto size = std::min(n, ReadBuffer.size());
  memcpy(buffer, ReadBuffer.data(), size);
  ReadBuffer.erase(0, size);
  return size;
}

bool cmDebuggerPipeBase::write(const void* buffer, size_t n)
{
  std::unique_lock<std::mutex> lock(Mutex);
  WriteBuffer.append(static_cast<const char*>(buffer), n);
  lock.unlock();
  WriteEvent.send();

  lock.lock();
  WriteComplete.wait(lock, [this] { return WriteBuffer.empty(); });
  return true;
}

void cmDebuggerPipeBase::StopLoop()
{
  LoopExit.send();

  if (LoopThread.joinable()) {
    LoopThread.join();
  }
}

void cmDebuggerPipeBase::BufferData(const std::string& data)
{
  std::unique_lock<std::mutex> lock(Mutex);
  ReadBuffer += data;
  lock.unlock();
  ReadReady.notify_all();
}

void cmDebuggerPipeBase::WriteInternal()
{
  std::unique_lock<std::mutex> lock(Mutex);
  auto n = WriteBuffer.length();
  assert(this->Pipe.get());
  write_req_t* req = new write_req_t;
  req->req.data = &WriteComplete;
  char* rawBuffer = new char[n];
  req->buf = uv_buf_init(rawBuffer, static_cast<unsigned int>(n));
  memcpy(req->buf.base, WriteBuffer.data(), n);
  WriteBuffer.clear();
  lock.unlock();

  uv_write(
    reinterpret_cast<uv_write_t*>(req), this->Pipe, &req->buf, 1,
    [](uv_write_t* cb_req, int status) {
      (void)status; // We need to free memory even if the write failed.
      write_req_t* wr = reinterpret_cast<write_req_t*>(cb_req);
      reinterpret_cast<std::condition_variable*>(wr->req.data)->notify_all();
      delete[] (wr->buf.base);
      delete wr;
    });

#ifdef __clang_analyzer__
  // Tell clang-analyzer that 'rawBuffer' does not leak.
  // We pass ownership to the closure.
  delete[] rawBuffer;
#endif
}

cmDebuggerPipeConnection::cmDebuggerPipeConnection(std::string name)
  : cmDebuggerPipeBase(std::move(name))
{
  ServerPipeClose.init(
    *Loop,
    [](uv_async_t* handle) {
      auto* conn = static_cast<cmDebuggerPipeConnection*>(handle->data);
      if (conn->ServerPipe.get()) {
        conn->ServerPipe->data = nullptr;
        conn->ServerPipe.reset();
      }
    },
    this);
}

cmDebuggerPipeConnection::~cmDebuggerPipeConnection()
{
  StopLoop();
}

bool cmDebuggerPipeConnection::StartListening(std::string& errorMessage)
{
  this->ServerPipe.init(*Loop, 0,
                        static_cast<cmDebuggerPipeConnection*>(this));

  int r;
  if ((r = uv_pipe_bind(this->ServerPipe, this->PipeName.c_str())) != 0) {
    errorMessage =
      "Internal Error with " + this->PipeName + ": " + uv_err_name(r);
    return false;
  }

  r = uv_listen(this->ServerPipe, 1, [](uv_stream_t* stream, int status) {
    if (status >= 0) {
      auto* conn = static_cast<cmDebuggerPipeConnection*>(stream->data);
      if (conn) {
        conn->Connect(stream);
      }
    }
  });

  if (r != 0) {
    errorMessage =
      "Internal Error listening on " + this->PipeName + ": " + uv_err_name(r);
    return false;
  }

  // Start the libuv event loop thread so that a client can connect.
  LoopThread = std::thread([this] { uv_run(Loop, UV_RUN_DEFAULT); });

  StartedListening.set_value();

  return true;
}

std::shared_ptr<dap::Reader> cmDebuggerPipeConnection::GetReader()
{
  return std::static_pointer_cast<dap::Reader>(shared_from_this());
}

std::shared_ptr<dap::Writer> cmDebuggerPipeConnection::GetWriter()
{
  return std::static_pointer_cast<dap::Writer>(shared_from_this());
}

bool cmDebuggerPipeConnection::isOpen()
{
  return this->Pipe.get() != nullptr;
}

void cmDebuggerPipeConnection::CloseConnection()
{
  ServerPipeClose.send();
}

void cmDebuggerPipeConnection::Connect(uv_stream_t* server)
{
  if (this->Pipe.get()) {
    // Accept and close all pipes but the first:
    cm::uv_pipe_ptr rejectPipe;

    rejectPipe.init(*Loop, 0);
    uv_accept(server, rejectPipe);

    return;
  }

  cm::uv_pipe_ptr ClientPipe;
  ClientPipe.init(*Loop, 0, static_cast<cmDebuggerPipeConnection*>(this));

  if (uv_accept(server, ClientPipe) != 0) {
    return;
  }

  StartReading<cmDebuggerPipeConnection>(ClientPipe);

  std::unique_lock<std::mutex> lock(Mutex);
  Pipe = std::move(ClientPipe);
  lock.unlock();
  Connected.notify_all();
}

cmDebuggerPipeClient::~cmDebuggerPipeClient()
{
  StopLoop();
}

void cmDebuggerPipeClient::Start()
{
  this->Pipe.init(*Loop, 0, static_cast<cmDebuggerPipeClient*>(this));

  uv_connect_t* connect = new uv_connect_t;
  connect->data = this;
  uv_pipe_connect(
    connect, Pipe, PipeName.c_str(), [](uv_connect_t* cb_connect, int status) {
      auto* conn = static_cast<cmDebuggerPipeClient*>(cb_connect->data);
      if (status >= 0) {
        conn->Connect();
      } else {
        conn->FailConnection();
      }
      delete cb_connect;
    });

  // Start the libuv event loop so that the pipe can connect.
  LoopThread = std::thread([this] { uv_run(Loop, UV_RUN_DEFAULT); });
}

bool cmDebuggerPipeClient::isOpen()
{
  return IsConnected;
}

void cmDebuggerPipeClient::CloseConnection()
{
  IsConnected = false;
}

void cmDebuggerPipeClient::Connect()
{
  StartReading<cmDebuggerPipeClient>(Pipe);
  std::unique_lock<std::mutex> lock(Mutex);
  IsConnected = true;
  lock.unlock();
  Connected.notify_all();
}

void cmDebuggerPipeClient::FailConnection()
{
  std::unique_lock<std::mutex> lock(Mutex);
  FailedToOpen = true;
  lock.unlock();
  Connected.notify_all();
}

} // namespace cmDebugger
