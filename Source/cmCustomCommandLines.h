/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCustomCommandLines_h
#define cmCustomCommandLines_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <initializer_list>
#include <string>
#include <vector>

#include <cm/string_view> // IWYU pragma: keep

/** Data structure to represent a single command line.  */
class cmCustomCommandLine : public std::vector<std::string>
{
};

/** Data structure to represent a list of command lines.  */
class cmCustomCommandLines : public std::vector<cmCustomCommandLine>
{
};

/** Return a command line from a list of command line parts.  */
cmCustomCommandLine cmMakeCommandLine(
  std::initializer_list<cm::string_view> ilist);

/** Return a command line vector with a single command line.  */
cmCustomCommandLines cmMakeSingleCommandLine(
  std::initializer_list<cm::string_view> ilist);

#endif
