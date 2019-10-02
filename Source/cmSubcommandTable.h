/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmSubcommandTable_h
#define cmSubcommandTable_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cm_static_string_view.hxx"

class cmExecutionStatus;

class cmSubcommandTable
{
public:
  using Command = bool (*)(std::vector<std::string> const&,
                           cmExecutionStatus&);

  using Elem = std::pair<cm::string_view, Command>;
  using InitElem = std::pair<cm::static_string_view, Command>;

  cmSubcommandTable(std::initializer_list<InitElem> init);

  bool operator()(cm::string_view key, std::vector<std::string> const& args,
                  cmExecutionStatus& status) const;

private:
  std::vector<Elem> Impl;
};

#endif
