/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCustomCommandLines.h"

cmCustomCommandLine cmMakeCommandLine(
  std::initializer_list<cm::string_view> ilist)
{
  cmCustomCommandLine commandLine;
  commandLine.reserve(ilist.size());
  for (cm::string_view cmd : ilist) {
    commandLine.emplace_back(cmd);
  }
  return commandLine;
}

cmCustomCommandLines cmMakeSingleCommandLine(
  std::initializer_list<cm::string_view> ilist)
{
  cmCustomCommandLines commandLines;
  commandLines.push_back(cmMakeCommandLine(ilist));
  return commandLines;
}
