// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef dap_string_buffer_h
#define dap_string_buffer_h

#include "dap/io.h"

#include <algorithm>  // std::min
#include <cstring>    // memcpy
#include <deque>
#include <memory>  // std::unique_ptr
#include <string>

namespace dap {

class StringBuffer : public virtual Reader, public virtual Writer {
 public:
  static inline std::unique_ptr<StringBuffer> create();

  inline bool write(const std::string& s);
  inline std::string string() const;

  // Reader / Writer compilance
  inline bool isOpen() override;
  inline void close() override;
  inline size_t read(void* buffer, size_t bytes) override;
  inline bool write(const void* buffer, size_t bytes) override;

 private:
  std::string str;
  std::deque<size_t> chunk_lengths;
  bool closed = false;
};

bool StringBuffer::isOpen() {
  return !closed;
}
void StringBuffer::close() {
  closed = true;
}

std::unique_ptr<StringBuffer> StringBuffer::create() {
  return std::unique_ptr<StringBuffer>(new StringBuffer());
}

bool StringBuffer::write(const std::string& s) {
  return write(s.data(), s.size());
}

std::string StringBuffer::string() const {
  return str;
}

size_t StringBuffer::read(void* buffer, size_t bytes) {
  if (closed || bytes == 0 || str.size() == 0 || chunk_lengths.size() == 0) {
    return 0;
  }
  size_t& chunk_length = chunk_lengths.front();

  auto len = std::min(bytes, chunk_length);
  memcpy(buffer, str.data(), len);
  str = std::string(str.begin() + len, str.end());
  if (bytes < chunk_length) {
    chunk_length -= bytes;
  } else {
    chunk_lengths.pop_front();
  }
  return len;
}

bool StringBuffer::write(const void* buffer, size_t bytes) {
  if (closed) {
    return false;
  }
  auto chars = reinterpret_cast<const char*>(buffer);
  str.append(chars, chars + bytes);
  chunk_lengths.push_back(bytes);
  return true;
}

}  // namespace dap

#endif  // dap_string_buffer_h
