/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include "cmStack.h"

class cmMakefile;

/**
 * Represents one call to find_package.
 */
class cmFindPackageCall
{
public:
  std::string Name;
  unsigned int Index;
};

/**
 * RAII type to manage the find_package call stack.
 */
// Note: implemented in cmMakefile.cxx
class cmFindPackageStackRAII
{
  cmMakefile* Makefile;
  cmFindPackageCall** Value = nullptr;

public:
  cmFindPackageStackRAII(cmMakefile* mf, std::string const& pkg);
  ~cmFindPackageStackRAII();

  cmFindPackageStackRAII(cmFindPackageStackRAII const&) = delete;
  cmFindPackageStackRAII& operator=(cmFindPackageStackRAII const&) = delete;

  /** Get a mutable pointer to the top of the stack.
      The pointer is invalidated if BindTop is called again or when the
      cmFindPackageStackRAII goes out of scope.  */
  void BindTop(cmFindPackageCall*& value);
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
