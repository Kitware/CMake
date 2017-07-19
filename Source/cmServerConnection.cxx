/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmServerConnection.h"

#include "cmServer.h"
#include "cmServerDictionary.h"

cmStdIoConnection::cmStdIoConnection(
  cmConnectionBufferStrategy* bufferStrategy)
  : cmEventBasedConnection(bufferStrategy)
  , Input()
  , Output()
{
}

void cmStdIoConnection::SetServer(cmServerBase* s)
{
  cmConnection::SetServer(s);

  if (uv_guess_handle(1) == UV_TTY) {
    usesTty = true;

    this->Input.tty = new uv_tty_t();
    uv_tty_init(this->Server->GetLoop(), this->Input.tty, 0, 1);
    uv_tty_set_mode(this->Input.tty, UV_TTY_MODE_NORMAL);
    this->Input.tty->data = static_cast<cmEventBasedConnection*>(this);
    this->ReadStream = reinterpret_cast<uv_stream_t*>(this->Input.tty);

    this->Output.tty = new uv_tty_t();
    uv_tty_init(this->Server->GetLoop(), this->Output.tty, 1, 0);
    uv_tty_set_mode(this->Output.tty, UV_TTY_MODE_NORMAL);
    this->Output.tty->data = static_cast<cmEventBasedConnection*>(this);
    this->WriteStream = reinterpret_cast<uv_stream_t*>(this->Output.tty);
  } else {
    usesTty = false;

    this->Input.pipe = new uv_pipe_t();
    uv_pipe_init(this->Server->GetLoop(), this->Input.pipe, 0);
    uv_pipe_open(this->Input.pipe, 0);
    this->Input.pipe->data = static_cast<cmEventBasedConnection*>(this);
    this->ReadStream = reinterpret_cast<uv_stream_t*>(this->Input.pipe);

    this->Output.pipe = new uv_pipe_t();
    uv_pipe_init(this->Server->GetLoop(), this->Output.pipe, 0);
    uv_pipe_open(this->Output.pipe, 1);
    this->Output.pipe->data = static_cast<cmEventBasedConnection*>(this);
    this->WriteStream = reinterpret_cast<uv_stream_t*>(this->Output.pipe);
  }
}

bool cmStdIoConnection::OnServeStart(std::string* pString)
{
  uv_read_start(this->ReadStream, on_alloc_buffer, on_read);
  Server->OnConnected(this);
  return cmConnection::OnServeStart(pString);
}

bool cmStdIoConnection::OnConnectionShuttingDown()
{
  cmEventBasedConnection::OnConnectionShuttingDown();

  if (usesTty) {
    uv_read_stop(reinterpret_cast<uv_stream_t*>(this->Input.tty));
    uv_close(reinterpret_cast<uv_handle_t*>(this->Input.tty),
             &on_close_delete<uv_tty_t>);
    uv_close(reinterpret_cast<uv_handle_t*>(this->Output.tty),
             &on_close_delete<uv_tty_t>);
  } else {
    uv_close(reinterpret_cast<uv_handle_t*>(this->Input.pipe),
             &on_close_delete<uv_pipe_t>);
    uv_close(reinterpret_cast<uv_handle_t*>(this->Output.pipe),
             &on_close_delete<uv_pipe_t>);
  }

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
