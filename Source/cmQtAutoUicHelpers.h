/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <set>
#include <string>

#include "cmsys/RegularExpression.hxx"

class cmQtAutoUicHelpers
{
public:
  cmQtAutoUicHelpers();
  virtual ~cmQtAutoUicHelpers() = default;
  void CollectUicIncludes(std::set<std::string>& includes,
                          const std::string& content) const;

private:
  cmsys::RegularExpression RegExpInclude;
};
