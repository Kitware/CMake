/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

class cmPlaceholderExpander
{
public:
  virtual ~cmPlaceholderExpander() = default;

  std::string& ExpandVariables(std::string& string);

protected:
  virtual std::string ExpandVariable(std::string const& variable) = 0;
};
