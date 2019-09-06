/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportSetMap.h"

#include <tuple>
#include <utility>

cmExportSet& cmExportSetMap::operator[](const std::string& name)
{
  auto it = this->find(name);
  if (it == this->end()) // Export set not found
  {
    auto tup_name = std::make_tuple(name);
    it = this->emplace(std::piecewise_construct, tup_name, tup_name).first;
  }
  return it->second;
}
