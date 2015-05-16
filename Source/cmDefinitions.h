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
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
#include <unordered_map>
#else
#include "cmsys/hash_map.hxx"
#endif
#endif

#include <list>

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
  /** Get the value associated with a key; null if none.
      Store the result locally if it came from a parent.  */
  static const char* Get(const std::string& key,
                         std::list<cmDefinitions>::reverse_iterator rbegin,
                         std::list<cmDefinitions>::reverse_iterator rend);

  /** Set (or unset if null) a value associated with a key.  */
  void Set(const std::string& key, const char* value);

  void Erase(const std::string& key);

  /** Get the set of all local keys.  */
  std::vector<std::string> LocalKeys() const;

  std::vector<std::string>
  ClosureKeys(std::set<std::string>& bound) const;

  static cmDefinitions MakeClosure(
      std::list<cmDefinitions>::const_reverse_iterator rbegin,
      std::list<cmDefinitions>::const_reverse_iterator rend);

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

#if defined(CMAKE_BUILD_WITH_CMAKE)
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
  typedef std::unordered_map<std::string, Def> MapType;
#else
  typedef cmsys::hash_map<std::string, Def> MapType;
#endif
#else
  typedef std::map<std::string, Def> MapType;
#endif
  MapType Map;

  static Def const& GetInternal(const std::string& key,
    std::list<cmDefinitions>::reverse_iterator rbegin,
    std::list<cmDefinitions>::reverse_iterator rend);
  void MakeClosure(std::set<std::string>& undefined,
                   std::list<cmDefinitions>::const_reverse_iterator rbegin,
                   std::list<cmDefinitions>::const_reverse_iterator rend);
};

#endif
