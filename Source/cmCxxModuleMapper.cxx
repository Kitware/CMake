/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCxxModuleMapper.h"

#include <cassert>
#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmScanDepFormat.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

CxxBmiLocation::CxxBmiLocation() = default;

CxxBmiLocation::CxxBmiLocation(std::string path)
  : BmiLocation(std::move(path))
{
}

CxxBmiLocation CxxBmiLocation::Unknown()
{
  return {};
}

CxxBmiLocation CxxBmiLocation::Private()
{
  return { std::string{} };
}

CxxBmiLocation CxxBmiLocation::Known(std::string path)
{
  return { std::move(path) };
}

bool CxxBmiLocation::IsKnown() const
{
  return this->BmiLocation.has_value();
}

bool CxxBmiLocation::IsPrivate() const
{
  if (auto const& loc = this->BmiLocation) {
    return loc->empty();
  }
  return false;
}

std::string const& CxxBmiLocation::Location() const
{
  if (auto const& loc = this->BmiLocation) {
    return *loc;
  }
  static std::string empty;
  return empty;
}

CxxBmiLocation CxxModuleLocations::BmiGeneratorPathForModule(
  std::string const& logical_name) const
{
  auto bmi_loc = this->BmiLocationForModule(logical_name);
  if (bmi_loc.IsKnown() && !bmi_loc.IsPrivate()) {
    bmi_loc =
      CxxBmiLocation::Known(this->PathForGenerator(bmi_loc.Location()));
  }
  return bmi_loc;
}

namespace {

struct TransitiveUsage
{
  TransitiveUsage(std::string name, std::string location, LookupMethod method)
    : LogicalName(std::move(name))
    , Location(std::move(location))
    , Method(method)
  {
  }

  std::string LogicalName;
  std::string Location;
  LookupMethod Method;
};

std::vector<TransitiveUsage> GetTransitiveUsages(
  CxxModuleLocations const& loc, std::vector<cmSourceReqInfo> const& required,
  CxxModuleUsage const& usages)
{
  std::set<std::string> transitive_usage_directs;
  std::set<std::string> transitive_usage_names;

  std::vector<TransitiveUsage> all_usages;

  for (auto const& r : required) {
    auto bmi_loc = loc.BmiGeneratorPathForModule(r.LogicalName);
    if (bmi_loc.IsKnown()) {
      all_usages.emplace_back(r.LogicalName, bmi_loc.Location(), r.Method);
      transitive_usage_directs.insert(r.LogicalName);

      // Insert transitive usages.
      auto transitive_usages = usages.Usage.find(r.LogicalName);
      if (transitive_usages != usages.Usage.end()) {
        transitive_usage_names.insert(transitive_usages->second.begin(),
                                      transitive_usages->second.end());
      }
    }
  }

  for (auto const& transitive_name : transitive_usage_names) {
    if (transitive_usage_directs.count(transitive_name)) {
      continue;
    }

    auto module_ref = usages.Reference.find(transitive_name);
    if (module_ref != usages.Reference.end()) {
      all_usages.emplace_back(transitive_name, module_ref->second.Path,
                              module_ref->second.Method);
    }
  }

  return all_usages;
}

std::string CxxModuleMapContentClang(CxxModuleLocations const& loc,
                                     cmScanDepInfo const& obj,
                                     CxxModuleUsage const& usages)
{
  std::stringstream mm;

  // Clang's command line only supports a single output. If more than one is
  // expected, we cannot make a useful module map file.
  if (obj.Provides.size() > 1) {
    return {};
  }

  // A series of flags which tell the compiler where to look for modules.

  for (auto const& p : obj.Provides) {
    auto bmi_loc = loc.BmiGeneratorPathForModule(p.LogicalName);
    if (bmi_loc.IsKnown()) {
      // Force the TU to be considered a C++ module source file regardless of
      // extension.
      mm << "-x c++-module\n";

      mm << "-fmodule-output=" << bmi_loc.Location() << '\n';
      break;
    }
  }

  auto all_usages = GetTransitiveUsages(loc, obj.Requires, usages);
  for (auto const& usage : all_usages) {
    mm << "-fmodule-file=" << usage.LogicalName << '=' << usage.Location
       << '\n';
  }

  return mm.str();
}

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
  mm << "$root " << loc.RootDirectory << '\n';

