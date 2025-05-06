/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <string>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmStdIoConsole.h"
#include "cmStdIoInit.h"
#include "cmStdIoStream.h"

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
  });
}
