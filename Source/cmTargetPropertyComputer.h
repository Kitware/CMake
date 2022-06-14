/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

class cmMakefile;

class cmTargetPropertyComputer
{
public:
  template <typename Target>
  static cmValue GetProperty(Target const* tgt, const std::string& prop,
                             cmMakefile const& mf)
  {
    if (cmValue loc = GetLocation(tgt, prop, mf)) {
      return loc;
    }
    if (cmSystemTools::GetFatalErrorOccurred()) {
      return nullptr;
    }
    if (prop == "SOURCES") {
      return GetSources(tgt, mf);
    }
    return nullptr;
  }

private:
  static bool HandleLocationPropertyPolicy(std::string const& tgtName,
                                           cmMakefile const& mf);

  template <typename Target>
  static const std::string& ComputeLocationForBuild(Target const* tgt);
  template <typename Target>
  static const std::string& ComputeLocation(Target const* tgt,
                                            std::string const& config);

  template <typename Target>
  static cmValue GetLocation(Target const* tgt, std::string const& prop,
                             cmMakefile const& mf)

  {
    // Watch for special "computed" properties that are dependent on
    // other properties or variables.  Always recompute them.
    if (tgt->GetType() == cmStateEnums::EXECUTABLE ||
        tgt->GetType() == cmStateEnums::STATIC_LIBRARY ||
        tgt->GetType() == cmStateEnums::SHARED_LIBRARY ||
        tgt->GetType() == cmStateEnums::MODULE_LIBRARY ||
        tgt->GetType() == cmStateEnums::UNKNOWN_LIBRARY) {
      static const std::string propLOCATION = "LOCATION";
      if (prop == propLOCATION) {
        if (!tgt->IsImported() &&
            !HandleLocationPropertyPolicy(tgt->GetName(), mf)) {
          return nullptr;
        }
        return cmValue(ComputeLocationForBuild(tgt));
      }

      // Support "LOCATION_<CONFIG>".
      if (cmHasLiteralPrefix(prop, "LOCATION_")) {
        if (!tgt->IsImported() &&
            !HandleLocationPropertyPolicy(tgt->GetName(), mf)) {
          return nullptr;
        }
        std::string configName = prop.substr(9);
        return cmValue(ComputeLocation(tgt, configName));
      }

      // Support "<CONFIG>_LOCATION".
      if (cmHasLiteralSuffix(prop, "_LOCATION") &&
          !cmHasLiteralPrefix(prop, "XCODE_ATTRIBUTE_")) {
        std::string configName(prop.c_str(), prop.size() - 9);
        if (configName != "IMPORTED") {
          if (!tgt->IsImported() &&
              !HandleLocationPropertyPolicy(tgt->GetName(), mf)) {
            return nullptr;
          }
          return cmValue(ComputeLocation(tgt, configName));
        }
      }
    }
    return nullptr;
  }

  template <typename Target>
  static cmValue GetSources(Target const* tgt, cmMakefile const& mf);
};
