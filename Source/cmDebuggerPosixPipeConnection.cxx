/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDebuggerPosixPipeConnection.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <utility>

#include <unistd.h>

#include <sys/socket.h>

namespace cmDebugger {

#ifndef _WIN32

cmDebuggerPipeConnection_POSIX::cmDebuggerPipeConnection_POSIX(
  std::string name)
  : PipeName(std::move(name))
{
  addr.sun_path[0] = '\0';
}

cmDebuggerPipeConnection_POSIX::~cmDebuggerPipeConnection_POSIX()
{
  if (isOpen()) {
    close();
  }
}

bool cmDebuggerPipeConnection_POSIX::StartListening(std::string& errorMessage)
{
  listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    errorMessage = "Failed to create socket: ";
    errorMessage += strerror(errno);
    return false;
  }

  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, PipeName.c_str(), sizeof(addr.sun_path));
  addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
  if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
    errorMessage = "Failed to bind name '";
    errorMessage += addr.sun_path;
    errorMessage += "' to socket: ";
    errorMessage += strerror(errno);
    close_listen();
    return false;
  }

  if (listen(listen_fd, 1) == -1) {
    errorMessage = "Failed to listen on socket: ";
    errorMessage += strerror(errno);
    close_listen();
    return false;
  }

  StartedListening.set_value();
  return true;
}

std::shared_ptr<dap::Reader> cmDebuggerPipeConnection_POSIX::GetReader()
{
  return std::static_pointer_cast<dap::Reader>(shared_from_this());
}

std::shared_ptr<dap::Writer> cmDebuggerPipeConnection_POSIX::GetWriter()
{
  return std::static_pointer_cast<dap::Writer>(shared_from_this());
}

bool cmDebuggerPipeConnection_POSIX::isOpen()
{
  return rw_pipe >= 0;
}

void cmDebuggerPipeConnection_POSIX::close()
{
  close_listen();
  ::close(rw_pipe);
  rw_pipe = -1;
}

void cmDebuggerPipeConnection_POSIX::close_listen()
{
  if (strlen(addr.sun_path) > 0) {
    unlink(addr.sun_path);
    addr.sun_path[0] = '\0';
  }
  ::close(listen_fd);
  listen_fd = -1;
}

void cmDebuggerPipeConnection_POSIX::WaitForConnection()
{
  sockaddr_un laddr;
  socklen_t len = sizeof(laddr);
  rw_pipe = accept(listen_fd, (sockaddr*)&laddr, &len);
  if (rw_pipe < 0) {
    close();
    return;
  }

  close_listen(); // no longer need the listen resources
}

size_t cmDebuggerPipeConnection_POSIX::read(void* buffer, size_t n)
{
  size_t result = 0;
  if (rw_pipe >= 0) {
    result = ::read(rw_pipe, buffer, n);
    if (result == 0) {
      close();
    }
  }

  return result;
}

bool cmDebuggerPipeConnection_POSIX::write(void const* buffer, size_t n)
{
  bool result = false;
  if (rw_pipe >= 0) {
    result = ::write(rw_pipe, buffer, n) >= 0;
    if (!result) {
      close();
    }
  }

  return result;
}

cmDebuggerPipeClient_POSIX::cmDebuggerPipeClient_POSIX(std::string name)
  : PipeName(std::move(name))
{
}

cmDebuggerPipeClient_POSIX::~cmDebuggerPipeClient_POSIX()
{
  close();
}

void cmDebuggerPipeClient_POSIX::WaitForConnection()
{
  rw_pipe = socket(AF_UNIX, SOCK_STREAM, 0);
  if (rw_pipe < 0) {
    throw std::runtime_error(std::string("Failed to create socket: ") +
                             strerror(errno));
  }

  sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, PipeName.c_str(), sizeof(addr.sun_path));
  addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
  if (connect(rw_pipe, (sockaddr*)&addr, sizeof(addr)) == -1) {
    close();
    throw std::runtime_error(
      std::string("Failed to connect path to socket: ") + strerror(errno));
  }
}

bool cmDebuggerPipeClient_POSIX::isOpen()
{
  return rw_pipe >= 0;
}

void cmDebuggerPipeClient_POSIX::close()
{
  if (isOpen()) {
    ::close(rw_pipe);
    rw_pipe = -1;
  }
}

size_t cmDebuggerPipeClient_POSIX::read(void* buffer, size_t n)
{
  int count = 0;
  if (isOpen()) {
    count = static_cast<int>(::read(rw_pipe, buffer, n));
    if (count == 0) {
      close();
    }
  }

  return count;
}

bool cmDebuggerPipeClient_POSIX::write(void const* buffer, size_t n)
{
  int count = 0;
  if (isOpen()) {
    count = static_cast<int>(::write(rw_pipe, buffer, n));
    if (count < 0) {
      close();
    }
  }

  return count > 0;
}

#endif // !_WIN32

} // namespace cmDebugger
