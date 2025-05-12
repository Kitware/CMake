/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <cm/string_view>
#include <cmext/string_view>

#include "cmStdIoInit.h"
#include "cmStdIoStream.h"

#include "testCommon.h"

namespace {

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

}

int testStdIo(int /*unused*/, char* /*unused*/[])
{
  cm::StdIo::Init();
  return runTests({
    testStream,
  });
}
