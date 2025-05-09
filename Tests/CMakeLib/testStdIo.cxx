/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmStdIoInit.h"

#include "testCommon.h"

namespace {
}

int testStdIo(int /*unused*/, char* /*unused*/[])
{
  cm::StdIo::Init();
  return runTests({});
}
