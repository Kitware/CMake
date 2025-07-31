/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <set>
#include <string>

#include <cm/optional>

#include "cmStack.h"

class cmMakefile;

/**
 * This data represents the actual contents of find_package
 * <PACKAGE>-Config.cmake or <PACKAGE>.cps file, and not what is passed
 * to the find_package command. They can be the same, but it is not guaranteed.
 */

class cmPackageInformation
{
public:
  cm::optional<std::string> Directory;
  cm::optional<std::string> Version;
  cm::optional<std::string> Description;
  cm::optional<std::string> License;
  cm::optional<std::string> Website;
  cm::optional<std::string> PackageUrl;
  std::set<std::string> Components;
};

/**
 * Represents one call to find_package.
 */
class cmFindPackageCall
{
public:
  std::string const Name;
  cmPackageInformation PackageInfo;
  unsigned int Index;
};

/**
 * RAII type to manage the find_package call stack.
 */
// Note: implemented in cmMakefile.cxx
class cmFindPackageStackRAII
{
  cmMakefile* Makefile;
  cmPackageInformation** Value = nullptr;

public:
  cmFindPackageStackRAII(cmMakefile* mf, std::string const& pkg);
  ~cmFindPackageStackRAII();

  cmFindPackageStackRAII(cmFindPackageStackRAII const&) = delete;
  cmFindPackageStackRAII& operator=(cmFindPackageStackRAII const&) = delete;

  /** Get a mutable pointer to the top of the stack.
      The pointer is invalidated if BindTop is called again or when the
      cmFindPackageStackRAII goes out of scope.  */
  void BindTop(cmPackageInformation*& value);
};

/**
 * Represents a stack of find_package calls with efficient value semantics.
 */
class cmFindPackageStack
  : protected cmStack<cmFindPackageCall, cmFindPackageStack>
{
  using cmStack::cmStack;
  friend cmFindPackageStack::Base;
  friend class cmFindPackageStackRAII;

public:
  using cmStack::Push;
  using cmStack::Pop;
  using cmStack::Empty;

  cmFindPackageCall const& Top() const;
};
#ifndef cmFindPackageStack_cxx
extern template class cmStack<cmFindPackageCall, cmFindPackageStack>;

extern template cmFindPackageCall&
cmStack<cmFindPackageCall, cmFindPackageStack>::Top<true>();
#endif