  for (auto const& p : obj.Provides) {
    auto bmi_loc = loc.BmiGeneratorPathForModule(p.LogicalName);
    if (bmi_loc.IsKnown()) {
      mm << p.LogicalName << ' ' << bmi_loc.Location() << '\n';
    }
  }
  for (auto const& r : obj.Requires) {
    auto bmi_loc = loc.BmiGeneratorPathForModule(r.LogicalName);
    if (bmi_loc.IsKnown()) {
      mm << r.LogicalName << ' ' << bmi_loc.Location() << '\n';
    }
  }

  return mm.str();
}

std::string CxxModuleMapContentMsvc(CxxModuleLocations const& loc,
                                    cmScanDepInfo const& obj,
                                    CxxModuleUsage const& usages)
{
  std::stringstream mm;

  // A response file of `-reference NAME=PATH` arguments.

  // MSVC's command line only supports a single output. If more than one is
  // expected, we cannot make a useful module map file.
  if (obj.Provides.size() > 1) {
    return {};
  }

  auto flag_for_method = [](LookupMethod method) -> cm::static_string_view {
    switch (method) {
      case LookupMethod::ByName:
        return "-reference"_s;
      case LookupMethod::IncludeAngle:
        return "-headerUnit:angle"_s;
      case LookupMethod::IncludeQuote:
        return "-headerUnit:quote"_s;
    }
    assert(false && "unsupported lookup method");
    return ""_s;
  };

  for (auto const& p : obj.Provides) {
    if (p.IsInterface) {
      mm << "-interface\n";
    } else {
      mm << "-internalPartition\n";
    }

    auto bmi_loc = loc.BmiGeneratorPathForModule(p.LogicalName);
    if (bmi_loc.IsKnown()) {
      mm << "-ifcOutput " << bmi_loc.Location() << '\n';
    }
  }

  auto all_usages = GetTransitiveUsages(loc, obj.Requires, usages);
  for (auto const& usage : all_usages) {
    auto flag = flag_for_method(usage.Method);

    mm << flag << ' ' << usage.LogicalName << '=' << usage.Location << '\n';
  }

  return mm.str();
}
}

bool CxxModuleUsage::AddReference(std::string const& logical,
                                  std::string const& loc, LookupMethod method)
{
  auto r = this->Reference.find(logical);
  if (r != this->Reference.end()) {
    auto& ref = r->second;

    if (ref.Path == loc && ref.Method == method) {
      return true;
    }

    auto method_name = [](LookupMethod m) -> cm::static_string_view {
      switch (m) {
        case LookupMethod::ByName:
          return "by-name"_s;
        case LookupMethod::IncludeAngle:
          return "include-angle"_s;
        case LookupMethod::IncludeQuote:
          return "include-quote"_s;
      }
      assert(false && "unsupported lookup method");
      return ""_s;
    };

    cmSystemTools::Error(cmStrCat("Disagreement of the location of the '",
                                  logical,
                                  "' module. "
                                  "Location A: '",
                                  ref.Path, "' via ", method_name(ref.Method),
                                  "; "
                                  "Location B: '",
                                  loc, "' via ", method_name(method), "."));
    return false;
  }

  auto& ref = this->Reference[logical];
  ref.Path = loc;
  ref.Method = method;

  return true;
}

cm::static_string_view CxxModuleMapExtension(
  cm::optional<CxxModuleMapFormat> format)
{
  if (format) {
    switch (*format) {
      case CxxModuleMapFormat::Clang:
        return ".pcm"_s;
      case CxxModuleMapFormat::Gcc:
        return ".gcm"_s;
      case CxxModuleMapFormat::Msvc:
        return ".ifc"_s;
    }
  }

  return ".bmi"_s;
}

