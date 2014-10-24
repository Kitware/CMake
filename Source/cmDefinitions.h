/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDefinitions_h
#define cmDefinitions_h

#include "cmStandardIncludes.h"
#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmsys/hash_map.hxx"
#endif

/** \class cmDefinitions
 * \brief Store a scope of variable definitions for CMake language.
 *
 * This stores the state of variable definitions (set or unset) for
 * one scope.  Sets are always local.  Gets search parent scopes
 * transitively and save results locally.
 */
class cmDefinitions
{
public:
  /** Construct with the given parent scope.  */
  cmDefinitions(cmDefinitions* parent = 0);

  /** Reset object as if newly constructed.  */
  void Reset(cmDefinitions* parent = 0);

  /** Returns the parent scope, if any.  */
  cmDefinitions* GetParent() const { return this->Up; }

  /** Get the value associated with a key; null if none.
      Store the result locally if it came from a parent.  */
  const char* Get(const std::string& key);

  /** Set (or unset if null) a value associated with a key.  */
  const char* Set(const std::string& key, const char* value);

  /** Get the set of all local keys.  */
  std::set<std::string> LocalKeys() const;

  /** Compute the closure of all defined keys with values.
      This flattens the scope.  The result has no parent.  */
  cmDefinitions Closure() const;

  /** Compute the set of all defined keys.  */
  std::set<std::string> ClosureKeys() const;

private:
  // String with existence boolean.
  struct Def: public std::string
  {
  private:
    typedef std::string std_string;
  public:
    Def(): std_string(), Exists(false) {}
    Def(const char* v): std_string(v?v:""), Exists(v?true:false) {}
    Def(const std_string& v): std_string(v), Exists(true) {}
    Def(Def const& d): std_string(d), Exists(d.Exists) {}
    bool Exists;
  };
  static Def NoDef;

  // Parent scope, if any.
  cmDefinitions* Up;

  // Local definitions, set or unset.
#if defined(CMAKE_BUILD_WITH_CMAKE)
  typedef cmsys::hash_map<std::string, Def> MapType;
#else
  typedef std::map<std::string, Def> MapType;
#endif
  MapType Map;

  // Internal query and update methods.
  Def const& GetInternal(const std::string& key);
  Def const& SetInternal(const std::string& key, Def const& def);

  // Implementation of Closure() method.
  struct ClosureTag {};
  cmDefinitions(ClosureTag const&, cmDefinitions const* root);
  void ClosureImpl(std::set<std::string>& undefined,
                   cmDefinitions const* defs);

  // Implementation of ClosureKeys() method.
  void ClosureKeys(std::set<std::string>& defined,
                   std::set<std::string>& undefined) const;
};

#endif
