/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmArgumentParser.h"

#include <algorithm>

#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"

namespace ArgumentParser {

auto KeywordActionMap::Emplace(cm::string_view name, KeywordAction action)
  -> std::pair<iterator, bool>
{
  auto const it = std::lower_bound(
    this->begin(), this->end(), name,
    [](value_type const& elem, cm::string_view k) { return elem.first < k; });
  return (it != this->end() && it->first == name)
    ? std::make_pair(it, false)
    : std::make_pair(this->emplace(it, name, std::move(action)), true);
}

auto KeywordActionMap::Find(cm::string_view name) const -> const_iterator
{
  auto const it = std::lower_bound(
    this->begin(), this->end(), name,
    [](value_type const& elem, cm::string_view k) { return elem.first < k; });
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
  this->GetState().KeywordValueFunc = std::move(f);
  this->GetState().KeywordValuesExpected = expect.Count;
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

void Instance::Bind(MaybeEmpty<std::string>& val)
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
        this->ParseResults->AddKeywordError(this->GetState().Keyword,
                                            "  empty string not allowed\n");
      }
      val = std::string(arg);
      return Continue::No;
    },
    ExpectAtLeast{ 1 });
}

void Instance::Bind(Maybe<std::string>& val)
{
  this->Bind(
    [&val](cm::string_view arg) -> Continue {
      val = std::string(arg);
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

void Instance::Consume(cm::string_view arg)
{
  ParserState& state = this->GetState();

  auto const it = state.Bindings.Keywords.Find(arg);
  if (it != state.Bindings.Keywords.end()) {
    this->FinishKeyword();
    state.Keyword = it->first;
    state.KeywordValuesSeen = 0;
    state.DoneWithPositional = true;
    if (state.Bindings.ParsedKeyword) {
      state.Bindings.ParsedKeyword(*this, it->first);
    }
    it->second(*this);
    return;
  }

  if (!state.DoneWithPositional) {
    auto const pit = state.Bindings.Positions.Find(state.Pos);
    if (pit != state.Bindings.Positions.end()) {
      pit->second(*this, state.Pos, arg);
      return;
    }

    if (state.Bindings.TrailingArgs) {
      state.Keyword = ""_s;
      state.KeywordValuesSeen = 0;
      state.DoneWithPositional = true;
      state.Bindings.TrailingArgs(*this);
      if (!state.KeywordValueFunc) {
        return;
      }
    }
  }

  if (state.KeywordValueFunc) {
    switch (state.KeywordValueFunc(arg)) {
      case Continue::Yes:
        break;
      case Continue::No:
        state.KeywordValueFunc = nullptr;
        break;
    }
    ++state.KeywordValuesSeen;
    return;
  }

  if (this->UnparsedArguments) {
    this->UnparsedArguments->emplace_back(arg);
  }
}

void Instance::FinishKeyword()
{
  ParserState const& state = this->GetState();
  if (!state.DoneWithPositional) {
    return;
  }

  if (state.KeywordValuesSeen < state.KeywordValuesExpected) {
    if (this->ParseResults) {
      this->ParseResults->AddKeywordError(state.Keyword,
                                          "  missing required value\n");
    }
    if (state.Bindings.KeywordMissingValue) {
      state.Bindings.KeywordMissingValue(*this, state.Keyword);
    }
  }
}

bool ParseResult::MaybeReportError(cmMakefile& mf) const
{
  if (*this) {
    return false;
  }
  std::string e;
  for (auto const& kel : this->KeywordErrors) {
    e = cmStrCat(e, "Error after keyword \"", kel.first, "\":\n");
    for (auto const& ke : kel.second) {
      e += ke;
    }
  }
  mf.IssueMessage(MessageType::FATAL_ERROR, e);
  return true;
}

bool ParseResult::Check(cm::string_view context,
                        std::vector<std::string> const* unparsedArguments,
                        cmExecutionStatus& status) const
{
  if (unparsedArguments && !unparsedArguments->empty()) {
    status.SetError(cmStrCat(context, " given unknown argument: \""_s,
                             unparsedArguments->front(), "\"."_s));
    return false;
  }

  if (!this->KeywordErrors.empty()) {
    std::string msg = cmStrCat(
      context, (context.empty() ? ""_s : " "_s), "given invalid "_s,
      (this->KeywordErrors.size() > 1 ? "arguments:"_s : "argument:"_s));
    for (auto const& kel : this->KeywordErrors) {
      for (auto const& ke : kel.second) {
        msg =
          cmStrCat(msg, "\n  "_s, kel.first, ": "_s, cmStripWhitespace(ke));
      }
    }
    status.SetError(msg);
    return false;
  }

  return true;
}

} // namespace ArgumentParser
