/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDebuggerWindowsPipeConnection.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace cmDebugger {

#ifdef _WIN32

DuplexPipe_WIN32::DuplexPipe_WIN32(HANDLE pipe)
  : hPipe(pipe)
{
  readOp.Offset = readOp.OffsetHigh = 0;
  readOp.hEvent = CreateEvent(NULL, true, false, NULL);
  writeOp.Offset = readOp.OffsetHigh = 0;
  writeOp.hEvent = CreateEvent(NULL, true, false, NULL);
}

DuplexPipe_WIN32::~DuplexPipe_WIN32()
{
  close();
}

size_t DuplexPipe_WIN32::read(void* buffer, size_t n)
{
  if (hPipe != INVALID_HANDLE_VALUE) {
    readOp.Offset = readOp.OffsetHigh = 0;
    ResetEvent(readOp.hEvent);
    auto r = ReadFile(hPipe, buffer, n, NULL, &readOp);
    auto err = GetLastError();
    if (r || err == ERROR_IO_PENDING) {
      DWORD nRead = 0;
      if (GetOverlappedResult(hPipe, &readOp, &nRead, true)) {
        return nRead;
      }
    }
  }

  return 0;
}

bool DuplexPipe_WIN32::write(void const* buffer, size_t n)
{
  if (hPipe != INVALID_HANDLE_VALUE) {
    writeOp.Offset = writeOp.OffsetHigh = 0;
    ResetEvent(writeOp.hEvent);
    auto w = WriteFile(hPipe, buffer, n, NULL, &writeOp);
    auto err = GetLastError();
    if (w || err == ERROR_IO_PENDING) {
      DWORD nWrite = 0;
      if (GetOverlappedResult(hPipe, &writeOp, &nWrite, true)) {
        return n == nWrite;
      }
    }
  }

  return false;
}

void DuplexPipe_WIN32::close()
{
  CloseHandle(hPipe);
  hPipe = INVALID_HANDLE_VALUE;
  CloseHandle(readOp.hEvent);
  CloseHandle(writeOp.hEvent);
  readOp.hEvent = writeOp.hEvent = INVALID_HANDLE_VALUE;
}

bool DuplexPipe_WIN32::WaitForConnection()
{
  auto connect = ConnectNamedPipe(hPipe, &readOp);
  auto err = GetLastError();
  if (!connect && err == ERROR_IO_PENDING) {
    DWORD ignored;
    if (GetOverlappedResult(hPipe, &readOp, &ignored, true)) {
      return true;
    }
  }

  return connect || err == ERROR_PIPE_CONNECTED;
}

cmDebuggerPipeConnection_WIN32::cmDebuggerPipeConnection_WIN32(
  std::string name)
  : PipeName(std::move(name))
  , pipes(nullptr)
{
}

cmDebuggerPipeConnection_WIN32::~cmDebuggerPipeConnection_WIN32()
{
  if (isOpen()) {
    pipes = nullptr;
  }
}

bool cmDebuggerPipeConnection_WIN32::StartListening(std::string& errorMessage)
{
  bool result = true;

  auto hPipe = CreateNamedPipeA(
    PipeName.c_str(),
    PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_REJECT_REMOTE_CLIENTS, 1,
    1024 * 16, 1024 * 16, NMPWAIT_USE_DEFAULT_WAIT, NULL);

  if (hPipe == INVALID_HANDLE_VALUE) {
    auto err = GetLastError();
    errorMessage = GetErrorMessage(err);
    result = false;
  }

  if (result) {
    pipes = std::make_unique<DuplexPipe_WIN32>(hPipe);
  }

  StartedListening.set_value();
  return result;
}

std::string cmDebuggerPipeConnection_WIN32::GetErrorMessage(DWORD errorCode)
{
  LPSTR message = nullptr;
  DWORD size = FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
    nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR)&message, 0, nullptr);
  std::string errorMessage = "Internal Error with " + this->PipeName + ": " +
    std::string(message, size);
  LocalFree(message);
  return errorMessage;
}

std::shared_ptr<dap::Reader> cmDebuggerPipeConnection_WIN32::GetReader()
{
  return std::static_pointer_cast<dap::Reader>(shared_from_this());
}

std::shared_ptr<dap::Writer> cmDebuggerPipeConnection_WIN32::GetWriter()
{
  return std::static_pointer_cast<dap::Writer>(shared_from_this());
}

bool cmDebuggerPipeConnection_WIN32::isOpen()
{
  return pipes != nullptr;
}

void cmDebuggerPipeConnection_WIN32::close()
{
  CloseConnection();
}

void cmDebuggerPipeConnection_WIN32::CloseConnection()
{
  if (isOpen()) {
    pipes->close();
    pipes = nullptr;
  }
}

void cmDebuggerPipeConnection_WIN32::WaitForConnection()
{
  if (!isOpen()) {
    return;
  }

  if (pipes->WaitForConnection()) {
    return;
  }

  CloseConnection();
}

size_t cmDebuggerPipeConnection_WIN32::read(void* buffer, size_t n)
{
  size_t result = 0;
  if (isOpen()) {
    result = pipes->read(buffer, n);
    if (result == 0) {
      CloseConnection();
    }
  }

  return result;
}

bool cmDebuggerPipeConnection_WIN32::write(void const* buffer, size_t n)
{
  bool result = false;
  if (isOpen()) {
    result = pipes->write(buffer, n);
    if (!result) {
      CloseConnection();
    }
  }

  return result;
}

cmDebuggerPipeClient_WIN32::cmDebuggerPipeClient_WIN32(std::string name)
  : PipeName(std::move(name))
{
}

cmDebuggerPipeClient_WIN32::~cmDebuggerPipeClient_WIN32()
{
  close();
}

void cmDebuggerPipeClient_WIN32::WaitForConnection()
{
  if (!isOpen()) {
    auto hPipe = CreateFileA(PipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                             NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
      auto err = GetLastError();
      throw std::runtime_error(std::string("CreateFile failed for pipe ") +
                               GetErrorMessage(err));
    }

    pipes = std::make_unique<DuplexPipe_WIN32>(hPipe);
  }
}

std::string cmDebuggerPipeClient_WIN32::GetErrorMessage(DWORD errorCode)
{
  LPSTR message = nullptr;
  DWORD size = FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
    nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR)&message, 0, nullptr);
  std::string errorMessage =
    this->PipeName + ": " + std::string(message, size);
  LocalFree(message);
  return errorMessage;
}

bool cmDebuggerPipeClient_WIN32::isOpen()
{
  return pipes != nullptr;
}

void cmDebuggerPipeClient_WIN32::close()
{
  if (isOpen()) {
    pipes->close();
    pipes = nullptr;
  }
}

size_t cmDebuggerPipeClient_WIN32::read(void* buffer, size_t n)
{
  size_t result = 0;
  if (isOpen()) {
    result = pipes->read(buffer, n);
    if (result == 0) {
      close();
    }
  }

  return result;
}

bool cmDebuggerPipeClient_WIN32::write(void const* buffer, size_t n)
{
  bool result = false;
  if (isOpen()) {
    result = pipes->write(buffer, n);
    if (!result) {
      close();
    }
  }

  return result;
}

#endif // _WIN32

} // namespace cmDebugger
