/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmStdIoStream.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <istream> // IWYU pragma: keep
#include <ostream> // IWYU pragma: keep

#ifdef _WIN32
#  include <windows.h>

#  include <io.h> // for _get_osfhandle
#else
#  include <string>

#  include <cm/optional>
#  include <cm/string_view>
#  include <cmext/string_view>

#  include <unistd.h>
#endif

#include "cm_fileno.hxx"

#ifndef _WIN32
#  include "cmSystemTools.h"
#endif

namespace cm {
namespace StdIo {

namespace {

#ifndef _WIN32
// List of known `TERM` names that support VT100 escape sequences.
// Order by `LC_COLLATE=C sort` to search using `std::lower_bound`.
std::array<cm::string_view, 56> const kVT100Names{ {
  "Eterm"_s,
  "alacritty"_s,
  "alacritty-direct"_s,
  "ansi"_s,
  "color-xterm"_s,
  "con132x25"_s,
  "con132x30"_s,
  "con132x43"_s,
  "con132x60"_s,
  "con80x25"_s,
  "con80x28"_s,
  "con80x30"_s,
  "con80x43"_s,
  "con80x50"_s,
  "con80x60"_s,
  "cons25"_s,
  "console"_s,
  "cygwin"_s,
  "dtterm"_s,
  "eterm-color"_s,
  "gnome"_s,
  "gnome-256color"_s,
  "konsole"_s,
  "konsole-256color"_s,
  "kterm"_s,
  "linux"_s,
  "linux-c"_s,
  "mach-color"_s,
  "mlterm"_s,
  "msys"_s,
  "putty"_s,
  "putty-256color"_s,
  "rxvt"_s,
  "rxvt-256color"_s,
  "rxvt-cygwin"_s,
  "rxvt-cygwin-native"_s,
  "rxvt-unicode"_s,
  "rxvt-unicode-256color"_s,
  "screen"_s,
  "screen-256color"_s,
  "screen-256color-bce"_s,
  "screen-bce"_s,
  "screen-w"_s,
  "screen.linux"_s,
  "st-256color"_s,
  "tmux"_s,
  "tmux-256color"_s,
  "vt100"_s,
  "xterm"_s,
  "xterm-16color"_s,
  "xterm-256color"_s,
  "xterm-88color"_s,
  "xterm-color"_s,
  "xterm-debian"_s,
  "xterm-kitty"_s,
  "xterm-termite"_s,
} };

bool TermIsVT100()
{
  if (cm::optional<std::string> term = cmSystemTools::GetEnvVar("TERM")) {
    // NOLINTNEXTLINE(readability-qualified-auto)
    auto i = std::lower_bound(kVT100Names.begin(), kVT100Names.end(), *term);
    if (i != kVT100Names.end() && *i == *term) {
      return true;
    }
  }
  return false;
}
#endif

} // anonymous namespace

Stream::Stream(std::ios& s, FILE* file, Direction direction)
  : IOS_(s)
  , FD_(cm_fileno(file))
{
#ifdef _WIN32
  DWORD mode;
  auto h = reinterpret_cast<HANDLE>(_get_osfhandle(this->FD_));
  if (GetConsoleMode(h, &mode)) {
    this->Console_ = h;
    DWORD vtMode = mode |
      (direction == Direction::In ? ENABLE_VIRTUAL_TERMINAL_INPUT
                                  : ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    if (SetConsoleMode(this->Console_, vtMode)) {
      this->Kind_ = TermKind::VT100;
    } else {
      SetConsoleMode(this->Console_, mode);
      this->Kind_ = TermKind::Console;
    }
  }
#else
  static_cast<void>(direction);
  if (isatty(this->FD_) && TermIsVT100()) {
    this->Kind_ = TermKind::VT100;
  }
#endif
}

IStream::IStream(std::istream& is, FILE* file)
  : Stream(is, file, Direction::In)
{
}

std::istream& IStream::IOS() const
{
  return dynamic_cast<std::istream&>(this->Stream::IOS());
}

OStream::OStream(std::ostream& os, FILE* file)
  : Stream(os, file, Direction::Out)
{
}

std::ostream& OStream::IOS() const
{
  return dynamic_cast<std::ostream&>(this->Stream::IOS());
}

}
}
