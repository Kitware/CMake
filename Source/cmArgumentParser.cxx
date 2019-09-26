/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmArgumentParser.h"

#include <algorithm>
#include <type_traits>

namespace ArgumentParser {

auto ActionMap::Emplace(cm::string_view name, Action action)
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

auto ActionMap::Find(cm::string_view name) const -> const_iterator
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

void Instance::Bind(StringList& val)
{
  this->CurrentString = nullptr;
  this->CurrentList = &val;
  this->ExpectValue = true;
}

void Instance::Bind(MultiStringList& val)
{
  this->CurrentString = nullptr;
  this->CurrentList = (static_cast<void>(val.emplace_back()), &val.back());
  this->ExpectValue = false;
}

void Instance::Consume(cm::string_view arg, void* result,
                       std::vector<std::string>* unparsedArguments,
                       std::vector<std::string>* keywordsMissingValue,
                       std::vector<std::string>* parsedKeywords)
{
  auto const it = this->Bindings.Find(arg);
  if (it != this->Bindings.end()) {
    if (parsedKeywords != nullptr) {
      parsedKeywords->emplace_back(arg);
    }
    it->second(*this, result);
    if (this->ExpectValue && keywordsMissingValue != nullptr) {
      keywordsMissingValue->emplace_back(arg);
    }
    return;
  }

  if (this->CurrentString != nullptr) {
    this->CurrentString->assign(std::string(arg));
    this->CurrentString = nullptr;
    this->CurrentList = nullptr;
  } else if (this->CurrentList != nullptr) {
    this->CurrentList->emplace_back(arg);
  } else if (unparsedArguments != nullptr) {
    unparsedArguments->emplace_back(arg);
  }

  if (this->ExpectValue) {
    if (keywordsMissingValue != nullptr) {
      keywordsMissingValue->pop_back();
    }
    this->ExpectValue = false;
  }
}

} // namespace ArgumentParser
