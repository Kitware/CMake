/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDefinitions.h"

#include <cassert>
#include <functional>
#include <unordered_set>
#include <utility>

#include <cm/string_view>

cmDefinitions::Def cmDefinitions::NoDef;

cmDefinitions::Def const& cmDefinitions::GetInternal(const std::string& key,
                                                     StackIter begin,
                                                     StackIter end, bool raise)
{
  assert(begin != end);
  {
    auto it = begin->Map.find(cm::String::borrow(key));
    if (it != begin->Map.end()) {
      it->second.Used = true;
      return it->second;
    }
  }
  StackIter it = begin;
  ++it;
  if (it == end) {
    return cmDefinitions::NoDef;
  }
  Def const& def = cmDefinitions::GetInternal(key, it, end, raise);
  if (!raise) {
    return def;
  }
  return begin->Map.emplace(key, def).first->second;
}

const std::string* cmDefinitions::Get(const std::string& key, StackIter begin,
                                      StackIter end)
{
  Def const& def = cmDefinitions::GetInternal(key, begin, end, false);
  return def.Value ? def.Value.str_if_stable() : nullptr;
}

void cmDefinitions::Raise(const std::string& key, StackIter begin,
                          StackIter end)
{
  cmDefinitions::GetInternal(key, begin, end, true);
}

bool cmDefinitions::HasKey(const std::string& key, StackIter begin,
                           StackIter end)
{
  for (StackIter it = begin; it != end; ++it) {
    if (it->Map.find(cm::String::borrow(key)) != it->Map.end()) {
      return true;
    }
  }
  return false;
}

cmDefinitions cmDefinitions::MakeClosure(StackIter begin, StackIter end)
{
  cmDefinitions closure;
  std::unordered_set<cm::string_view> undefined;
  for (StackIter it = begin; it != end; ++it) {
    // Consider local definitions.
    for (auto const& mi : it->Map) {
      // Use this key if it is not already set or unset.
      if (closure.Map.find(mi.first) == closure.Map.end() &&
          undefined.find(mi.first.view()) == undefined.end()) {
        if (mi.second.Value) {
          closure.Map.insert(mi);
        } else {
          undefined.emplace(mi.first.view());
        }
      }
    }
  }
  return closure;
}

std::vector<std::string> cmDefinitions::ClosureKeys(StackIter begin,
                                                    StackIter end)
{
  std::vector<std::string> defined;
  std::unordered_set<cm::string_view> bound;

  for (StackIter it = begin; it != end; ++it) {
    defined.reserve(defined.size() + it->Map.size());
    for (auto const& mi : it->Map) {
      // Use this key if it is not already set or unset.
      if (bound.emplace(mi.first.view()).second && mi.second.Value) {
        defined.push_back(*mi.first.str_if_stable());
      }
    }
  }

  return defined;
}

void cmDefinitions::Set(const std::string& key, cm::string_view value)
{
  this->Map[key] = Def(value);
}

void cmDefinitions::Unset(const std::string& key)
{
  this->Map[key] = Def();
}

std::vector<std::string> cmDefinitions::UnusedKeys() const
{
  std::vector<std::string> keys;
  keys.reserve(this->Map.size());
  // Consider local definitions.
  for (auto const& mi : this->Map) {
    if (!mi.second.Used) {
      keys.push_back(*mi.first.str_if_stable());
    }
  }
  return keys;
}
