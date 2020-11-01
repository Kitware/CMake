/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSubcommandTable.h"

#include <algorithm>

#include "cmExecutionStatus.h"
#include "cmStringAlgorithms.h"

cmSubcommandTable::cmSubcommandTable(std::initializer_list<InitElem> init)
  : Impl(init.begin(), init.end())
{
  std::sort(this->Impl.begin(), this->Impl.end(),
            [](Elem const& left, Elem const& right) {
              return left.first < right.first;
            });
}

bool cmSubcommandTable::operator()(cm::string_view key,
                                   std::vector<std::string> const& args,
                                   cmExecutionStatus& status) const
{
  auto const it = std::lower_bound(
    this->Impl.begin(), this->Impl.end(), key,
    [](Elem const& elem, cm::string_view k) { return elem.first < k; });
  if (it != this->Impl.end() && it->first == key) {
    return it->second(args, status);
  }
  status.SetError(cmStrCat("does not recognize sub-command ", key));
  return false;
}
