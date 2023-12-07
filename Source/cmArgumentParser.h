/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cm/type_traits>
#include <cmext/string_view>

#include "cmArgumentParserTypes.h" // IWYU pragma: keep

template <typename Result>
class cmArgumentParser; // IWYU pragma: keep

class cmMakefile;

namespace ArgumentParser {

class ParseResult
{
  std::map<cm::string_view, std::string> KeywordErrors;

public:
  explicit operator bool() const { return this->KeywordErrors.empty(); }

  void AddKeywordError(cm::string_view key, cm::string_view text)

  {
    this->KeywordErrors[key] += text;
  }

  std::map<cm::string_view, std::string> const& GetKeywordErrors() const
  {
    return this->KeywordErrors;
  }

  bool MaybeReportError(cmMakefile& mf) const;
};

template <typename Result>
typename std::enable_if<std::is_base_of<ParseResult, Result>::value,
                        ParseResult*>::type
AsParseResultPtr(Result& result)
{
  return &result;
}

template <typename Result>
typename std::enable_if<!std::is_base_of<ParseResult, Result>::value,
                        ParseResult*>::type
AsParseResultPtr(Result&)
{
  return nullptr;
}

enum class Continue
{
  No,
  Yes,
};

struct ExpectAtLeast
{
  std::size_t Count = 0;

  ExpectAtLeast(std::size_t count)
    : Count(count)
  {
  }
};

class Instance;
using KeywordAction = std::function<void(Instance&)>;
using KeywordNameAction = std::function<void(Instance&, cm::string_view)>;
using PositionAction =
  std::function<void(Instance&, std::size_t, cm::string_view)>;

// using KeywordActionMap = cm::flat_map<cm::string_view, KeywordAction>;
class KeywordActionMap
  : public std::vector<std::pair<cm::string_view, KeywordAction>>
{
public:
  std::pair<iterator, bool> Emplace(cm::string_view name,
                                    KeywordAction action);
  const_iterator Find(cm::string_view name) const;
};

// using PositionActionMap = cm::flat_map<cm::string_view, PositionAction>;
class PositionActionMap
  : public std::vector<std::pair<std::size_t, PositionAction>>
{
public:
  std::pair<iterator, bool> Emplace(std::size_t pos, PositionAction action);
  const_iterator Find(std::size_t pos) const;
};

class ActionMap
{
public:
  KeywordActionMap Keywords;
  KeywordNameAction KeywordMissingValue;
  KeywordNameAction ParsedKeyword;
  PositionActionMap Positions;
};

class Base
{
public:
  using ExpectAtLeast = ArgumentParser::ExpectAtLeast;
  using Continue = ArgumentParser::Continue;
  using Instance = ArgumentParser::Instance;
  using ParseResult = ArgumentParser::ParseResult;

  ArgumentParser::ActionMap Bindings;

  bool MaybeBind(cm::string_view name, KeywordAction action)
  {
    return this->Bindings.Keywords.Emplace(name, std::move(action)).second;
  }

  void Bind(cm::string_view name, KeywordAction action)
  {
    bool const inserted = this->MaybeBind(name, std::move(action));
    assert(inserted);
    static_cast<void>(inserted);
  }

  void BindParsedKeyword(KeywordNameAction action)
  {
    assert(!this->Bindings.ParsedKeyword);
    this->Bindings.ParsedKeyword = std::move(action);
  }

  void BindKeywordMissingValue(KeywordNameAction action)
  {
    assert(!this->Bindings.KeywordMissingValue);
    this->Bindings.KeywordMissingValue = std::move(action);
  }

  void Bind(std::size_t pos, PositionAction action)
  {
    bool const inserted =
      this->Bindings.Positions.Emplace(pos, std::move(action)).second;
    assert(inserted);
    static_cast<void>(inserted);
  }
};

class Instance
{
public:
  Instance(ActionMap const& bindings, ParseResult* parseResult,
           std::vector<std::string>* unparsedArguments, void* result = nullptr)
    : Bindings(bindings)
    , ParseResults(parseResult)
    , UnparsedArguments(unparsedArguments)
    , Result(result)
  {
  }

