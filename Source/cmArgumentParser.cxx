/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmArgumentParser.h"

#include <algorithm>

#include "cmArgumentParserTypes.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"

namespace ArgumentParser {

auto KeywordActionMap::Emplace(cm::string_view name, KeywordAction action)
  -> std::pair<iterator, bool>
{
  auto const it =
    std::lower_bound(this->begin(), this->end(), name,
                     [](value_type const& elem, cm::string_view const& k) {
                       return elem.first < k;
                     });
  return (it != this->end() && it->first == name)
    ? std::make_pair(it, false)
    : std::make_pair(this->emplace(it, name, std::move(action)), true);
}

auto KeywordActionMap::Find(cm::string_view name) const -> const_iterator
{
  auto const it =
    std::lower_bound(this->begin(), this->end(), name,
                     [](value_type const& elem, cm::string_view const& k) {
                       return elem.first < k;
                     });
  return (it != this->end() && it->first == name) ? it : this->end();
}

void Instance::Bind(bool& val)
{
  val = true;
  this->CurrentString = nullptr;
  this->CurrentList = nullptr;
  this->ExpectValue = false;
}

void Instance::Bind(std::string& val)
{
  this->CurrentString = &val;
  this->CurrentList = nullptr;
  this->ExpectValue = true;
}

void Instance::Bind(Maybe<std::string>& val)
{
  this->CurrentString = &val;
  this->CurrentList = nullptr;
  this->ExpectValue = false;
}

void Instance::Bind(MaybeEmpty<std::vector<std::string>>& val)
{
  this->CurrentString = nullptr;
  this->CurrentList = &val;
  this->ExpectValue = false;
}

void Instance::Bind(NonEmpty<std::vector<std::string>>& val)
{
  this->CurrentString = nullptr;
  this->CurrentList = &val;
  this->ExpectValue = true;
}

void Instance::Bind(std::vector<std::vector<std::string>>& val)
{
  this->CurrentString = nullptr;
  this->CurrentList = (static_cast<void>(val.emplace_back()), &val.back());
  this->ExpectValue = false;
}

void Instance::Consume(cm::string_view arg)
{
  auto const it = this->Bindings.Keywords.Find(arg);
  if (it != this->Bindings.Keywords.end()) {
    this->FinishKeyword();
    this->Keyword = it->first;
    if (this->Bindings.ParsedKeyword) {
      this->Bindings.ParsedKeyword(*this, it->first);
    }
    it->second(*this);
    return;
  }

  if (this->CurrentString != nullptr) {
    this->CurrentString->assign(std::string(arg));
    this->CurrentString = nullptr;
    this->CurrentList = nullptr;
  } else if (this->CurrentList != nullptr) {
    this->CurrentList->emplace_back(arg);
  } else if (this->UnparsedArguments != nullptr) {
    this->UnparsedArguments->emplace_back(arg);
  }

  this->ExpectValue = false;
}

void Instance::FinishKeyword()
{
  if (this->Keyword.empty()) {
    return;
  }
  if (this->ExpectValue) {
    if (this->ParseResults != nullptr) {
      this->ParseResults->AddKeywordError(this->Keyword,
                                          "  missing required value\n");
    }
    if (this->Bindings.KeywordMissingValue) {
      this->Bindings.KeywordMissingValue(*this, this->Keyword);
    }
  }
}

bool ParseResult::MaybeReportError(cmMakefile& mf) const
{
  if (*this) {
    return false;
  }
  std::string e;
  for (auto const& ke : this->KeywordErrors) {
    e = cmStrCat(e, "Error after keyword \"", ke.first, "\":\n", ke.second);
  }
  mf.IssueMessage(MessageType::FATAL_ERROR, e);
  return true;
}

} // namespace ArgumentParser
