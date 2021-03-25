/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGccDepfileLexerHelper.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "cmGccDepfileReaderTypes.h"

#include "LexerParser/cmGccDepfileLexer.h"

#ifdef _WIN32
#  include "cmsys/Encoding.h"
#endif

bool cmGccDepfileLexerHelper::readFile(const char* filePath)
{
#ifdef _WIN32
  wchar_t* wpath = cmsysEncoding_DupToWide(filePath);
  FILE* file = _wfopen(wpath, L"rb");
  free(wpath);
#else
  FILE* file = fopen(filePath, "r");
#endif
  if (!file) {
    return false;
  }
  this->newEntry();
  yyscan_t scanner;
  cmGccDepfile_yylex_init(&scanner);
  cmGccDepfile_yyset_extra(this, scanner);
  cmGccDepfile_yyrestart(file, scanner);
  cmGccDepfile_yylex(scanner);
  cmGccDepfile_yylex_destroy(scanner);
  this->sanitizeContent();
  fclose(file);
  return this->HelperState != State::Failed;
}

void cmGccDepfileLexerHelper::newEntry()
{
  if (this->HelperState == State::Rule && !this->Content.empty()) {
    if (!this->Content.back().rules.empty() &&
        !this->Content.back().rules.back().empty()) {
      this->HelperState = State::Failed;
    }
    return;
  }
  this->HelperState = State::Rule;
  this->Content.emplace_back();
  this->newRule();
}

void cmGccDepfileLexerHelper::newRule()
{
  auto& entry = this->Content.back();
  if (entry.rules.empty() || !entry.rules.back().empty()) {
    entry.rules.emplace_back();
  }
}

void cmGccDepfileLexerHelper::newDependency()
{
  if (this->HelperState == State::Failed) {
    return;
  }
  this->HelperState = State::Dependency;
  auto& entry = this->Content.back();
  if (entry.paths.empty() || !entry.paths.back().empty()) {
    entry.paths.emplace_back();
  }
}

void cmGccDepfileLexerHelper::newRuleOrDependency()
{
  if (this->HelperState == State::Rule) {
    this->newRule();
  } else if (this->HelperState == State::Dependency) {
    this->newDependency();
  }
}

void cmGccDepfileLexerHelper::addToCurrentPath(const char* s)
{
  if (this->Content.empty()) {
    return;
  }
  cmGccStyleDependency* dep = &this->Content.back();
  std::string* dst = nullptr;
  switch (this->HelperState) {
    case State::Rule: {
      if (dep->rules.empty()) {
        return;
      }
      dst = &dep->rules.back();
    } break;
    case State::Dependency: {
      if (dep->paths.empty()) {
        return;
      }
      dst = &dep->paths.back();
    } break;
    case State::Failed:
      return;
  }
  dst->append(s);
}

void cmGccDepfileLexerHelper::sanitizeContent()
{
  for (auto it = this->Content.begin(); it != this->Content.end();) {
    // Remove empty rules
    for (auto rit = it->rules.begin(); rit != it->rules.end();) {
      if (rit->empty()) {
        rit = it->rules.erase(rit);
      } else {
        ++rit;
      }
    }
    // Remove the entry if rules are empty
    if (it->rules.empty()) {
      it = this->Content.erase(it);
    } else {
      // Remove empty paths
      for (auto pit = it->paths.begin(); pit != it->paths.end();) {
        if (pit->empty()) {
          pit = it->paths.erase(pit);
        } else {
          ++pit;
        }
      }
      ++it;
    }
  }
}