  void Bind(std::function<Continue(cm::string_view)> f, ExpectAtLeast expect);
  void Bind(bool& val);
  void Bind(std::string& val);
  void Bind(NonEmpty<std::string>& val);
  void Bind(Maybe<std::string>& val);
  void Bind(MaybeEmpty<std::vector<std::string>>& val);
  void Bind(NonEmpty<std::vector<std::string>>& val);
  void Bind(std::vector<std::vector<std::string>>& val);
  template <typename U>
  void Bind(NonEmpty<std::vector<std::pair<std::string, U>>>& val,
            U const& context)
  {
    this->Bind(
      [&val, &context](cm::string_view arg) -> Continue {
        val.emplace_back(std::string(arg), context);
        return Continue::Yes;
      },
      ExpectAtLeast{ 1 });
  }

  // cm::optional<> records the presence the keyword to which it binds.
  template <typename T>
  void Bind(cm::optional<T>& optVal)
  {
    if (!optVal) {
      optVal.emplace();
    }
    this->Bind(*optVal);
  }

  template <typename T, typename U>
  void Bind(cm::optional<T>& optVal, U const& context)
  {
    if (!optVal) {
      optVal.emplace();
    }
    this->Bind(*optVal, context);
  }

  template <typename Range>
  void Parse(Range const& args, std::size_t pos = 0)
  {
    for (cm::string_view arg : args) {
      this->Consume(pos++, arg);
    }
    this->FinishKeyword();
  }

private:
  ActionMap const& Bindings;
  ParseResult* ParseResults = nullptr;
  std::vector<std::string>* UnparsedArguments = nullptr;
  void* Result = nullptr;

  cm::string_view Keyword;
  std::size_t KeywordValuesSeen = 0;
  std::size_t KeywordValuesExpected = 0;
  std::function<Continue(cm::string_view)> KeywordValueFunc;
  bool DoneWithPositional = false;

  void Consume(std::size_t pos, cm::string_view arg);
  void FinishKeyword();

  template <typename Result>
  friend class ::cmArgumentParser;
};

} // namespace ArgumentParser

template <typename Result>
class cmArgumentParser : private ArgumentParser::Base
{
public:
  // I *think* this function could be made `constexpr` when the code is
  // compiled as C++20.  This would allow building a parser at compile time.
  template <typename T>
  cmArgumentParser& Bind(cm::static_string_view name, T Result::*member)
  {
    this->Base::Bind(name, [member](Instance& instance) {
      instance.Bind(static_cast<Result*>(instance.Result)->*member);
    });
    return *this;
  }

  template <typename T, typename U>
  cmArgumentParser& BindWithContext(cm::static_string_view name,
                                    T Result::*member, U Result::*context)
  {
    this->Base::Bind(name, [member, context](Instance& instance) {
      auto* result = static_cast<Result*>(instance.Result);
      instance.Bind(result->*member, result->*context);
    });
    return *this;
  }

  cmArgumentParser& Bind(cm::static_string_view name,
                         Continue (Result::*member)(cm::string_view),
                         ExpectAtLeast expect = { 1 })
  {
    this->Base::Bind(name, [member, expect](Instance& instance) {
      Result* result = static_cast<Result*>(instance.Result);
      instance.Bind(
        [result, member](cm::string_view arg) -> Continue {
          return (result->*member)(arg);
        },
        expect);
    });
    return *this;
  }

  cmArgumentParser& Bind(cm::static_string_view name,
                         Continue (Result::*member)(cm::string_view,
                                                    cm::string_view),
                         ExpectAtLeast expect = { 1 })
  {
    this->Base::Bind(name, [member, expect](Instance& instance) {
      Result* result = static_cast<Result*>(instance.Result);
      cm::string_view keyword = instance.Keyword;
      instance.Bind(
        [result, member, keyword](cm::string_view arg) -> Continue {
          return (result->*member)(keyword, arg);
        },
        expect);
    });
    return *this;
  }

  cmArgumentParser& Bind(cm::static_string_view name,
                         std::function<Continue(Result&, cm::string_view)> f,
                         ExpectAtLeast expect = { 1 })
  {
    this->Base::Bind(name, [f, expect](Instance& instance) {
      Result* result = static_cast<Result*>(instance.Result);
      instance.Bind(
        [result, &f](cm::string_view arg) -> Continue {
          return f(*result, arg);
        },
        expect);
    });
    return *this;
  }

