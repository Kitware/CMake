/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
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
    if (tgt->GetType() == cm::TargetType::EXECUTABLE ||
        tgt->GetType() == cm::TargetType::STATIC_LIBRARY ||
        tgt->GetType() == cm::TargetType::SHARED_LIBRARY ||
        tgt->GetType() == cm::TargetType::MODULE_LIBRARY ||
        tgt->GetType() == cm::TargetType::UNKNOWN_LIBRARY) {
      static std::string const propLOCATION = "LOCATION";
      if (prop == propLOCATION) {
        if (!tgt->IsImported()) {
          IssueLocationPropertyError(tgt->GetName(), mf);
          return nullptr;
        }
        return cmValue(ImportedLocation(tgt, std::string()));
      }

      // Support "LOCATION_<CONFIG>".
      if (cmHasLiteralPrefix(prop, "LOCATION_")) {
        if (!tgt->IsImported()) {
          IssueLocationPropertyError(tgt->GetName(), mf);
          return nullptr;
        }
        std::string configName = prop.substr(9);
        return cmValue(ImportedLocation(tgt, configName));
      }

      // Support "<CONFIG>_LOCATION".
      if (cmHasLiteralSuffix(prop, "_LOCATION") &&
          !cmHasLiteralPrefix(prop, "XCODE_ATTRIBUTE_")) {
        std::string configName(prop.c_str(), prop.size() - 9);
        if (configName != "IMPORTED") {
          if (!tgt->IsImported()) {
            IssueLocationPropertyError(tgt->GetName(), mf);
            return nullptr;
          }
          return cmValue(ImportedLocation(tgt, configName));
        }
      }
    }
    return nullptr;
  }

  template <typename Target>
  static cmValue GetSources(Target const* tgt);
};
