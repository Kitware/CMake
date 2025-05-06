/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <string>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmStdIoConsole.h"
#include "cmStdIoInit.h"
#include "cmStdIoStream.h"
#include "cmStdIoTerminal.h"

#include "testCommon.h"

namespace {

#ifdef _WIN32
cm::string_view const kUTF8 =
  "  Chinese Hindi  Greek English Russian\n  "
  "\xe6\xb3\xa8\xe6\x84\x8f    "                             // Chinese
  "\xe0\xa4\xaf\xe0\xa5\x82\xe0\xa4\xa8\xe0"                 // ...
  "\xa4\xbf\xe0\xa4\x95\xe0\xa5\x8b\xe0\xa4\xa1 "            // Hindi
  "\xce\xb5\xce\xaf\xce\xbd\xce\xb1\xce\xb9 "                // Greek
  "very    "                                                 // English
  "\xd0\xb7\xd0\xb4\xd0\xbe\xd1\x80\xd0\xbe\xd0\xb2\xd0\xbe" // Russian
  "!"_s;
#endif

void printTermKind(cm::string_view t, cm::StdIo::Stream& s)
{
  switch (s.Kind()) {
    case cm::StdIo::TermKind::None:
      std::cout << "  " << t << " is not a terminal.\n";
      break;
    case cm::StdIo::TermKind::VT100:
      std::cout << "  " << t << " is a VT100 terminal.\n";
      break;
#ifdef _WIN32
    case cm::StdIo::TermKind::Console:
      std::cout << "  " << t << " is a Windows Console.\n";
      break;
#endif
  };
}

bool testStream()
{
  std::cout << "testStream()\n";
  printTermKind("stdin"_s, cm::StdIo::In());
  printTermKind("stdout"_s, cm::StdIo::Out());
  printTermKind("stderr"_s, cm::StdIo::Err());
  return true;
}

bool testConsoleStdIn = false;

bool testConsole()
{
  std::cout << "testConsole()\n";
#ifdef _WIN32
  std::cout << kUTF8 << '\n';
#endif
  if (testConsoleStdIn) {
    std::cout << "  input: " << std::flush;
    std::string line;
    if (std::getline(std::cin, line)) {
      std::cout << " output: " << line << '\n';
    }
  }
  return true;
}

void testTerminalPrint(cm::StdIo::TermAttrSet const& attrs,
                       cm::string_view text)
{
  using namespace cm::StdIo;
  std::cout << "  ";
  Print(Out(), attrs, text);
#ifdef _WIN32
  if (Out().Kind() == TermKind::Console) {
    std::cout << " : ";
    Print(Out(), attrs | TermAttr::BackgroundBold, text);
  }
#endif
  std::cout << std::endl;
}

bool testTerminal()
{
  std::cout << "testTerminal()\n";
  using cm::StdIo::TermAttr;
  testTerminalPrint(TermAttr::Normal, "Normal"_s);
  testTerminalPrint(TermAttr::ForegroundBold, "Bold"_s);
  testTerminalPrint(TermAttr::ForegroundBlack, "Black"_s);
  testTerminalPrint(TermAttr::ForegroundBlue, "Blue"_s);
  testTerminalPrint(TermAttr::ForegroundCyan, "Cyan"_s);
  testTerminalPrint(TermAttr::ForegroundGreen, "Green"_s);
  testTerminalPrint(TermAttr::ForegroundMagenta, "Magenta"_s);
  testTerminalPrint(TermAttr::ForegroundRed, "Red"_s);
  testTerminalPrint(TermAttr::ForegroundWhite, "White"_s);
  testTerminalPrint(TermAttr::ForegroundYellow, "Yellow"_s);
  testTerminalPrint({ TermAttr::ForegroundBold, TermAttr::BackgroundBlack },
                    "Bold on Black"_s);
  testTerminalPrint({ TermAttr::ForegroundBlack, TermAttr::BackgroundBlue },
                    "Black on Blue"_s);
  testTerminalPrint({ TermAttr::ForegroundBlack, TermAttr::BackgroundCyan },
                    "Black on Cyan"_s);
  testTerminalPrint({ TermAttr::ForegroundBlack, TermAttr::BackgroundGreen },
                    "Black on Green"_s);
  testTerminalPrint({ TermAttr::ForegroundBlack, TermAttr::BackgroundMagenta },
                    "Black on Magenta"_s);
  testTerminalPrint({ TermAttr::ForegroundBlack, TermAttr::BackgroundRed },
                    "Black on Red"_s);
  testTerminalPrint({ TermAttr::ForegroundBlack, TermAttr::BackgroundWhite },
                    "Black on White"_s);
  testTerminalPrint({ TermAttr::ForegroundBlack, TermAttr::BackgroundYellow },
                    "Black on Yellow"_s);
  return true;
}

cm::string_view const kUsage = "usage: CMakeLibTests testStdIo [--stdin]"_s;

}

int testStdIo(int argc, char* argv[])
{
  cm::StdIo::Init();
  cm::StdIo::Console console;

  for (int i = 1; i < argc; ++i) {
    if (argv[i] == "--stdin"_s && !testConsoleStdIn) {
      testConsoleStdIn = true;
    } else {
      std::cerr << kUsage << '\n';
      return 1;
    }
  }

  return runTests({
    testStream,
    testConsole,
    testTerminal,
  });
}
