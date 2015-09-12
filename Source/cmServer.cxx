/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmServer.h"

#include "cmServerProtocol.h"

#include "cmVersionMacros.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_writer.h"
#endif

typedef struct
{
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
  (void)handle;
  *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

void free_write_req(uv_write_t* req)
{
  write_req_t* wr = (write_req_t*)req;
  free(wr->buf.base);
  free(wr);
}

void on_stdout_write(uv_write_t* req, int status)
{
  (void)status;
  auto server = reinterpret_cast<cmMetadataServer*>(req->data);
  free_write_req(req);
  server->PopOne();
}

void write_data(uv_stream_t* dest, std::string content, uv_write_cb cb)
{
  write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
  req->req.data = dest->data;
  req->buf = uv_buf_init((char*)malloc(content.size()), content.size());
  memcpy(req->buf.base, content.c_str(), content.size());
  uv_write((uv_write_t*)req, (uv_stream_t*)dest, &req->buf, 1, cb);
}

void read_stdin(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
  if (nread > 0) {
    auto server = reinterpret_cast<cmMetadataServer*>(stream->data);
    std::string result = std::string(buf->base, buf->base + nread);
    server->handleData(result);
  }

  if (buf->base)
    free(buf->base);
}

cmMetadataServer::cmMetadataServer()
  : Protocol(0)
{
  mLoop = uv_default_loop();

  uv_pipe_init(mLoop, &mStdin_pipe, 0);
  uv_pipe_open(&mStdin_pipe, 0);
  mStdin_pipe.data = this;

  uv_pipe_init(mLoop, &mStdout_pipe, 0);
  uv_pipe_open(&mStdout_pipe, 1);
  mStdout_pipe.data = this;

  this->State = Uninitialized;
  this->Writing = false;
}

cmMetadataServer::~cmMetadataServer()
{
  uv_close((uv_handle_t*)&mStdin_pipe, NULL);
  uv_close((uv_handle_t*)&mStdout_pipe, NULL);
  uv_loop_close(mLoop);
  delete this->Protocol;
}

void cmMetadataServer::PopOne()
{
  this->Writing = false;
  if (mQueue.empty()) {
    return;
  }
  this->Protocol->processRequest(mQueue.front());
  mQueue.erase(mQueue.begin());
}

void cmMetadataServer::handleData(const std::string& data)
{
  mDataBuffer += data;

  for (;;) {
    auto needle = mDataBuffer.find('\n');

    if (needle == std::string::npos) {
      return;
    }
    std::string line = mDataBuffer.substr(0, needle);
    mDataBuffer.erase(mDataBuffer.begin(), mDataBuffer.begin() + needle + 1);
    if (line == "[== CMake MetaMagic ==[") {
      mJsonData.clear();
      continue;
    }
    if (line == "]== CMake MetaMagic ==]") {
      mQueue.push_back(mJsonData);
      mJsonData.clear();
      if (!this->Writing) {
        this->PopOne();
      }
    } else {
      mJsonData += line;
      mJsonData += "\n";
    }
  }
}

void cmMetadataServer::ServeMetadata(const std::string& buildDir)
{
  this->State = Started;

  Json::Value obj = Json::objectValue;
  obj["progress"] = "process-started";
  this->WriteResponse(obj);

  this->Protocol = new cmServerProtocol(this, buildDir);

  uv_read_start((uv_stream_t*)&mStdin_pipe, alloc_buffer, read_stdin);

  uv_run(mLoop, UV_RUN_DEFAULT);
}

void cmMetadataServer::WriteResponse(const Json::Value& jsonValue)
{
  Json::FastWriter writer;

  std::string result = "\n[== CMake MetaMagic ==[\n";
  result += writer.write(jsonValue);
  result += "]== CMake MetaMagic ==]\n";

  this->Writing = true;
  write_data((uv_stream_t*)&mStdout_pipe, result, on_stdout_write);
}
