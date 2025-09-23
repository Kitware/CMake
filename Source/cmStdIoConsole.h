/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmStdIoInit.h"

#ifdef _WIN32
#  include <memory>
#endif

namespace cm {
namespace StdIo {

/**
 * On Windows, enables I/O with `cin`, `cout`, and `cerr` in UTF-8 encoding.
 * On non-Windows platforms, does nothing.
 *
 * Construct an instance of this at the beginning of `main`:
 *
 * * If `cin`, `cout`, or `cerr` is attached to a Windows Console whose
 *   input/output code page is not UTF-8, this replaces its `streambuf`
 *   with one that reads/writes from/to the console using wide-character
 *   Windows APIs to avoid limitations of the code page's narrow encoding.
 *
 * * If `cin`, `cout`, or `cerr` is not attached to a Windows Console,
 *   this sets its stream to binary mode for consistency with the case
 *   that it's attached to a console.
 *
 * Destroy the instance of this to restore the original `streambuf`s.
 */
class Console : private Init
{
#ifdef _WIN32
  class Impl;
  std::unique_ptr<Impl> Impl_;
#endif
public:
  Console();
  ~Console(); // NOLINT(performance-trivially-destructible)
  Console(Console&&) noexcept;
  Console(Console const&) = delete;
  Console& operator=(Console&&) noexcept;
  Console& operator=(Console const&) = delete;
};

}
}
