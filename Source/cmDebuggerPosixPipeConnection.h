/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <future>
#include <memory>
#include <string>

#include <cm3p/cppdap/io.h>

#include <sys/un.h>

#include "cmDebuggerAdapter.h"

namespace cmDebugger {

#ifndef _WIN32

class cmDebuggerPipeConnection_POSIX
  : public dap::ReaderWriter
  , public cmDebuggerConnection
  , public std::enable_shared_from_this<cmDebuggerPipeConnection_POSIX>
{
public:
  cmDebuggerPipeConnection_POSIX(std::string name);
  ~cmDebuggerPipeConnection_POSIX() override;

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
  void close_listen(); // release listen resources

  std::string const PipeName;
  sockaddr_un addr;
  int listen_fd = -1; // listen fd
  int rw_pipe = -1;   // rw fd
};

using cmDebuggerPipeConnection = cmDebuggerPipeConnection_POSIX;

class cmDebuggerPipeClient_POSIX
  : public dap::ReaderWriter
  , public std::enable_shared_from_this<cmDebuggerPipeClient_POSIX>
{
public:
  cmDebuggerPipeClient_POSIX(std::string name);
  ~cmDebuggerPipeClient_POSIX() override;
  void WaitForConnection();

  bool isOpen() override;
  void close() override;
  size_t read(void* buffer, size_t n) override;
  bool write(void const* buffer, size_t n) override;

private:
  std::string const PipeName;
  int rw_pipe = -1;
};

using cmDebuggerPipeClient = cmDebuggerPipeClient_POSIX;

#endif // !_WIN32

} // namespace cmDebugger
