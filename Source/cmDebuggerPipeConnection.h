/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <condition_variable>
#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <cm3p/cppdap/io.h>
#include <cm3p/uv.h>

#include "cmDebuggerAdapter.h"
#include "cmUVHandlePtr.h"

namespace cmDebugger {

class cmDebuggerPipeBase : public dap::ReaderWriter
{
public:
  cmDebuggerPipeBase(std::string name);

  void WaitForConnection();

  // dap::ReaderWriter implementation

  void close() final;
  size_t read(void* buffer, size_t n) final;
  bool write(const void* buffer, size_t n) final;

protected:
  virtual void CloseConnection(){};
  template <typename T>
  void StartReading(uv_stream_t* stream)
  {
    uv_read_start(
      stream,
      // alloc_cb
      [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        (void)handle;
        char* rawBuffer = new char[suggested_size];
        *buf =
          uv_buf_init(rawBuffer, static_cast<unsigned int>(suggested_size));
      },
      // read_cb
      [](uv_stream_t* readStream, ssize_t nread, const uv_buf_t* buf) {
        auto conn = static_cast<T*>(readStream->data);
        if (conn) {
          if (nread >= 0) {
            conn->BufferData(std::string(buf->base, buf->base + nread));
          } else {
            conn->close();
          }
        }
        delete[] (buf->base);
      });
  }
  void StopLoop();

  const std::string PipeName;
  std::thread LoopThread;
  cm::uv_loop_ptr Loop;
  cm::uv_pipe_ptr Pipe;
  std::mutex Mutex;
  std::condition_variable Connected;
  bool FailedToOpen = false;

private:
  void BufferData(const std::string& data);
  void WriteInternal();

  cm::uv_async_ptr LoopExit;
  cm::uv_async_ptr WriteEvent;
  cm::uv_async_ptr PipeClose;
  std::string WriteBuffer;
  std::string ReadBuffer;
  std::condition_variable ReadReady;
  std::condition_variable WriteComplete;
};

class cmDebuggerPipeConnection
  : public cmDebuggerPipeBase
  , public cmDebuggerConnection
  , public std::enable_shared_from_this<cmDebuggerPipeConnection>
{
public:
  cmDebuggerPipeConnection(std::string name);
  ~cmDebuggerPipeConnection() override;

  void WaitForConnection() override
  {
    cmDebuggerPipeBase::WaitForConnection();
  }

  bool StartListening(std::string& errorMessage) override;
  std::shared_ptr<dap::Reader> GetReader() override;
  std::shared_ptr<dap::Writer> GetWriter() override;

  // dap::ReaderWriter implementation

  bool isOpen() override;

  // Used for unit test synchronization
  std::promise<void> StartedListening;

private:
  void CloseConnection() override;
  void Connect(uv_stream_t* server);

  cm::uv_pipe_ptr ServerPipe;
  cm::uv_async_ptr ServerPipeClose;
};

class cmDebuggerPipeClient : public cmDebuggerPipeBase
{
public:
  using cmDebuggerPipeBase::cmDebuggerPipeBase;
  ~cmDebuggerPipeClient() override;

  void Start();

  // dap::ReaderWriter implementation

  bool isOpen() override;

private:
  void CloseConnection() override;
  void Connect();
  void FailConnection();

  bool IsConnected = false;
};

} // namespace cmDebugger
