/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmExpandedCommandArgument
 * \brief Represents an expanded command argument
 *
 * cmCommandArgument stores a string representing an expanded
 * command argument and context information.
 */

class cmExpandedCommandArgument
{
public:
  cmExpandedCommandArgument();
  cmExpandedCommandArgument(std::string value, bool quoted);

  std::string const& GetValue() const;

  bool WasQuoted() const;

  bool operator==(const char* value) const;
  bool operator==(std::string const& value) const;

  bool empty() const;

private:
  std::string Value;
  bool Quoted = false;
};
