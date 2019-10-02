/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmArgumentParser_h
#define cmArgumentParser_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <cassert>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cm_static_string_view.hxx"

namespace ArgumentParser {

using StringList = std::vector<std::string>;
using MultiStringList = std::vector<StringList>;

class Instance;
using Action = std::function<void(Instance&, void*)>;

// using ActionMap = cm::flat_map<cm::string_view, Action>;
class ActionMap : public std::vector<std::pair<cm::string_view, Action>>
{
public:
  std::pair<iterator, bool> Emplace(cm::string_view name, Action action);
  const_iterator Find(cm::string_view name) const;
};

class Instance
{
public:
  Instance(ActionMap const& bindings)
    : Bindings(bindings)
  {
  }

  void Bind(bool& val);
  void Bind(std::string& val);
  void Bind(StringList& val);
  void Bind(MultiStringList& val);

  void Consume(cm::string_view arg, void* result,
               std::vector<std::string>* unparsedArguments,
               std::vector<std::string>* keywordsMissingValue,
               std::vector<std::string>* parsedKeywords);

private:
  ActionMap const& Bindings;
  std::string* CurrentString = nullptr;
  StringList* CurrentList = nullptr;
  bool ExpectValue = false;
};

} // namespace ArgumentParser

template <typename Result>
class cmArgumentParser
{
public:
  // I *think* this function could be made `constexpr` when the code is
  // compiled as C++20.  This would allow building a parser at compile time.
  template <typename T>
  cmArgumentParser& Bind(cm::static_string_view name, T Result::*member)
  {
    bool const inserted =
      this->Bindings
        .Emplace(name,
                 [member](ArgumentParser::Instance& instance, void* result) {
                   instance.Bind(static_cast<Result*>(result)->*member);
                 })
        .second;
    assert(inserted), (void)inserted;
    return *this;
  }

  template <typename Range>
  void Parse(Result& result, Range const& args,
             std::vector<std::string>* unparsedArguments = nullptr,
             std::vector<std::string>* keywordsMissingValue = nullptr,
             std::vector<std::string>* parsedKeywords = nullptr) const
  {
    ArgumentParser::Instance instance(this->Bindings);
    for (cm::string_view arg : args) {
      instance.Consume(arg, &result, unparsedArguments, keywordsMissingValue,
                       parsedKeywords);
    }
  }

  template <typename Range>
  Result Parse(Range const& args,
               std::vector<std::string>* unparsedArguments = nullptr,
               std::vector<std::string>* keywordsMissingValue = nullptr,
               std::vector<std::string>* parsedKeywords = nullptr) const
  {
    Result result;
    this->Parse(result, args, unparsedArguments, keywordsMissingValue,
                parsedKeywords);
    return result;
  }

private:
  ArgumentParser::ActionMap Bindings;
};

template <>
class cmArgumentParser<void>
{
public:
  template <typename T>
  cmArgumentParser& Bind(cm::static_string_view name, T& ref)
  {
    bool const inserted = this->Bind(cm::string_view(name), ref);
    assert(inserted), (void)inserted;
    return *this;
  }

  template <typename Range>
  void Parse(Range const& args,
             std::vector<std::string>* unparsedArguments = nullptr,
             std::vector<std::string>* keywordsMissingValue = nullptr,
             std::vector<std::string>* parsedKeywords = nullptr) const
  {
    ArgumentParser::Instance instance(this->Bindings);
    for (cm::string_view arg : args) {
      instance.Consume(arg, nullptr, unparsedArguments, keywordsMissingValue,
                       parsedKeywords);
    }
  }

protected:
  template <typename T>
  bool Bind(cm::string_view name, T& ref)
  {
    return this->Bindings
      .Emplace(name,
               [&ref](ArgumentParser::Instance& instance, void*) {
                 instance.Bind(ref);
               })
      .second;
  }

private:
  ArgumentParser::ActionMap Bindings;
};

#endif
