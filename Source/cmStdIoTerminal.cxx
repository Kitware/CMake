/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmStdIoTerminal.h"

#include <array>
#include <functional>
#include <iosfwd>
#include <string>
#include <type_traits>

#include <cm/string_view>
#include <cmext/string_view>

#ifdef _WIN32
#  include <windows.h>
#endif

#include <cm/optional>

#include "cmStdIoStream.h"
#include "cmSystemTools.h"

namespace cm {
namespace StdIo {

namespace {

#ifdef _WIN32
WORD const kConsoleAttrMask = FOREGROUND_RED | FOREGROUND_GREEN |
  FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN |
  BACKGROUND_BLUE | BACKGROUND_INTENSITY;
std::array<WORD, kTermAttrCount> const kConsoleAttrs{ {
  0,                                                   // Normal
  FOREGROUND_INTENSITY,                                // ForegroundBold
  0,                                                   // ForegroundBlack
  FOREGROUND_BLUE,                                     // ForegroundBlue
  FOREGROUND_GREEN | FOREGROUND_BLUE,                  // ForegroundCyan
  FOREGROUND_GREEN,                                    // ForegroundGreen
  FOREGROUND_RED | FOREGROUND_BLUE,                    // ForegroundMagenta
  FOREGROUND_RED,                                      // ForegroundRed
  FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // ForegroundWhite
  FOREGROUND_RED | FOREGROUND_GREEN,                   // ForegroundYellow
  BACKGROUND_INTENSITY,                                // BackgroundBold
  0,                                                   // BackgroundBlack
  BACKGROUND_BLUE,                                     // BackgroundBlue
  BACKGROUND_GREEN | BACKGROUND_BLUE,                  // BackgroundCyan
  BACKGROUND_GREEN,                                    // BackgroundGreen
  BACKGROUND_RED | BACKGROUND_BLUE,                    // BackgroundMagenta
  BACKGROUND_RED,                                      // BackgroundRed
  BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE, // BackgroundWhite
  BACKGROUND_RED | BACKGROUND_GREEN,                   // BackgroundYellow
} };

WORD ConsoleAttrs(WORD consoleAttrs, TermAttrSet const& attrs)
{
  consoleAttrs =
    attrs.contains(TermAttr::Normal) ? consoleAttrs & kConsoleAttrMask : 0;
  for (TermAttr attr : attrs) {
    auto index = static_cast<std::underlying_type<TermAttr>::type>(attr);
    consoleAttrs |= kConsoleAttrs[index];
  }
  return consoleAttrs;
}
#endif

// VT100 escape sequence strings.
#if defined(__MVS__) // z/OS: assume EBCDIC
#  define ESC "\47"
#else
#  define ESC "\33"
#endif

std::array<cm::string_view, kTermAttrCount> const kVT100Codes{ {
  ESC "[0m"_s,  // Normal
  ESC "[1m"_s,  // ForegroundBold
  ESC "[30m"_s, // ForegroundBlack
  ESC "[34m"_s, // ForegroundBlue
  ESC "[36m"_s, // ForegroundCyan
  ESC "[32m"_s, // ForegroundGreen
  ESC "[35m"_s, // ForegroundMagenta
  ESC "[31m"_s, // ForegroundRed
  ESC "[37m"_s, // ForegroundWhite
  ESC "[33m"_s, // ForegroundYellow
  ""_s,         // BackgroundBold
  ESC "[40m"_s, // BackgroundBlack
  ESC "[44m"_s, // BackgroundBlue
  ESC "[46m"_s, // BackgroundCyan
  ESC "[42m"_s, // BackgroundGreen
  ESC "[45m"_s, // BackgroundMagenta
  ESC "[41m"_s, // BackgroundRed
  ESC "[47m"_s, // BackgroundWhite
  ESC "[43m"_s, // BackgroundYellow
} };

void SetVT100Attrs(std::ostream& os, TermAttrSet const& attrs)
{
  for (TermAttr attr : attrs) {
    auto index = static_cast<std::underlying_type<TermAttr>::type>(attr);
    os << kVT100Codes[index];
  }
}

auto const TermEnv = []() -> cm::optional<TermKind> {
  /* Disable color according to https://bixense.com/clicolors/ convention. */
  if (cm::optional<std::string> noColor =
        cmSystemTools::GetEnvVar("NO_COLOR")) {
    if (!noColor->empty() && *noColor != "0"_s) {
      return TermKind::None;
    }
  }
  /* Force color according to https://bixense.com/clicolors/ convention.  */
  if (cm::optional<std::string> cliColorForce =
        cmSystemTools::GetEnvVar("CLICOLOR_FORCE")) {
    if (!cliColorForce->empty() && *cliColorForce != "0"_s) {
      return TermKind::VT100;
    }
  }
  /* Disable color according to https://bixense.com/clicolors/ convention. */
  if (cm::optional<std::string> cliColor =
        cmSystemTools::GetEnvVar("CLICOLOR")) {
    if (*cliColor == "0"_s) {
      return TermKind::None;
    }
  }
  /* GNU make 4.1+ may tell us that its output is destined for a TTY. */
  if (cm::optional<std::string> makeTermOut =
        cmSystemTools::GetEnvVar("MAKE_TERMOUT")) {
    if (!makeTermOut->empty()) {
      return TermKind::VT100;
    }
  }
  return cm::nullopt;
}();

void Print(OStream& os, TermAttrSet const& attrs,
           std::function<void(std::ostream&)> const& f)
{
  TermKind kind = TermEnv ? *TermEnv : os.Kind();
  switch (kind) {
    case TermKind::None:
      f(os.IOS());
      break;
    case TermKind::VT100:
      SetVT100Attrs(os.IOS(), attrs);
      f(os.IOS());
      SetVT100Attrs(os.IOS(), TermAttr::Normal);
      break;
#ifdef _WIN32
    case TermKind::Console: {
      HANDLE console = os.Console();
      CONSOLE_SCREEN_BUFFER_INFO sbi;
      if (!attrs.empty() && GetConsoleScreenBufferInfo(console, &sbi)) {
        Out().IOS().flush();
        Err().IOS().flush();
        SetConsoleTextAttribute(console, ConsoleAttrs(sbi.wAttributes, attrs));
        f(os.IOS());
        Out().IOS().flush();
        Err().IOS().flush();
        SetConsoleTextAttribute(
          console, ConsoleAttrs(sbi.wAttributes, TermAttr::Normal));
      } else {
        f(os.IOS());
      }
    } break;
#endif
  };
}

} // anonymous namespace

void Print(OStream& os, TermAttrSet const& attrs, cm::string_view s)
{
  Print(os, attrs, [s](std::ostream& o) { o << s; });
}

}
}
