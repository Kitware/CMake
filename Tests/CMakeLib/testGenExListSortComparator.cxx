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

bool expectEq(char const* name, std::string const& got,
              std::string const& want)
{
  if (got != want) {
    std::cerr << name << ": expected '" << want << "', got '" << got << "'\n";
    return false;
  }
  return true;
}
}

static bool testSortNumericAscending()
{
  GenExFixture fx;
  return expectEq(
    "testSortNumericAscending",
    fx.Eval("$<LIST:SORT,3;1;2,COMPARATOR,$<STRLESS:$<_0>,$<_1>>>"), "1;2;3");
}

static bool testSortByExtension()
{
  GenExFixture fx;
  return expectEq("testSortByExtension",
                  fx.Eval("$<LIST:SORT,c.z;a.a;b.m,COMPARATOR,"
                          "$<STRLESS:$<PATH:GET_EXTENSION,$<_0>>,"
                          "$<PATH:GET_EXTENSION,$<_1>>>>"),
                  "a.a;b.m;c.z");
}

static bool testSortDescending()
{
  GenExFixture fx;
  return expectEq(
    "testSortDescending",
    fx.Eval(
      "$<LIST:SORT,3;1;2,COMPARATOR,$<STRLESS:$<_0>,$<_1>>,ORDER:DESCENDING>"),
    "3;2;1");
}

static bool testSortNestedBinding()
{
  GenExFixture fx;
  // Nested binding: the inner APPLY rebinds $<_0> but the outer $<_1> is
  // restored after it, so this reduces to STRLESS(a, b) ascending.
  return expectEq(
    "testSortNestedBinding",
    fx.Eval("$<LIST:SORT,y;x,COMPARATOR,"
            "$<STRLESS:$<LIST:TRANSFORM,$<_0>,APPLY,$<_0>>,$<_1>>>"),
    "x;y");
}

static bool testSortEmpty()
{
  GenExFixture fx;
  return expectEq("testSortEmpty",
                  fx.Eval("$<LIST:SORT,,COMPARATOR,$<STRLESS:$<_0>,$<_1>>>"),
                  "");
}

static bool testSortSingle()
{
  GenExFixture fx;
  return expectEq("testSortSingle",
                  fx.Eval("$<LIST:SORT,x,COMPARATOR,$<STRLESS:$<_0>,$<_1>>>"),
                  "x");
}

static bool testSortEqualElements()
{
  GenExFixture fx;
  // Equal elements are FALSE both ways, so the strict-weak-ordering guard must
  // not trip and the duplicates are preserved.
  return expectEq(
    "testSortEqualElements",
    fx.Eval("$<LIST:SORT,b;a;b,COMPARATOR,$<STRLESS:$<_0>,$<_1>>>"), "a;b;b");
}

static bool testSortCaseInsensitive()
{
  // CASE:INSENSITIVE case-folds the body's operands, so B;a;C orders as a;B;C
  // (elements keep their original case).
  GenExFixture fx;
  return expectEq(
    "testSortCaseInsensitive",
    fx.Eval(
      "$<LIST:SORT,B;a;C,COMPARATOR,$<STRLESS:$<_0>,$<_1>>,CASE:INSENSITIVE>"),
    "a;B;C");
}

int testGenExListSortComparator(int /*argc*/, char* /*argv*/[])
{
  if (!testSortNumericAscending()) {
    return 1;
  }
  if (!testSortByExtension()) {
    return 1;
  }
  if (!testSortDescending()) {
    return 1;
  }
  if (!testSortNestedBinding()) {
    return 1;
  }
  if (!testSortEmpty()) {
    return 1;
  }
  if (!testSortSingle()) {
    return 1;
  }
  if (!testSortEqualElements()) {
    return 1;
  }
  if (!testSortCaseInsensitive()) {
    return 1;
  }
  return 0;
}
