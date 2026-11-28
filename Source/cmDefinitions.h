/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmLinkedTree.h"
#include "cmValue.h"

/** \class cmDefinitions
 * \brief Store a scope of variable definitions for CMake language.
 *
 * This stores the state of variable definitions (set or unset) for
 * one scope.  Sets are always local.  Gets search parent scopes
 * transitively and save results locally.
 */
class cmDefinitions
{
  using StackIter = cmLinkedTree<cmDefinitions>::iterator;

public:
  cmDefinitions() = default;
  cmDefinitions(cmDefinitions const&) = default;
  cmDefinitions& operator=(cmDefinitions const&) = default;
  cmDefinitions& operator=(cmDefinitions&&) = default;

  // The map's move constructor is noexcept for all cases except MSVC,
  // which makes the vector in cmLinkedTree copy instead of move when it grows.
  // MSVC only throws out of memory exceptions if we cannot allocate a small
  // sentinel node, at which point it will be near impossible to recover
  // already. Therefore this can be tagged as "essentially noexcept".
  cmDefinitions(cmDefinitions&& other) noexcept
    : Map(std::move(other.Map))
  {
  }

  // -- Static member functions

  static cmValue Get(cm::string_view key, StackIter begin, StackIter end);

  static void Lower(cm::string_view key, StackIter begin, StackIter end);

  static bool HasKey(cm::string_view key, StackIter begin, StackIter end);

  enum class UnsetKeys
  {
    Exclude,
    Include,
  };

  static std::vector<std::string> ClosureKeys(
    StackIter begin, StackIter end, UnsetKeys unsetKeys = UnsetKeys::Exclude);

  static cmDefinitions MakeClosure(StackIter begin, StackIter end);

  // -- Member functions

  /** Bind a key to the given value.  Stores the value as an owned string.  */
  void Set(cm::string_view key, std::string value);

  /** Bind a key to no value.  */
  void Unset(cm::string_view key);

private:
  struct Def
  {
    std::shared_ptr<std::string const> Key;
    std::shared_ptr<std::string const> Value;
  };

  // Index the definitions by views into their own key allocation.
  std::unordered_map<cm::string_view, Def> Map;

  void SetInternal(cm::string_view key,
                   std::shared_ptr<std::string const> value);

  enum class Lowering
  {
    No,
    Yes,
  };

  static Def const& GetInternal(cm::string_view key, StackIter begin,
                                StackIter end, Lowering lowering);
};
