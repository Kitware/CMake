/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmDefinitions_h
#define cmDefinitions_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <cm/string_view>

#include "cmLinkedTree.h"
#include "cmString.hxx"

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
  // -- Static member functions

  static const std::string* Get(const std::string& key, StackIter begin,
                                StackIter end);

  static void Raise(const std::string& key, StackIter begin, StackIter end);

  static bool HasKey(const std::string& key, StackIter begin, StackIter end);

  static std::vector<std::string> ClosureKeys(StackIter begin, StackIter end);

  static cmDefinitions MakeClosure(StackIter begin, StackIter end);

  // -- Member functions

  /** Set a value associated with a key.  */
  void Set(const std::string& key, cm::string_view value);

  /** Unset a definition.  */
  void Unset(const std::string& key);

  /** List of unused keys.  */
  std::vector<std::string> UnusedKeys() const;

private:
  /** String with existence boolean.  */
  struct Def
  {
  public:
    Def() = default;
    Def(cm::string_view value)
      : Value(value)
    {
    }
    cm::String Value;
    bool Used = false;
  };
  static Def NoDef;

  std::unordered_map<cm::String, Def> Map;

  static Def const& GetInternal(const std::string& key, StackIter begin,
                                StackIter end, bool raise);
};

#endif
