/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cassert>
#include <functional>
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

class Instance;
using KeywordAction = std::function<void(Instance&)>;
using KeywordNameAction = std::function<void(Instance&, cm::string_view)>;

// using KeywordActionMap = cm::flat_map<cm::string_view, KeywordAction>;
class KeywordActionMap
  : public std::vector<std::pair<cm::string_view, KeywordAction>>
{
public:
  std::pair<iterator, bool> Emplace(cm::string_view name,
                                    KeywordAction action);
  const_iterator Find(cm::string_view name) const;
};

class ActionMap
{
public:
  KeywordActionMap Keywords;
  KeywordNameAction KeywordMissingValue;
  KeywordNameAction ParsedKeyword;
};

class Base
{
public:
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

  void Bind(bool& val);
  void Bind(std::string& val);
  void Bind(Maybe<std::string>& val);
  void Bind(MaybeEmpty<std::vector<std::string>>& val);
  void Bind(NonEmpty<std::vector<std::string>>& val);
  void Bind(std::vector<std::vector<std::string>>& val);

  // cm::optional<> records the presence the keyword to which it binds.
  template <typename T>
  void Bind(cm::optional<T>& optVal)
  {
    if (!optVal) {
      optVal.emplace();
    }
    this->Bind(*optVal);
  }

  template <typename Range>
  void Parse(Range const& args)
  {
    for (cm::string_view arg : args) {
      this->Consume(arg);
    }
    this->FinishKeyword();
  }

private:
  ActionMap const& Bindings;
  ParseResult* ParseResults = nullptr;
  std::vector<std::string>* UnparsedArguments = nullptr;
  void* Result = nullptr;

  cm::string_view Keyword;
  std::string* CurrentString = nullptr;
  std::vector<std::string>* CurrentList = nullptr;
  bool ExpectValue = false;

  void Consume(cm::string_view arg);
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
             std::vector<std::string>* unparsedArguments) const
  {
    using ArgumentParser::AsParseResultPtr;
    ParseResult* parseResultPtr = AsParseResultPtr(result);
    Instance instance(this->Bindings, parseResultPtr, unparsedArguments,
                      &result);
    instance.Parse(args);
    return parseResultPtr ? static_cast<bool>(*parseResultPtr) : true;
  }

  template <typename Range>
  Result Parse(Range const& args,
               std::vector<std::string>* unparsedArguments) const
  {
    Result result;
    this->Parse(result, args, unparsedArguments);
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

  cmArgumentParser& BindParsedKeywords(std::vector<cm::string_view>& ref)
  {
    this->Base::BindParsedKeyword(
      [&ref](Instance&, cm::string_view arg) { ref.emplace_back(arg); });
    return *this;
  }

  template <typename Range>
  ParseResult Parse(Range const& args,
                    std::vector<std::string>* unparsedArguments) const
  {
    ParseResult parseResult;
    Instance instance(this->Bindings, &parseResult, unparsedArguments);
    instance.Parse(args);
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
