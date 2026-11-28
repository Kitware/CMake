/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDefinitions.h"

#include <cassert>
#include <unordered_set>
#include <utility>

#include <cm/string_view>

void cmDefinitions::SetInternal(cm::string_view key,
                                std::shared_ptr<std::string const> value)
{
  auto i = this->Map.find(key);
  if (i != this->Map.end()) {
    i->second.Value = std::move(value);
    return;
  }
  Def def{ std::make_shared<std::string const>(key), std::move(value) };
  this->Map.emplace(*def.Key, std::move(def));
}

cmDefinitions::Def const& cmDefinitions::GetInternal(cm::string_view key,
                                                     StackIter begin,
                                                     StackIter end,
                                                     Lowering lowering)
{
  assert(begin != end);
  // Look for a binding in the current scope.
  {
    auto it = begin->Map.find(key);
    if (it != begin->Map.end()) {
      return it->second;
    }
  }
  // No binding in the current scope.  Try the parent scope.
  StackIter it = begin;
  ++it;
  if (it == end) {
    // No parent scope.  This binding does not exist.
    static Def const noDef{};
    return noDef;
  }
  // Look for a binding in the parent scope.
  Def const& parentDef = cmDefinitions::GetInternal(key, it, end, lowering);
  if (lowering == Lowering::Yes) {
    // Lower the binding to the current scope explicitly, even if unset.
    Def def{ parentDef.Key ? parentDef.Key
                           : std::make_shared<std::string const>(key),
             parentDef.Value };
    return begin->Map.emplace(*def.Key, std::move(def)).first->second;
  }
  // Return the binding from the parent scope.
  return parentDef;
}

cmValue cmDefinitions::Get(cm::string_view key, StackIter begin, StackIter end)
{
  Def const& def = cmDefinitions::GetInternal(key, begin, end, Lowering::No);
  return def.Value ? cmValue(def.Value.get()) : nullptr;
}

void cmDefinitions::Lower(cm::string_view key, StackIter begin, StackIter end)
{
  cmDefinitions::GetInternal(key, begin, end, Lowering::Yes);
}

bool cmDefinitions::HasKey(cm::string_view key, StackIter begin, StackIter end)
{
  for (StackIter it = begin; it != end; ++it) {
    if (it->Map.find(key) != it->Map.end()) {
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
          undefined.find(mi.first) == undefined.end()) {
        if (mi.second.Value) {
          closure.Map.insert(mi);
        } else {
          undefined.emplace(mi.first);
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
      if (bound.emplace(mi.first).second &&
          (unsetKeys == UnsetKeys::Include || mi.second.Value)) {
        defined.emplace_back(*mi.second.Key);
      }
    }
  }

  return defined;
}

void cmDefinitions::Set(cm::string_view key, std::string value)
{
  this->SetInternal(key,
                    std::make_shared<std::string const>(std::move(value)));
}

void cmDefinitions::Unset(cm::string_view key)
{
  this->SetInternal(key, {});
}
