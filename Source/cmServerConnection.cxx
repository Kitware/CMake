/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmServerConnection.h"

#include "cmConfigure.h"
#include "cmServer.h"
#include "cmServerDictionary.h"
#ifdef _WIN32
#include "io.h"
#else
#include <unistd.h>
#endif
#include <cassert>

cmStdIoConnection::cmStdIoConnection(
  cmConnectionBufferStrategy* bufferStrategy)
  : cmEventBasedConnection(bufferStrategy)
{
}

void cmStdIoConnection::SetupStream(uv_stream_t*& stream, int file_id)
{
  assert(stream == nullptr);
  switch (uv_guess_handle(file_id)) {
    case UV_TTY: {
      auto tty = new uv_tty_t();
      uv_tty_init(this->Server->GetLoop(), tty, file_id, file_id == 0);
      uv_tty_set_mode(tty, UV_TTY_MODE_NORMAL);
      stream = reinterpret_cast<uv_stream_t*>(tty);
      break;
    }
    case UV_FILE:
      if (file_id == 0) {
        return;
      }
      // Intentional fallthrough; stdin can _not_ be treated as a named
      // pipe, however stdout can be.
      CM_FALLTHROUGH;
    case UV_NAMED_PIPE: {
      auto pipe = new uv_pipe_t();
      uv_pipe_init(this->Server->GetLoop(), pipe, 0);
      uv_pipe_open(pipe, file_id);
      stream = reinterpret_cast<uv_stream_t*>(pipe);
      break;
    }
    default:
      assert(false && "Unable to determine stream type");
      return;
  }
  stream->data = static_cast<cmEventBasedConnection*>(this);
}

void cmStdIoConnection::SetServer(cmServerBase* s)
{
  cmConnection::SetServer(s);
  if (!s) {
    return;
  }

  SetupStream(this->ReadStream, 0);
  SetupStream(this->WriteStream, 1);
}

void shutdown_connection(uv_prepare_t* prepare)
{
  cmStdIoConnection* connection =
    reinterpret_cast<cmStdIoConnection*>(prepare->data);

  if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(prepare))) {
    uv_close(reinterpret_cast<uv_handle_t*>(prepare),
             &cmEventBasedConnection::on_close_delete<uv_prepare_t>);
  }
  connection->OnDisconnect(0);
}

bool cmStdIoConnection::OnServeStart(std::string* pString)
{
  Server->OnConnected(this);
  if (this->ReadStream) {
    uv_read_start(this->ReadStream, on_alloc_buffer, on_read);
  } else if (uv_guess_handle(0) == UV_FILE) {
    char buffer[1024];
    while (auto len = read(0, buffer, sizeof(buffer))) {
      ReadData(std::string(buffer, buffer + len));
    }

    // We can't start the disconnect from here, add a prepare hook to do that
    // for us
    auto prepare = new uv_prepare_t();
    prepare->data = this;
    uv_prepare_init(Server->GetLoop(), prepare);
    uv_prepare_start(prepare, shutdown_connection);
  }
  return cmConnection::OnServeStart(pString);
}

void cmStdIoConnection::ShutdownStream(uv_stream_t*& stream)
{
  if (!stream) {
    return;
  }
  switch (stream->type) {
    case UV_TTY: {
      assert(!uv_is_closing(reinterpret_cast<uv_handle_t*>(stream)));
      if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(stream))) {
        uv_close(reinterpret_cast<uv_handle_t*>(stream),
                 &on_close_delete<uv_tty_t>);
      }
      break;
    }
    case UV_FILE:
    case UV_NAMED_PIPE: {
      assert(!uv_is_closing(reinterpret_cast<uv_handle_t*>(stream)));
      if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(stream))) {
        uv_close(reinterpret_cast<uv_handle_t*>(stream),
                 &on_close_delete<uv_pipe_t>);
      }
      break;
    }
    default:
      assert(false && "Unable to determine stream type");
  }

  stream = nullptr;
}

bool cmStdIoConnection::OnConnectionShuttingDown()
{
  if (ReadStream) {
    uv_read_stop(ReadStream);
  }

  ShutdownStream(ReadStream);
  ShutdownStream(WriteStream);

  cmEventBasedConnection::OnConnectionShuttingDown();

  return true;
}

cmServerPipeConnection::cmServerPipeConnection(const std::string& name)
  : cmPipeConnection(name, new cmServerBufferStrategy)
{
}

cmServerStdIoConnection::cmServerStdIoConnection()
  : cmStdIoConnection(new cmServerBufferStrategy)
{
}

cmConnectionBufferStrategy::~cmConnectionBufferStrategy()
{
}

void cmConnectionBufferStrategy::clear()
{
}

std::string cmServerBufferStrategy::BufferOutMessage(
  const std::string& rawBuffer) const
{
  return std::string("\n") + kSTART_MAGIC + std::string("\n") + rawBuffer +
    kEND_MAGIC + std::string("\n");
}

std::string cmServerBufferStrategy::BufferMessage(std::string& RawReadBuffer)
{
  for (;;) {
    auto needle = RawReadBuffer.find('\n');

    if (needle == std::string::npos) {
      return "";
    }
    std::string line = RawReadBuffer.substr(0, needle);
    const auto ls = line.size();
    if (ls > 1 && line.at(ls - 1) == '\r') {
      line.erase(ls - 1, 1);
    }
    RawReadBuffer.erase(RawReadBuffer.begin(),
                        RawReadBuffer.begin() + static_cast<long>(needle) + 1);
    if (line == kSTART_MAGIC) {
      RequestBuffer.clear();
      continue;
    }
    if (line == kEND_MAGIC) {
      std::string rtn;
      rtn.swap(this->RequestBuffer);
      return rtn;
    }

    this->RequestBuffer += line;
    this->RequestBuffer += "\n";
  }
}
