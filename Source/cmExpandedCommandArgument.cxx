/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExpandedCommandArgument.h"

#include <utility>

cmExpandedCommandArgument::cmExpandedCommandArgument() = default;

cmExpandedCommandArgument::cmExpandedCommandArgument(std::string value,
                                                     bool quoted)
  : Value(std::move(value))
  , Quoted(quoted)
{
}

std::string const& cmExpandedCommandArgument::GetValue() const
{
  return this->Value;
}

bool cmExpandedCommandArgument::WasQuoted() const
{
  return this->Quoted;
}

bool cmExpandedCommandArgument::operator==(const char* value) const
{
  return this->Value == value;
}

bool cmExpandedCommandArgument::operator==(std::string const& value) const
{
  return this->Value == value;
}

bool cmExpandedCommandArgument::empty() const
{
  return this->Value.empty();
}
