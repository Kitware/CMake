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

static bool testFilterPredicateInclude()
{
  GenExFixture fx;
  // Keep only elements equal to "a".
  return expectEq(
    "testFilterPredicateInclude",
    fx.Eval("$<LIST:FILTER,a;b;a;c,INCLUDE,PREDICATE,$<STREQUAL:$<_0>,a>>"),
    "a;a");
}

static bool testFilterPredicateExclude()
{
  GenExFixture fx;
  // Drop elements equal to "a".
  return expectEq(
    "testFilterPredicateExclude",
    fx.Eval("$<LIST:FILTER,a;b;a;c,EXCLUDE,PREDICATE,$<STREQUAL:$<_0>,a>>"),
    "b;c");
}

static bool testFilterRegexKeyword()
{
  GenExFixture fx;
  // Explicit REGEX keyword behaves like the bare form.
  return expectEq("testFilterRegexKeyword",
                  fx.Eval("$<LIST:FILTER,foo;bar;baz,INCLUDE,REGEX,^ba>"),
                  "bar;baz");
}

static bool testFilterBareRegexUnchanged()
{
  GenExFixture fx;
  // The legacy bare form still works.
  return expectEq("testFilterBareRegexUnchanged",
                  fx.Eval("$<LIST:FILTER,foo;bar;baz,EXCLUDE,^ba>"), "foo");
}

static bool testCannedTransformStillWorks()
{
  GenExFixture fx;
  // Existing canned action + REGEX selector must be unaffected by the
  // refactor.
  return expectEq("testCannedTransformStillWorks",
                  fx.Eval("$<LIST:TRANSFORM,foo;bar;baz,TOUPPER,REGEX,^ba>"),
                  "foo;BAR;BAZ");
}

static bool testTransformPredicateCanned()
{
  GenExFixture fx;
  // PREPEND "X-" only to elements equal to "a"; others pass through.
  return expectEq(
    "testTransformPredicateCanned",
    fx.Eval(
      "$<LIST:TRANSFORM,a;b;a,PREPEND,X-,PREDICATE,$<STREQUAL:$<_0>,a>>"),
    "X-a;b;X-a");
}

static bool testTransformPredicateNoneSelected()
{
  GenExFixture fx;
  // No element matches: the list is returned unchanged.
  return expectEq("testTransformPredicateNoneSelected",
                  fx.Eval("$<LIST:TRANSFORM,a;b;c,TOUPPER,PREDICATE,0>"),
                  "a;b;c");
}

static bool testTransformPredicateApply()
{
  GenExFixture fx;
  // Upper-case only elements equal to "a"; "b" passes through unchanged.
  return expectEq("testTransformPredicateApply",
                  fx.Eval("$<LIST:TRANSFORM,a;b;a,APPLY,$<UPPER_CASE:$<_0>>,"
                          "PREDICATE,$<STREQUAL:$<_0>,a>>"),
                  "A;b;A");
}

static bool testTransformPredicateApplyShadowing()
{
  GenExFixture fx;
  // The apply body and predicate body each independently bind $<_0>.
  return expectEq("testTransformPredicateApplyShadowing",
                  fx.Eval("$<LIST:TRANSFORM,a;bb,APPLY,$<_0>$<_0>,PREDICATE,"
                          "$<STREQUAL:$<_0>,a>>"),
                  "aa;bb");
}

static bool testFilterPredicateEmptyList()
{
  GenExFixture fx;
  return expectEq("testFilterPredicateEmptyList",
                  fx.Eval("$<LIST:FILTER,,INCLUDE,PREDICATE,1>"), "");
}

static bool testTransformPredicateEmptyList()
{
  GenExFixture fx;
  return expectEq("testTransformPredicateEmptyList",
                  fx.Eval("$<LIST:TRANSFORM,,TOUPPER,PREDICATE,1>"), "");
}

int testGenExListPredicate(int /*argc*/, char* /*argv*/[])
{
  if (!testFilterPredicateInclude()) {
    return 1;
  }
  if (!testFilterPredicateExclude()) {
    return 1;
  }
  if (!testFilterRegexKeyword()) {
    return 1;
  }
  if (!testFilterBareRegexUnchanged()) {
    return 1;
  }
  if (!testCannedTransformStillWorks()) {
    return 1;
  }
  if (!testTransformPredicateCanned()) {
    return 1;
  }
  if (!testTransformPredicateNoneSelected()) {
    return 1;
  }
  if (!testTransformPredicateApply()) {
    return 1;
  }
  if (!testTransformPredicateApplyShadowing()) {
    return 1;
  }
  if (!testFilterPredicateEmptyList()) {
    return 1;
  }
  if (!testTransformPredicateEmptyList()) {
    return 1;
  }
  return 0;
}
