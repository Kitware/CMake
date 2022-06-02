/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCxxModuleMapper.h"

#include <cassert>

#include "cmScanDepFormat.h"

cm::optional<std::string> CxxModuleLocations::BmiGeneratorPathForModule(
  std::string const& logical_name) const
{
  if (auto l = this->BmiLocationForModule(logical_name)) {
    return this->PathForGenerator(*l);
  }
  return {};
}

cm::static_string_view CxxModuleMapExtension(
  cm::optional<CxxModuleMapFormat> format)
{
  return ".bmi"_s;
}

std::string CxxModuleMapContent(CxxModuleMapFormat format,
                                CxxModuleLocations const& loc,
                                cmScanDepInfo const& obj)
{
  assert(false);
  return {};
}
