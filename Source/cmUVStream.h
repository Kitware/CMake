/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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
  : std::basic_istream<CharT, Traits>(&this->Buffer)
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
    [](uv_stream_t* s, ssize_t nread, uv_buf_t const* buffer) {
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
