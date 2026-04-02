/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <set>
#include <string>

#include <cm/optional>

#include "cmConstStack.h"

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
  std::shared_ptr<cmPackageInformation const> PackageInfo;
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
