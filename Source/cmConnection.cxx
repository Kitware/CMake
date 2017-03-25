/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConnection.h"

#include "cmServer.h"
#include "cm_uv.h"

#include <cassert>
#include <cstring>

struct write_req_t
{
  uv_write_t req;
  uv_buf_t buf;
};

void cmConnection::on_alloc_buffer(uv_handle_t* handle, size_t suggested_size,
                                   uv_buf_t* buf)
{
  (void)(handle);
  char* rawBuffer = new char[suggested_size];
  *buf = uv_buf_init(rawBuffer, static_cast<unsigned int>(suggested_size));
}

void cmConnection::on_read(uv_stream_t* stream, ssize_t nread,
                           const uv_buf_t* buf)
{
  auto conn = reinterpret_cast<cmConnection*>(stream->data);
  if (conn) {
    if (nread >= 0) {
      conn->ReadData(std::string(buf->base, buf->base + nread));
    } else {
      conn->OnDisconnect((int)nread);
    }
  }

  delete[](buf->base);
}

void cmConnection::on_close_delete(uv_handle_t* handle)
{
  delete handle;
}

void cmConnection::on_close(uv_handle_t*)
{
}

void cmConnection::on_write(uv_write_t* req, int status)
{
  (void)(status);

  // Free req and buffer
  write_req_t* wr = reinterpret_cast<write_req_t*>(req);
  delete[](wr->buf.base);
  delete wr;
}

void cmConnection::on_new_connection(uv_stream_t* stream, int status)
{
  (void)(status);
  auto conn = reinterpret_cast<cmConnection*>(stream->data);

  if (conn) {
    conn->Connect(stream);
  }
}

bool cmConnection::IsOpen() const
{
  return this->WriteStream != CM_NULLPTR;
}

void cmConnection::WriteData(const std::string& data)
{
  assert(this->WriteStream);

  auto ds = data.size();

  write_req_t* req = new write_req_t;
  req->req.data = this;
  req->buf = uv_buf_init(new char[ds], static_cast<unsigned int>(ds));
  memcpy(req->buf.base, data.c_str(), ds);
  uv_write(reinterpret_cast<uv_write_t*>(req),
           static_cast<uv_stream_t*>(this->WriteStream), &req->buf, 1,
           on_write);
}

cmConnection::~cmConnection()
{
  OnServerShuttingDown();
}

void cmConnection::ReadData(const std::string& data)
{
  this->RawReadBuffer += data;
  if (BufferStrategy) {
    std::string packet = BufferStrategy->BufferMessage(this->RawReadBuffer);
    do {
      ProcessRequest(packet);
      packet = BufferStrategy->BufferMessage(this->RawReadBuffer);
    } while (!packet.empty());

  } else {
    ProcessRequest(this->RawReadBuffer);
    this->RawReadBuffer.clear();
  }
}

void cmConnection::SetServer(cmServerBase* s)
{
  Server = s;
}

cmConnection::cmConnection(cmConnectionBufferStrategy* bufferStrategy)
  : BufferStrategy(bufferStrategy)
{
}

void cmConnection::Connect(uv_stream_t*)
{
  Server->OnConnected(nullptr);
}

void cmConnection::ProcessRequest(const std::string& request)
{
  Server->ProcessRequest(this, request);
}

bool cmConnection::OnServeStart(std::string* errString)
{
  (void)errString;
  return true;
}

void cmConnection::OnDisconnect(int errorCode)
{
  (void)errorCode;
  this->Server->OnDisconnect(this);
}

bool cmConnection::OnServerShuttingDown()
{
  this->WriteStream->data = nullptr;
  this->ReadStream->data = nullptr;

  this->ReadStream = nullptr;
  this->WriteStream = nullptr;
  return true;
}
