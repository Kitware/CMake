/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include "cmConstStack.h"

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
 * Represents a stack of find_package calls with efficient value semantics.
 */
class cmFindPackageStack
  : public cmConstStack<cmFindPackageCall, cmFindPackageStack>
{
  using cmConstStack::cmConstStack;
  friend class cmConstStack<cmFindPackageCall, cmFindPackageStack>;
};
#ifndef cmFindPackageStack_cxx
extern template class cmConstStack<cmFindPackageCall, cmFindPackageStack>;
#endif
