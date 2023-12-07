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

#include <windows.h>

#include <cm3p/cppdap/io.h>

#include "cmDebuggerAdapter.h"

namespace cmDebugger {

#ifdef _WIN32

class DuplexPipe_WIN32
{
public:
  DuplexPipe_WIN32(HANDLE read);
  ~DuplexPipe_WIN32();

  void close();
  size_t read(void* buffer, size_t n);
  bool write(void const* buffer, size_t n);

  bool WaitForConnection();

private:
  HANDLE hPipe;
  OVERLAPPED readOp;
  OVERLAPPED writeOp;
};

class cmDebuggerPipeConnection_WIN32
  : public dap::ReaderWriter
  , public cmDebuggerConnection
  , public std::enable_shared_from_this<cmDebuggerPipeConnection_WIN32>
{
public:
  cmDebuggerPipeConnection_WIN32(std::string name);
  ~cmDebuggerPipeConnection_WIN32() override;

  void WaitForConnection() override;

  bool StartListening(std::string& errorMessage) override;
  std::shared_ptr<dap::Reader> GetReader() override;
  std::shared_ptr<dap::Writer> GetWriter() override;

  // dap::ReaderWriter implementation

  bool isOpen() override;
  void close() override;
  size_t read(void* buffer, size_t n) override;
  bool write(void const* buffer, size_t n) override;

  // Used for unit test synchronization
  std::promise<void> StartedListening;

private:
  void CloseConnection();
  std::string GetErrorMessage(DWORD errorCode);

  std::string const PipeName;
  std::unique_ptr<DuplexPipe_WIN32> pipes;
};

using cmDebuggerPipeConnection = cmDebuggerPipeConnection_WIN32;

class cmDebuggerPipeClient_WIN32
  : public dap::ReaderWriter
  , public std::enable_shared_from_this<cmDebuggerPipeClient_WIN32>
{
public:
  cmDebuggerPipeClient_WIN32(std::string name);
  ~cmDebuggerPipeClient_WIN32();
  void WaitForConnection();

  bool isOpen() override;
  void close() override;
  size_t read(void* buffer, size_t n) override;
  bool write(void const* buffer, size_t n) override;

private:
  std::string GetErrorMessage(DWORD errorCode);

  std::string const PipeName;
  std::unique_ptr<DuplexPipe_WIN32> pipes;
};

using cmDebuggerPipeClient = cmDebuggerPipeClient_WIN32;

#endif // _WIN32

} // namespace cmDebugger
