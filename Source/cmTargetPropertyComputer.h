/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <utility>

#include "cmStringAlgorithms.h"
#include "cmTargetTypes.h"
#include "cmValue.h"

class cmMakefile;

class cmTargetPropertyComputer
{
public:
  template <typename Target>
  static cmValue GetProperty(Target const* tgt, std::string const& prop,
                             cmMakefile const& mf)
  {
    if (cmValue loc = GetLocation(tgt, prop, mf)) {
      return loc;
    }
    if (prop == "SOURCES") {
      return GetSources(tgt);
    }
    return nullptr;
  }

  // True if `prop` is a *computed* location property for `type` -- i.e. one of
  // LOCATION, LOCATION_<CONFIG>, or <CONFIG>_LOCATION on a target type that
  // synthesizes a location.  Reading one of these from a non-imported target
  // is an error.  Excludes IMPORTED_LOCATION and XCODE_ATTRIBUTE_*.  If
  // non-null, `config` is filled with the requested configuration ("" for
  // plain LOCATION).
  static bool IsComputedLocationProperty(cm::TargetType type,
                                         std::string const& prop,
                                         std::string* config = nullptr)
  {
    switch (type) {
      case cm::TargetType::EXECUTABLE:
      case cm::TargetType::STATIC_LIBRARY:
      case cm::TargetType::SHARED_LIBRARY:
      case cm::TargetType::MODULE_LIBRARY:
      case cm::TargetType::UNKNOWN_LIBRARY:
        break;
      default:
        return false;
    }
    if (prop == "LOCATION") {
      if (config) {
        config->clear();
      }
      return true;
    }
    if (cmHasLiteralPrefix(prop, "LOCATION_")) {
      if (config) {
        *config = prop.substr(9);
      }
      return true;
    }
    if (cmHasLiteralSuffix(prop, "_LOCATION") &&
        !cmHasLiteralPrefix(prop, "XCODE_ATTRIBUTE_")) {
      std::string c(prop.c_str(), prop.size() - 9);
      if (c != "IMPORTED") {
        if (config) {
          *config = std::move(c);
        }
        return true;
      }
    }
    return false;
  }

private:
  static void IssueLocationPropertyError(std::string const& tgtName,
                                         cmMakefile const& mf);

  template <typename Target>
  static std::string const& ImportedLocation(Target const* tgt,
                                             std::string const& config);

  template <typename Target>
  static cmValue GetLocation(Target const* tgt, std::string const& prop,
                             cmMakefile const& mf)
  {
    // Watch for special "computed" properties that are dependent on
    // other properties or variables.  Always recompute them.
    std::string config;
    if (!IsComputedLocationProperty(tgt->GetType(), prop, &config)) {
      return nullptr;
    }
    if (!tgt->IsImported()) {
      IssueLocationPropertyError(tgt->GetName(), mf);
      return nullptr;
    }
    return cmValue(ImportedLocation(tgt, config));
  }

  template <typename Target>
  static cmValue GetSources(Target const* tgt);
};
