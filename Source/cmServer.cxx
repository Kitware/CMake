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
#include <cmsys/Encoding.hxx>
#include <cmsys/FStream.hxx>

#ifndef _WIN32
#include <unistd.h>
#define CM_O_CREAT O_CREAT
#define CM_O_RDWR O_RDWR
#else
#include <fcntl.h>
#define CM_O_CREAT _O_CREAT
#define CM_O_RDWR _O_RDWR
#endif

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

void on_logfile_write(uv_write_t* req, int status)
{
  (void)req;
  (void)status;
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

  if (uv_guess_handle(1) == UV_TTY) {
    uv_tty_init(mLoop, &mStdin_tty, 0, 1);
    uv_tty_set_mode(&mStdin_tty, UV_TTY_MODE_NORMAL);
    mStdin_tty.data = this;

    uv_tty_init(mLoop, &mStdout_tty, 1, 0);
    uv_tty_set_mode(&mStdout_tty, UV_TTY_MODE_NORMAL);
    mStdout_tty.data = this;
  } else {
    uv_pipe_init(mLoop, &mStdin_pipe, 0);
    uv_pipe_open(&mStdin_pipe, 0);
    mStdin_pipe.data = this;

    uv_pipe_init(mLoop, &mStdout_pipe, 0);
    uv_pipe_open(&mStdout_pipe, 1);
    mStdout_pipe.data = this;
  }

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

// return true if server should quit, false otherwise.
bool cmMetadataServer::PopOne()
{
  this->Writing = false;
  if (mQueue.empty()) {
    return false;
  }

  auto fname = mBuildDir + "/cmake-daemon-" + std::to_string(getpid());

  uv_fs_t file_req;
  int fd = uv_fs_open(mLoop, &file_req, fname.c_str(), O_RDWR, 0644, NULL);
  uv_pipe_open(&mLogFile_pipe, fd);

  auto request = mQueue.front();
  request = "<<< " + request;
  write_data((uv_stream_t*)&mLogFile_pipe, request, on_logfile_write);

  auto const quit = this->Protocol->processRequest(mQueue.front());
  mQueue.erase(mQueue.begin());
  return quit;
}

void cmMetadataServer::handleData(const std::string& data)
{
#ifdef _WIN32
#define LINE_SEP "\r\n"
#else
#define LINE_SEP "\n"
#endif

  mDataBuffer += data;

  for (;;) {
    auto needle = mDataBuffer.find(LINE_SEP);

    if (needle == std::string::npos) {
      return;
    }
    std::string line = mDataBuffer.substr(0, needle);
    mDataBuffer.erase(0, needle + sizeof(LINE_SEP));
    if (line == "[== CMake MetaMagic ==[") {
      mJsonData.clear();
      continue;
    }
    if (line == "]== CMake MetaMagic ==]") {
      mQueue.push_back(mJsonData);
      mJsonData.clear();
      if (!this->Writing) {
        if (this->PopOne()) {
          uv_stop(mLoop);
          return;
        }
      }
    } else {
      mJsonData += line;
      mJsonData += "\n";
    }
  }
}

void cmMetadataServer::ServeMetadata(const std::string& buildDir)
{
  auto fname = buildDir + "/cmake-daemon-" + std::to_string(getpid());

  uv_fs_t file_req;
  int fd = uv_fs_open(mLoop, &file_req, fname.c_str(), CM_O_CREAT | CM_O_RDWR,
                      0644, NULL);
  uv_pipe_init(mLoop, &mLogFile_pipe, 0);
  uv_pipe_open(&mLogFile_pipe, fd);

  write_data((uv_stream_t*)&mLogFile_pipe, "Log file\n", on_logfile_write);

  mBuildDir = buildDir;
  this->State = Started;

  Json::Value obj = Json::objectValue;
  obj["progress"] = "process-started";
  this->WriteResponse(obj);

  this->Protocol = new cmServerProtocol(this, buildDir);

  if (uv_guess_handle(1) == UV_TTY) {
    uv_read_start((uv_stream_t*)&mStdin_tty, alloc_buffer, read_stdin);
  } else {
    uv_read_start((uv_stream_t*)&mStdin_pipe, alloc_buffer, read_stdin);
  }

  uv_run(mLoop, UV_RUN_DEFAULT);
}

void cmMetadataServer::WriteResponse(const Json::Value& jsonValue)
{
  Json::FastWriter writer;
  auto decoded = writer.write(jsonValue);

  auto fname = mBuildDir + "/cmake-daemon-" + std::to_string(getpid());

  uv_fs_t file_req;
  int fd = uv_fs_open(mLoop, &file_req, fname.c_str(), O_RDWR, 0644, NULL);
  uv_pipe_open(&mLogFile_pipe, fd);

  auto response = decoded;
  response = ">>> " + response;

  write_data((uv_stream_t*)&mLogFile_pipe, response, on_logfile_write);

  std::string result = "\n[== CMake MetaMagic ==[\n";
  result += decoded;
  result += "]== CMake MetaMagic ==]\n";

  this->Writing = true;
  if (uv_guess_handle(1) == UV_TTY) {
    write_data((uv_stream_t*)&mStdout_tty, result, on_stdout_write);
  } else {
    write_data((uv_stream_t*)&mStdout_pipe, result, on_stdout_write);
  }
}
