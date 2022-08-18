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

auto PositionActionMap::Emplace(std::size_t pos, PositionAction action)
  -> std::pair<iterator, bool>
{
  auto const it = std::lower_bound(
    this->begin(), this->end(), pos,
    [](value_type const& elem, std::size_t k) { return elem.first < k; });
  return (it != this->end() && it->first == pos)
    ? std::make_pair(it, false)
    : std::make_pair(this->emplace(it, pos, std::move(action)), true);
}

auto PositionActionMap::Find(std::size_t pos) const -> const_iterator
{
  auto const it = std::lower_bound(
    this->begin(), this->end(), pos,
    [](value_type const& elem, std::size_t k) { return elem.first < k; });
  return (it != this->end() && it->first == pos) ? it : this->end();
}

void Instance::Bind(std::function<Continue(cm::string_view)> f,
                    ExpectAtLeast expect)
{
  this->KeywordValueFunc = std::move(f);
  this->KeywordValuesExpected = expect.Count;
}

void Instance::Bind(bool& val)
{
  val = true;
  this->Bind(nullptr, ExpectAtLeast{ 0 });
}

void Instance::Bind(std::string& val)
{
  this->Bind(
    [&val](cm::string_view arg) -> Continue {
      val = std::string(arg);
      return Continue::No;
    },
    ExpectAtLeast{ 1 });
}

void Instance::Bind(NonEmpty<std::string>& val)
{
  this->Bind(
    [this, &val](cm::string_view arg) -> Continue {
      if (arg.empty() && this->ParseResults) {
        this->ParseResults->AddKeywordError(this->Keyword,
                                            "  empty string not allowed\n");
      }
      val.assign(std::string(arg));
      return Continue::No;
    },
    ExpectAtLeast{ 1 });
}

void Instance::Bind(Maybe<std::string>& val)
{
  this->Bind(
    [&val](cm::string_view arg) -> Continue {
      static_cast<std::string&>(val) = std::string(arg);
      return Continue::No;
    },
    ExpectAtLeast{ 0 });
}

void Instance::Bind(MaybeEmpty<std::vector<std::string>>& val)
{
  this->Bind(
    [&val](cm::string_view arg) -> Continue {
      val.emplace_back(arg);
      return Continue::Yes;
    },
    ExpectAtLeast{ 0 });
}

void Instance::Bind(NonEmpty<std::vector<std::string>>& val)
{
  this->Bind(
    [&val](cm::string_view arg) -> Continue {
      val.emplace_back(arg);
      return Continue::Yes;
    },
    ExpectAtLeast{ 1 });
}

void Instance::Bind(std::vector<std::vector<std::string>>& multiVal)
{
  multiVal.emplace_back();
  std::vector<std::string>& val = multiVal.back();
  this->Bind(
    [&val](cm::string_view arg) -> Continue {
      val.emplace_back(arg);
      return Continue::Yes;
    },
    ExpectAtLeast{ 0 });
}

void Instance::Consume(std::size_t pos, cm::string_view arg)
{
  auto const it = this->Bindings.Keywords.Find(arg);
  if (it != this->Bindings.Keywords.end()) {
    this->FinishKeyword();
    this->Keyword = it->first;
    this->KeywordValuesSeen = 0;
    this->DoneWithPositional = true;
    if (this->Bindings.ParsedKeyword) {
      this->Bindings.ParsedKeyword(*this, it->first);
    }
    it->second(*this);
    return;
  }

  if (this->KeywordValueFunc) {
    switch (this->KeywordValueFunc(arg)) {
      case Continue::Yes:
        break;
      case Continue::No:
        this->KeywordValueFunc = nullptr;
        break;
    }
    ++this->KeywordValuesSeen;
    return;
  }

  if (!this->DoneWithPositional) {
    auto const pit = this->Bindings.Positions.Find(pos);
    if (pit != this->Bindings.Positions.end()) {
      pit->second(*this, pos, arg);
      return;
    }
  }

  if (this->UnparsedArguments != nullptr) {
    this->UnparsedArguments->emplace_back(arg);
  }
}

void Instance::FinishKeyword()
{
  if (this->Keyword.empty()) {
    return;
  }
  if (this->KeywordValuesSeen < this->KeywordValuesExpected) {
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
