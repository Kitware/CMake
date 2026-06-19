/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <iostream>
#include <string>
#include <vector>

#include "cmGenExContext.h"

static bool testContextBinding()
{
  cm::GenEx::Context ctx(nullptr, "Debug");
  bool ok = true;
  if (ctx.HasBoundOperand() || ctx.BoundOperandCount() != 0) {
    std::cerr << "binding should start unset\n";
    ok = false;
  }
  ctx.SetBoundOperand("net");
  if (!ctx.HasBoundOperand() || ctx.BoundOperandCount() != 1 ||
      ctx.GetBoundOperand() != "net") {
    std::cerr << "binding did not round-trip\n";
    ok = false;
  }
  return ok;
}

static bool testContextMultipleOperands()
{
  cm::GenEx::Context ctx(nullptr, "Debug");
  bool ok = true;
  ctx.SetBoundOperands({ "a", "b" });
  if (ctx.BoundOperandCount() != 2 || !ctx.HasBoundOperand(0) ||
      !ctx.HasBoundOperand(1) || ctx.GetBoundOperand(0) != "a" ||
      ctx.GetBoundOperand(1) != "b") {
    std::cerr << "two-operand binding did not round-trip\n";
    ok = false;
  }
  if (ctx.HasBoundOperand(2)) {
    std::cerr << "index past the frame should be out of range\n";
    ok = false;
  }
  // Re-binding replaces the whole frame, which the shadow/restore of nested
  // bindings relies on.
  ctx.SetBoundOperand("x");
  if (ctx.BoundOperandCount() != 1 || ctx.HasBoundOperand(1) ||
      ctx.GetBoundOperand(0) != "x") {
    std::cerr << "re-binding did not replace the frame\n";
    ok = false;
  }
  ctx.SetBoundOperands({});
  if (ctx.BoundOperandCount() != 0 || ctx.HasBoundOperand(0)) {
    std::cerr << "empty frame should clear the binding\n";
    ok = false;
  }
  return ok;
}

int testGenExBoundOperand(int /*argc*/, char* /*argv*/[])
{
  if (!testContextBinding()) {
    return 1;
  }
  if (!testContextMultipleOperands()) {
    return 1;
  }
  return 0;
}
