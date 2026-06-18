/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include <iostream>
#include <string>

#include <cm/memory>

#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmake.h"

namespace {
struct GenExFixture
{
  cmake CMake{ cmState::Role::Project };
  std::unique_ptr<cmGlobalGenerator> GG;
  std::unique_ptr<cmMakefile> MF;
  std::unique_ptr<cmLocalGenerator> LG;

  GenExFixture()
  {
    this->GG = cm::make_unique<cmGlobalGenerator>(&this->CMake);
    cmStateSnapshot snapshot = this->CMake.GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentBinary(".");
    snapshot.GetDirectory().SetCurrentSource(".");
    this->MF = cm::make_unique<cmMakefile>(this->GG.get(), snapshot);
    this->LG = this->GG->CreateLocalGenerator(this->MF.get());
  }

  std::string Eval(std::string const& expr)
  {
    return cmGeneratorExpression::Evaluate(expr, this->LG.get(), "Debug");
  }
};
}

static bool testApplyBasic()
{
  GenExFixture fx;
  return fx.Eval("$<LIST:TRANSFORM,net;audio,APPLY,$<UPPER_CASE:$<_0>>>") ==
    "NET;AUDIO";
}

static bool testApplyConstantBody()
{
  // The body need not reference the operand at all.
  GenExFixture fx;
  return fx.Eval("$<LIST:TRANSFORM,a;b;c,APPLY,X>") == "X;X;X";
}

static bool testApplyOperandUsedTwice()
{
  // The operand may be substituted more than once in a single body.
  GenExFixture fx;
  return fx.Eval("$<LIST:TRANSFORM,a;b,APPLY,$<_0>$<_0>>") == "aa;bb";
}

static bool testApplyFlatMapExpands()
{
  GenExFixture fx;
  // A list-valued body expands: a -> "x;y", b -> "z" => 3 items.
  std::string out =
    fx.Eval("$<LIST:TRANSFORM,a;b,APPLY,$<IF:$<STREQUAL:$<_0>,a>,x;y,z>>");
  std::string len = fx.Eval("$<LIST:LENGTH,$<LIST:TRANSFORM,a;b,APPLY,"
                            "$<IF:$<STREQUAL:$<_0>,a>,x;y,z>>>");
  return out == "x;y;z" && len == "3";
}

static bool testApplySelector()
{
  GenExFixture fx;
  return fx.Eval(
           "$<LIST:TRANSFORM,a;b;c;d,APPLY,$<UPPER_CASE:$<_0>>,AT,0,2>") ==
    "A;b;C;d";
}

static bool testApplyNestedShadowing()
{
  GenExFixture fx;
  return fx.Eval(
           "$<LIST:TRANSFORM,a;b,APPLY,"
           "$<LIST:TRANSFORM,$<_0>1;$<_0>2,APPLY,$<UPPER_CASE:$<_0>>>>") ==
    "A1;A2;B1;B2";
}

static bool testApplyEmptyList()
{
  GenExFixture fx;
  // An empty list produces an empty string without error (matches canned
  // TRANSFORM behavior).
  std::string got = fx.Eval("$<LIST:TRANSFORM,,APPLY,x$<_0>y>");
  if (!got.empty()) {
    std::cerr << "testApplyEmptyList: expected empty string, got '" << got
              << "'\n";
    return false;
  }
  return true;
}

static bool testApplyEmptyElement()
{
  GenExFixture fx;
  // Every element is selected, including the leading empty one, so the body
  // wraps each: "" -> "xy", "b" -> "xby".
  return fx.Eval("$<LIST:TRANSFORM,;b,APPLY,x$<_0>y>") == "xy;xby";
}

static bool testApplyEmptyPassthrough()
{
  GenExFixture fx;
  // The list ";b" has two elements: "" (index 0, unselected) and "b"
  // (index 1, selected).  The unselected empty element must pass through
  // verbatim, not be silently dropped.
  std::string got = fx.Eval("$<LIST:TRANSFORM,;b,APPLY,X$<_0>Y,AT,1>");
  if (got != ";XbY") {
    std::cerr << "testApplyEmptyPassthrough: expected ';XbY', got '" << got
              << "'\n";
    return false;
  }
  return true;
}

static bool testApplyEmptyResult()
{
  GenExFixture fx;
  // Body yields "" for "a" and "z" for "b".  The empty element produced by
  // the body must not be dropped.
  std::string got =
    fx.Eval("$<LIST:TRANSFORM,a;b,APPLY,$<IF:$<STREQUAL:$<_0>,a>,,z>>");
  if (got != ";z") {
    std::cerr << "testApplyEmptyResult: expected ';z', got '" << got << "'\n";
    return false;
  }
  return true;
}
int testGenExTransformApply(int /*argc*/, char* /*argv*/[])
{
  if (!testApplyBasic()) {
    return 1;
  }
  if (!testApplyConstantBody()) {
    return 1;
  }
  if (!testApplyOperandUsedTwice()) {
    return 1;
  }
  if (!testApplyFlatMapExpands()) {
    return 1;
  }
  if (!testApplySelector()) {
    return 1;
  }
  if (!testApplyNestedShadowing()) {
    return 1;
  }
  if (!testApplyEmptyList()) {
    return 1;
  }
  if (!testApplyEmptyElement()) {
    return 1;
  }
  if (!testApplyEmptyPassthrough()) {
    return 1;
  }
  if (!testApplyEmptyResult()) {
    return 1;
  }
  return 0;
}
