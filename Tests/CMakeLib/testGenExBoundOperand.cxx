/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <iostream>
#include <string>

#include "cmGenExContext.h"

static bool testContextBinding()
{
  cm::GenEx::Context ctx(nullptr, "Debug");
  bool ok = true;
  if (ctx.HasBoundOperand()) {
    std::cerr << "binding should start unset\n";
    ok = false;
  }
  ctx.SetBoundOperand("net");
  if (!ctx.HasBoundOperand() || ctx.GetBoundOperand() != "net") {
    std::cerr << "binding did not round-trip\n";
    ok = false;
  }
  return ok;
}

int testGenExBoundOperand(int /*argc*/, char* /*argv*/[])
{
  if (!testContextBinding()) {
    return 1;
  }
  return 0;
}
