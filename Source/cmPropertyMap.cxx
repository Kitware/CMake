/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmPropertyMap.h"

#include <algorithm>
#include <utility>

void cmPropertyMap::Clear()
{
  this->Map_.clear();
}

void cmPropertyMap::SetProperty(const std::string& name, cmValue value)
{
  if (!value) {
    this->Map_.erase(name);
    return;
  }

  this->Map_[name] = *value;
}

void cmPropertyMap::AppendProperty(const std::string& name,
                                   const std::string& value, bool asString)
{
  // Skip if nothing to append.
  if (value.empty()) {
    return;
  }

  {
    std::string& pVal = this->Map_[name];
    if (!pVal.empty() && !asString) {
      pVal += ';';
    }
    pVal += value;
  }
}

void cmPropertyMap::RemoveProperty(const std::string& name)
{
  this->Map_.erase(name);
}

cmValue cmPropertyMap::GetPropertyValue(const std::string& name) const
{
  auto it = this->Map_.find(name);
  if (it != this->Map_.end()) {
    return cmValue(it->second);
  }
  return nullptr;
}

std::vector<std::string> cmPropertyMap::GetKeys() const
{
  std::vector<std::string> keyList;
  keyList.reserve(this->Map_.size());
  for (auto const& item : this->Map_) {
    keyList.push_back(item.first);
  }
  std::sort(keyList.begin(), keyList.end());
  return keyList;
}

std::vector<std::pair<std::string, std::string>> cmPropertyMap::GetList() const
{
  using StringPair = std::pair<std::string, std::string>;
  std::vector<StringPair> kvList;
  kvList.reserve(this->Map_.size());
  for (auto const& item : this->Map_) {
    kvList.emplace_back(item.first, item.second);
  }
  std::sort(kvList.begin(), kvList.end(),
            [](StringPair const& a, StringPair const& b) {
              return a.first < b.first;
            });
  return kvList;
}
