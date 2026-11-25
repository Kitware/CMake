/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDefinitions.h"

#include <cassert>
#include <unordered_set>
#include <utility>

#include <cm/string_view>

cmDefinitions::Def cmDefinitions::NoDef;

cmDefinitions::Def const& cmDefinitions::GetInternal(std::string const& key,
                                                     StackIter begin,
                                                     StackIter end,
                                                     Lowering lowering)
{
  assert(begin != end);
  // Look for a binding in the current scope.
  {
    auto it = begin->Map.find(cm::String::borrow(key));
    if (it != begin->Map.end()) {
      return it->second;
    }
  }
  // No binding in the current scope.  Try the parent scope.
  StackIter it = begin;
  ++it;
  if (it == end) {
    // No parent scope.  This binding does not exist.
    return cmDefinitions::NoDef;
  }
  // Look for a binding in the parent scope.
  Def const& parentDef = cmDefinitions::GetInternal(key, it, end, lowering);
  if (lowering == Lowering::Yes) {
    // Lower the binding to the current scope explicitly, even if unset.
    return begin->Map.emplace(key, parentDef).first->second;
  }
  // Return the binding from the parent scope.
  return parentDef;
}

cmValue cmDefinitions::Get(std::string const& key, StackIter begin,
                           StackIter end)
{
  Def const& def = cmDefinitions::GetInternal(key, begin, end, Lowering::No);
  return def.Value ? cmValue(def.Value.str_if_stable()) : nullptr;
}

void cmDefinitions::Lower(std::string const& key, StackIter begin,
                          StackIter end)
{
  cmDefinitions::GetInternal(key, begin, end, Lowering::Yes);
}

bool cmDefinitions::HasKey(std::string const& key, StackIter begin,
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
                                                    StackIter end,
                                                    UnsetKeys unsetKeys)
{
  std::vector<std::string> defined;
  std::unordered_set<cm::string_view> bound;

  for (StackIter it = begin; it != end; ++it) {
    defined.reserve(defined.size() + it->Map.size());
    for (auto const& mi : it->Map) {
      // Use this key if it is not already set or unset.
      if (bound.emplace(mi.first.view()).second &&
          (unsetKeys == UnsetKeys::Include || mi.second.Value)) {
        defined.push_back(*mi.first.str_if_stable());
      }
    }
  }

  return defined;
}

void cmDefinitions::Set(std::string const& key, cm::string_view value)
{
  this->Map[key] = Def(value);
}

void cmDefinitions::Unset(std::string const& key)
{
  this->Map[key] = Def();
}
