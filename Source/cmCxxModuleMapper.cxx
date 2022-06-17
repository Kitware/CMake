/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCxxModuleMapper.h"

#include <cassert>
#include <sstream>
#include <vector>

#include "cmScanDepFormat.h"

cm::optional<std::string> CxxModuleLocations::BmiGeneratorPathForModule(
  std::string const& logical_name) const
{
  if (auto l = this->BmiLocationForModule(logical_name)) {
    return this->PathForGenerator(*l);
  }
  return {};
}

namespace {

std::string CxxModuleMapContentGcc(CxxModuleLocations const& loc,
                                   cmScanDepInfo const& obj)
{
  std::stringstream mm;

  // Documented in GCC's documentation. The format is a series of
  // lines with a module name and the associated filename separated
  // by spaces. The first line may use `$root` as the module name
  // to specify a "repository root". That is used to anchor any
  // relative paths present in the file (CMake should never
  // generate any).

  // Write the root directory to use for module paths.
  mm << "$root " << loc.RootDirectory << "\n";

  for (auto const& p : obj.Provides) {
    if (auto bmi_loc = loc.BmiGeneratorPathForModule(p.LogicalName)) {
      mm << p.LogicalName << ' ' << *bmi_loc << '\n';
    }
  }
  for (auto const& r : obj.Requires) {
    if (auto bmi_loc = loc.BmiGeneratorPathForModule(r.LogicalName)) {
      mm << r.LogicalName << ' ' << *bmi_loc << '\n';
    }
  }

  return mm.str();
}
}

cm::static_string_view CxxModuleMapExtension(
  cm::optional<CxxModuleMapFormat> format)
{
  if (format) {
    switch (*format) {
      case CxxModuleMapFormat::Gcc:
        return ".gcm"_s;
    }
  }

  return ".bmi"_s;
}

std::string CxxModuleMapContent(CxxModuleMapFormat format,
                                CxxModuleLocations const& loc,
                                cmScanDepInfo const& obj)
{
  switch (format) {
    case CxxModuleMapFormat::Gcc:
      return CxxModuleMapContentGcc(loc, obj);
  }

  assert(false);
  return {};
}
