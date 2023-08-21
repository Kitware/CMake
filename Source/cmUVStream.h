/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <cassert>
#include <functional>
#include <istream>

#include <cm/memory>

#include <cm3p/uv.h>

#include "cmUVHandlePtr.h"
#include "cmUVStreambuf.h"

template <typename CharT, typename Traits = std::char_traits<CharT>>
class cmBasicUVIStream : public std::basic_istream<CharT>
{
public:
  cmBasicUVIStream();
  cmBasicUVIStream(uv_stream_t* stream);

  bool is_open() const;

  void open(uv_stream_t* stream);

  void close();

private:
  cmBasicUVStreambuf<CharT, Traits> Buffer;
};

template <typename CharT, typename Traits>
cmBasicUVIStream<CharT, Traits>::cmBasicUVIStream()
  : std::basic_istream<CharT, Traits>(&this->Buffer)
{
}

template <typename CharT, typename Traits>
cmBasicUVIStream<CharT, Traits>::cmBasicUVIStream(uv_stream_t* stream)
  : cmBasicUVIStream()
{
  this->open(stream);
}

template <typename CharT, typename Traits>
bool cmBasicUVIStream<CharT, Traits>::is_open() const
{
  return this->Buffer.is_open();
}

template <typename CharT, typename Traits>
void cmBasicUVIStream<CharT, Traits>::open(uv_stream_t* stream)
{
  this->Buffer.open(stream);
}

template <typename CharT, typename Traits>
void cmBasicUVIStream<CharT, Traits>::close()
{
  this->Buffer.close();
}

using cmUVIStream = cmBasicUVIStream<char>;

template <typename CharT, typename Traits = std::char_traits<CharT>>
class cmBasicUVPipeIStream : public cmBasicUVIStream<CharT, Traits>
{
public:
  cmBasicUVPipeIStream();
  cmBasicUVPipeIStream(uv_loop_t& loop, int fd);

  using cmBasicUVIStream<CharT, Traits>::is_open;

  void open(uv_loop_t& loop, int fd);

  void close();

private:
  cm::uv_pipe_ptr Pipe;
};

template <typename CharT, typename Traits>
cmBasicUVPipeIStream<CharT, Traits>::cmBasicUVPipeIStream() = default;

template <typename CharT, typename Traits>
cmBasicUVPipeIStream<CharT, Traits>::cmBasicUVPipeIStream(uv_loop_t& loop,
                                                          int fd)
{
  this->open(loop, fd);
}

template <typename CharT, typename Traits>
void cmBasicUVPipeIStream<CharT, Traits>::open(uv_loop_t& loop, int fd)
{
  this->Pipe.init(loop, 0);
  uv_pipe_open(this->Pipe, fd);
  this->cmBasicUVIStream<CharT, Traits>::open(this->Pipe);
}

template <typename CharT, typename Traits>
void cmBasicUVPipeIStream<CharT, Traits>::close()
{
  this->cmBasicUVIStream<CharT, Traits>::close();
  this->Pipe.reset();
}

using cmUVPipeIStream = cmBasicUVPipeIStream<char>;

class cmUVStreamReadHandle
{
private:
  std::vector<char> Buffer;
  std::function<void(std::vector<char>)> OnRead;
  std::function<void()> OnFinish;

  template <typename ReadCallback, typename FinishCallback>
  friend std::unique_ptr<cmUVStreamReadHandle> cmUVStreamRead(
    uv_stream_t* stream, ReadCallback onRead, FinishCallback onFinish);
};

template <typename ReadCallback, typename FinishCallback>
std::unique_ptr<cmUVStreamReadHandle> cmUVStreamRead(uv_stream_t* stream,
                                                     ReadCallback onRead,
                                                     FinishCallback onFinish)
{
  auto handle = cm::make_unique<cmUVStreamReadHandle>();
  handle->OnRead = std::move(onRead);
  handle->OnFinish = std::move(onFinish);

  stream->data = handle.get();
  uv_read_start(
    stream,
    [](uv_handle_t* s, std::size_t suggestedSize, uv_buf_t* buffer) {
      auto* data = static_cast<cmUVStreamReadHandle*>(s->data);
      data->Buffer.resize(suggestedSize);
      buffer->base = data->Buffer.data();
      buffer->len = suggestedSize;
    },
    [](uv_stream_t* s, ssize_t nread, const uv_buf_t* buffer) {
      auto* data = static_cast<cmUVStreamReadHandle*>(s->data);
      if (nread > 0) {
        (void)buffer;
        assert(buffer->base == data->Buffer.data());
        data->Buffer.resize(nread);
        data->OnRead(std::move(data->Buffer));
      } else if (nread < 0 /*|| nread == UV_EOF*/) {
        data->OnFinish();
        uv_read_stop(s);
      }
    });

  return handle;
}