  cmArgumentParser& Bind(
    cm::static_string_view name,
    std::function<Continue(Result&, cm::string_view, cm::string_view)> f,
    ExpectAtLeast expect = { 1 })
  {
    this->Base::Bind(name, [f, expect](Instance& instance) {
      Result* result = static_cast<Result*>(instance.Result);
      cm::string_view keyword = instance.Keyword;
      instance.Bind(
        [result, keyword, &f](cm::string_view arg) -> Continue {
          return f(*result, keyword, arg);
        },
        expect);
    });
    return *this;
  }

  cmArgumentParser& Bind(std::size_t position,
                         cm::optional<std::string> Result::*member)
  {
    this->Base::Bind(
      position,
      [member](Instance& instance, std::size_t, cm::string_view arg) {
        Result* result = static_cast<Result*>(instance.Result);
        result->*member = arg;
      });
    return *this;
  }

  cmArgumentParser& BindParsedKeywords(
    std::vector<cm::string_view> Result::*member)
  {
    this->Base::BindParsedKeyword(
      [member](Instance& instance, cm::string_view arg) {
        (static_cast<Result*>(instance.Result)->*member).emplace_back(arg);
      });
    return *this;
  }

  template <typename Range>
  bool Parse(Result& result, Range const& args,
             std::vector<std::string>* unparsedArguments,
             std::size_t pos = 0) const
  {
    using ArgumentParser::AsParseResultPtr;
    ParseResult* parseResultPtr = AsParseResultPtr(result);
    Instance instance(this->Bindings, parseResultPtr, unparsedArguments,
                      &result);
    instance.Parse(args, pos);
    return parseResultPtr ? static_cast<bool>(*parseResultPtr) : true;
  }

  template <typename Range>
  Result Parse(Range const& args, std::vector<std::string>* unparsedArguments,
               std::size_t pos = 0) const
  {
    Result result;
    this->Parse(result, args, unparsedArguments, pos);
    return result;
  }
};

template <>
class cmArgumentParser<void> : private ArgumentParser::Base
{
public:
  template <typename T>
  cmArgumentParser& Bind(cm::static_string_view name, T& ref)
  {
    this->Base::Bind(name, [&ref](Instance& instance) { instance.Bind(ref); });
    return *this;
  }

  cmArgumentParser& Bind(cm::static_string_view name,
                         std::function<Continue(cm::string_view)> f,
                         ExpectAtLeast expect = { 1 })
  {
    this->Base::Bind(name, [f, expect](Instance& instance) {
      instance.Bind([&f](cm::string_view arg) -> Continue { return f(arg); },
                    expect);
    });
    return *this;
  }

  cmArgumentParser& Bind(
    cm::static_string_view name,
    std::function<Continue(cm::string_view, cm::string_view)> f,
    ExpectAtLeast expect = { 1 })
  {
    this->Base::Bind(name, [f, expect](Instance& instance) {
      cm::string_view keyword = instance.Keyword;
      instance.Bind(
        [keyword, &f](cm::string_view arg) -> Continue {
          return f(keyword, arg);
        },
        expect);
    });
    return *this;
  }

  cmArgumentParser& Bind(std::size_t position, cm::optional<std::string>& ref)
  {
    this->Base::Bind(position,
                     [&ref](Instance&, std::size_t, cm::string_view arg) {
                       ref = std::string(arg);
                     });
    return *this;
  }

  cmArgumentParser& BindParsedKeywords(std::vector<cm::string_view>& ref)
  {
    this->Base::BindParsedKeyword(
      [&ref](Instance&, cm::string_view arg) { ref.emplace_back(arg); });
    return *this;
  }

  template <typename Range>
  ParseResult Parse(Range const& args,
                    std::vector<std::string>* unparsedArguments,
                    std::size_t pos = 0) const
  {
    ParseResult parseResult;
    Instance instance(this->Bindings, &parseResult, unparsedArguments);
    instance.Parse(args, pos);
    return parseResult;
  }

protected:
  using Base::Instance;
  using Base::BindKeywordMissingValue;

  template <typename T>
  bool Bind(cm::string_view name, T& ref)
  {
    return this->MaybeBind(name,
                           [&ref](Instance& instance) { instance.Bind(ref); });
  }
};
