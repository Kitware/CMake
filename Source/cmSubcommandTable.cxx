/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSubcommandTable.h"

#include <algorithm>

#include <cmext/string_view>

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
  std::string error = cmStrCat("does not recognize sub-command "_s, key);
  std::vector<std::string> candidates;
  candidates.reserve(this->Impl.size());
  for (Elem const& elem : this->Impl) {
    candidates.emplace_back(elem.first);
  }
  std::string const suggestion = cmFindClosestString(key, candidates);
  if (!suggestion.empty()) {
    error =
      cmStrCat(std::move(error), ". Did you mean: \""_s, suggestion, "\"?"_s);
  }
  status.SetError(error);
  return false;
}
