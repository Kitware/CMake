/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmPipeConnection.h"

#include "cmServer.h"

cmPipeConnection::cmPipeConnection(const std::string& name,
                                   cmConnectionBufferStrategy* bufferStrategy)
  : cmEventBasedConnection(bufferStrategy)
  , PipeName(name)
{
}

void cmPipeConnection::Connect(uv_stream_t* server)
{
  if (this->ClientPipe) {
    // Accept and close all pipes but the first:
    uv_pipe_t* rejectPipe = new uv_pipe_t();

    uv_pipe_init(this->Server->GetLoop(), rejectPipe, 0);
    uv_accept(server, reinterpret_cast<uv_stream_t*>(rejectPipe));
    uv_close(reinterpret_cast<uv_handle_t*>(rejectPipe),
             &on_close_delete<uv_pipe_t>);
    return;
  }

  this->ClientPipe = new uv_pipe_t();
  uv_pipe_init(this->Server->GetLoop(), this->ClientPipe, 0);
  this->ClientPipe->data = static_cast<cmEventBasedConnection*>(this);
  auto client = reinterpret_cast<uv_stream_t*>(this->ClientPipe);
  if (uv_accept(server, client) != 0) {
    uv_close(reinterpret_cast<uv_handle_t*>(client),
             &on_close_delete<uv_pipe_t>);
    this->ClientPipe = nullptr;
    return;
  }
  this->ReadStream = client;
  this->WriteStream = client;

  uv_read_start(this->ReadStream, on_alloc_buffer, on_read);
  Server->OnConnected(this);
}

bool cmPipeConnection::OnServeStart(std::string* errorMessage)
{
  this->ServerPipe = new uv_pipe_t();
  uv_pipe_init(this->Server->GetLoop(), this->ServerPipe, 0);
  this->ServerPipe->data = static_cast<cmEventBasedConnection*>(this);

  int r;
  if ((r = uv_pipe_bind(this->ServerPipe, this->PipeName.c_str())) != 0) {
    *errorMessage = std::string("Internal Error with ") + this->PipeName +
      ": " + uv_err_name(r);
    return false;
  }
  auto serverStream = reinterpret_cast<uv_stream_t*>(this->ServerPipe);
  if ((r = uv_listen(serverStream, 1, on_new_connection)) != 0) {
    *errorMessage = std::string("Internal Error listening on ") +
      this->PipeName + ": " + uv_err_name(r);
    return false;
  }

  return cmConnection::OnServeStart(errorMessage);
}

bool cmPipeConnection::OnConnectionShuttingDown()
{
  if (this->ClientPipe) {
    uv_close(reinterpret_cast<uv_handle_t*>(this->ClientPipe),
             &on_close_delete<uv_pipe_t>);
    this->WriteStream->data = nullptr;
  }
  uv_close(reinterpret_cast<uv_handle_t*>(this->ServerPipe),
           &on_close_delete<uv_pipe_t>);

  this->ClientPipe = nullptr;
  this->ServerPipe = nullptr;
  this->WriteStream = nullptr;
  this->ReadStream = nullptr;

  return cmEventBasedConnection::OnConnectionShuttingDown();
}