std::set<std::string> CxxModuleUsageSeed(
  CxxModuleLocations const& loc, std::vector<cmScanDepInfo> const& objects,
  CxxModuleUsage& usages)
{
  // Track inner usages to populate usages from internal bits.
  //
  // This is a map of modules that required some other module that was not
  // found to those that were not found.
  std::map<std::string, std::set<std::string>> internal_usages;
  std::set<std::string> unresolved;

  for (cmScanDepInfo const& object : objects) {
    // Add references for each of the provided modules.
    for (auto const& p : object.Provides) {
      auto bmi_loc = loc.BmiGeneratorPathForModule(p.LogicalName);
      if (bmi_loc.IsKnown()) {
        // XXX(cxx-modules): How to support header units?
        usages.AddReference(p.LogicalName, bmi_loc.Location(),
                            LookupMethod::ByName);
      }
    }

    // For each requires, pull in what is required.
    for (auto const& r : object.Requires) {
      // Find the required name in the current target.
      auto bmi_loc = loc.BmiGeneratorPathForModule(r.LogicalName);
      if (bmi_loc.IsPrivate()) {
        cmSystemTools::Error(
          cmStrCat("Unable to use module '", r.LogicalName,
                   "' as it is 'PRIVATE' and therefore not accessible outside "
                   "of its owning target."));
        continue;
      }

      // Find transitive usages.
      auto transitive_usages = usages.Usage.find(r.LogicalName);

      for (auto const& p : object.Provides) {
        auto& this_usages = usages.Usage[p.LogicalName];

        // Add the direct usage.
        this_usages.insert(r.LogicalName);

        // Add the transitive usage.
        if (transitive_usages != usages.Usage.end()) {
          this_usages.insert(transitive_usages->second.begin(),
                             transitive_usages->second.end());
        } else if (bmi_loc.IsKnown()) {
          // Mark that we need to update transitive usages later.
          internal_usages[p.LogicalName].insert(r.LogicalName);
        }
      }

      if (bmi_loc.IsKnown()) {
        usages.AddReference(r.LogicalName, bmi_loc.Location(), r.Method);
      }
    }
  }

  // While we have internal usages to manage.
  while (!internal_usages.empty()) {
    size_t starting_size = internal_usages.size();

    // For each internal usage.
    for (auto usage = internal_usages.begin(); usage != internal_usages.end();
         /* see end of loop */) {
      auto& this_usages = usages.Usage[usage->first];

      for (auto use = usage->second.begin(); use != usage->second.end();
           /* see end of loop */) {
        // Check if this required module uses other internal modules; defer
        // if so.
        if (internal_usages.count(*use)) {
          // Advance the iterator.
          ++use;
          continue;
        }

        auto transitive_usages = usages.Usage.find(*use);
        if (transitive_usages != usages.Usage.end()) {
          this_usages.insert(transitive_usages->second.begin(),
                             transitive_usages->second.end());
        }

        // Remove the entry and advance the iterator.
        use = usage->second.erase(use);
      }

      // Erase the entry if it doesn't have any remaining usages.
      if (usage->second.empty()) {
        usage = internal_usages.erase(usage);
      } else {
        ++usage;
      }
    }

    // Check that at least one usage was resolved.
    if (starting_size == internal_usages.size()) {
      // Nothing could be resolved this loop; we have a cycle, so record the
      // cycle and exit.
      for (auto const& usage : internal_usages) {
        unresolved.insert(usage.first);
      }
      break;
    }
  }

  return unresolved;
}

std::string CxxModuleMapContent(CxxModuleMapFormat format,
                                CxxModuleLocations const& loc,
                                cmScanDepInfo const& obj,
                                CxxModuleUsage const& usages)
{
  switch (format) {
    case CxxModuleMapFormat::Clang:
      return CxxModuleMapContentClang(loc, obj, usages);
    case CxxModuleMapFormat::Gcc:
      return CxxModuleMapContentGcc(loc, obj);
    case CxxModuleMapFormat::Msvc:
      return CxxModuleMapContentMsvc(loc, obj, usages);
  }

  assert(false);
  return {};
}
