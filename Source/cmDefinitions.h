/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmDefinitions_h
#define cmDefinitions_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <unordered_map>
#include <vector>

#include "cmLinkedTree.h"

/** \class cmDefinitions
 * \brief Store a scope of variable definitions for CMake language.
 *
 * This stores the state of variable definitions (set or unset) for
 * one scope.  Sets are always local.  Gets search parent scopes
 * transitively and save results locally.
 */
class cmDefinitions
{
  typedef cmLinkedTree<cmDefinitions>::iterator StackIter;

public:
  static const std::string* Get(const std::string& key, StackIter begin,
                                StackIter end);

  static void Raise(const std::string& key, StackIter begin, StackIter end);

  static bool HasKey(const std::string& key, StackIter begin, StackIter end);

  /** Set (or unset if null) a value associated with a key.  */
  void Set(const std::string& key, const char* value);

  std::vector<std::string> UnusedKeys() const;

  static std::vector<std::string> ClosureKeys(StackIter begin, StackIter end);

  static cmDefinitions MakeClosure(StackIter begin, StackIter end);

private:
  // String with existence boolean.
  struct Def : public std::string
  {
  private:
    typedef std::string std_string;

  public:
    Def() = default;
    Def(const char* v)
      : std_string(v ? v : "")
      , Exists(v ? true : false)
    {
    }
    Def(const std_string& v)
      : std_string(v)
      , Exists(true)
    {
    }
    bool Exists = false;
    bool Used = false;
  };
  static Def NoDef;

  typedef std::unordered_map<std::string, Def> MapType;
  MapType Map;

  static Def const& GetInternal(const std::string& key, StackIter begin,
                                StackIter end, bool raise);
};

#endif
