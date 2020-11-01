/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmListFileCache.h"
#include "cmProperty.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmMessenger;

class cmTargetPropertyComputer
{
public:
  template <typename Target>
  static cmProp GetProperty(Target const* tgt, const std::string& prop,
                            cmMessenger* messenger,
                            cmListFileBacktrace const& context)
  {
    if (cmProp loc = GetLocation(tgt, prop, messenger, context)) {
      return loc;
    }
    if (cmSystemTools::GetFatalErrorOccured()) {
      return nullptr;
    }
    if (prop == "SOURCES") {
      return GetSources(tgt, messenger, context);
    }
    return nullptr;
  }

private:
  static bool HandleLocationPropertyPolicy(std::string const& tgtName,
                                           cmMessenger* messenger,
                                           cmListFileBacktrace const& context);

  template <typename Target>
  static const std::string& ComputeLocationForBuild(Target const* tgt);
  template <typename Target>
  static const std::string& ComputeLocation(Target const* tgt,
                                            std::string const& config);

  template <typename Target>
  static cmProp GetLocation(Target const* tgt, std::string const& prop,
                            cmMessenger* messenger,
                            cmListFileBacktrace const& context)

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
            !HandleLocationPropertyPolicy(tgt->GetName(), messenger,
                                          context)) {
          return nullptr;
        }
        return &ComputeLocationForBuild(tgt);
      }

      // Support "LOCATION_<CONFIG>".
      if (cmHasLiteralPrefix(prop, "LOCATION_")) {
        if (!tgt->IsImported() &&
            !HandleLocationPropertyPolicy(tgt->GetName(), messenger,
                                          context)) {
          return nullptr;
        }
        std::string configName = prop.substr(9);
        return &ComputeLocation(tgt, configName);
      }

      // Support "<CONFIG>_LOCATION".
      if (cmHasLiteralSuffix(prop, "_LOCATION") &&
          !cmHasLiteralPrefix(prop, "XCODE_ATTRIBUTE_")) {
        std::string configName(prop.c_str(), prop.size() - 9);
        if (configName != "IMPORTED") {
          if (!tgt->IsImported() &&
              !HandleLocationPropertyPolicy(tgt->GetName(), messenger,
                                            context)) {
            return nullptr;
          }
          return &ComputeLocation(tgt, configName);
        }
      }
    }
    return nullptr;
  }

  template <typename Target>
  static cmProp GetSources(Target const* tgt, cmMessenger* messenger,
                           cmListFileBacktrace const& context);
};
