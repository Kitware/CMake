/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cwchar>
#include <locale>

class codecvt : public std::codecvt<char, char, mbstate_t>
{
public:
  enum Encoding
  {
    None,
    UTF8,
    UTF8_WITH_BOM,
    ANSI
  };

#ifndef CMAKE_BOOTSTRAP

  codecvt(Encoding e);

protected:
  ~codecvt() override;
  bool do_always_noconv() const noexcept override;
  result do_out(mbstate_t& state, const char* from, const char* from_end,
                const char*& from_next, char* to, char* to_end,
                char*& to_next) const override;
  result do_unshift(mbstate_t& state, char* to, char*,
                    char*& to_next) const override;
  int do_max_length() const noexcept override;
  int do_encoding() const noexcept override;

private:
  // The mbstate_t argument to do_out and do_unshift is responsible
  // for storing state between calls.  We cannot control the type
  // since we want to imbue on standard streams.  However, we do
  // know that it is a trivial type.  Define our own type to overlay
  // on it safely with no alignment requirements.
  struct State
  {
    // Buffer bytes we have consumed from a partial codepoint.
    char partial[3];

    // Number of bytes we have buffered from a partial codepoint.
    unsigned char buffered : 4;

    // Size of the current codepoint in bytes.
    unsigned char size : 4;
  };

  bool m_noconv;
#  if defined(_WIN32)
  unsigned int m_codepage;
  result Decode(mbstate_t& state, int need, const char*& from_next,
                char*& to_next, char* to_end) const;
  result DecodePartial(mbstate_t& state, char*& to_next, char* to_end) const;
  void BufferPartial(mbstate_t& state, int need, const char*& from_next) const;
#  endif

#endif
};
