/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

class cmMakefile;
class cmTarget;
class cmValue;

enum class cmGetTargetPropertyResult
{
  Success,
  TargetNotFound,
};

cmGetTargetPropertyResult cmGetTargetProperty(std::string const& targetName,
                                              std::string const& propertyName,
                                              cmMakefile& mf,
                                              cmValue& propertyValue);

// Overload for callers that have already resolved the target.  The
// targetName is still required for ALIASED_TARGET / ALIAS_GLOBAL,
// which dispatch on whether the *lookup* name is an alias (not whether
// the resolved target is one).
cmGetTargetPropertyResult cmGetTargetProperty(std::string const& targetName,
                                              cmTarget const* target,
                                              std::string const& propertyName,
                                              cmMakefile& mf,
                                              cmValue& propertyValue);
