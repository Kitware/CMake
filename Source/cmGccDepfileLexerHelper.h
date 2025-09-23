/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <utility>

#include <cmGccDepfileReaderTypes.h>

class cmGccDepfileLexerHelper
{
public:
  cmGccDepfileLexerHelper() = default;

  bool readFile(char const* filePath);
  cmGccDepfileContent extractContent() && { return std::move(this->Content); }

  // Functions called by the lexer
  void newEntry();
  void newRule();
  void newDependency();
  void newRuleOrDependency();
  void addToCurrentPath(char const* s);

private:
  void sanitizeContent();

  cmGccDepfileContent Content;

  enum class State
  {
    Rule,
    Dependency,
    Failed,
  };
  State HelperState = State::Rule;
};

#define YY_EXTRA_TYPE cmGccDepfileLexerHelper*
