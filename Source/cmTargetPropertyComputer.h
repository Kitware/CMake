/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmTargetPropertyComputer_h
#define cmTargetPropertyComputer_h

#include <cmConfigure.h> // IWYU pragma: keep

#include "cmListFileCache.h"
#include "cmSystemTools.h"

#include <map>
#include <string>

class cmTarget;
class cmMessenger;

class cmTargetPropertyComputer
{
public:
  template <typename Target>
  static const char* GetProperty(Target const* tgt, const std::string& prop,
                                 cmMessenger* messenger,
                                 cmListFileBacktrace const& context)
  {
    if (const char* loc = GetLocation(tgt, prop, messenger, context)) {
      return loc;
    }
    if (cmSystemTools::GetFatalErrorOccured()) {
      return CM_NULLPTR;
    }
    if (prop == "SOURCES") {
      return GetSources(tgt, messenger, context);
    }
    return CM_NULLPTR;
  }

  static bool WhiteListedInterfaceProperty(const std::string& prop);

  static bool PassesWhitelist(cmState::TargetType tgtType,
                              std::string const& prop, cmMessenger* messenger,
                              cmListFileBacktrace const& context);

private:
  static bool HandleLocationPropertyPolicy(std::string const& tgtName,
                                           cmMessenger* messenger,
                                           cmListFileBacktrace const& context);

  template <typename Target>
  static const char* ComputeLocationForBuild(Target const* tgt);
  template <typename Target>
  static const char* ComputeLocation(Target const* tgt,
                                     std::string const& config);

  template <typename Target>
  static const char* GetLocation(Target const* tgt, std::string const& prop,
                                 cmMessenger* messenger,
                                 cmListFileBacktrace const& context)

  {
    // Watch for special "computed" properties that are dependent on
    // other properties or variables.  Always recompute them.
    if (tgt->GetType() == cmState::EXECUTABLE ||
        tgt->GetType() == cmState::STATIC_LIBRARY ||
        tgt->GetType() == cmState::SHARED_LIBRARY ||
        tgt->GetType() == cmState::MODULE_LIBRARY ||
        tgt->GetType() == cmState::UNKNOWN_LIBRARY) {
      static const std::string propLOCATION = "LOCATION";
      if (prop == propLOCATION) {
        if (!tgt->IsImported() &&
            !HandleLocationPropertyPolicy(tgt->GetName(), messenger,
                                          context)) {
          return CM_NULLPTR;
        }
        return ComputeLocationForBuild(tgt);
      }

      // Support "LOCATION_<CONFIG>".
      else if (cmHasLiteralPrefix(prop, "LOCATION_")) {
        if (!tgt->IsImported() &&
            !HandleLocationPropertyPolicy(tgt->GetName(), messenger,
                                          context)) {
          return CM_NULLPTR;
        }
        const char* configName = prop.c_str() + 9;
        return ComputeLocation(tgt, configName);
      }

      // Support "<CONFIG>_LOCATION".
      else if (cmHasLiteralSuffix(prop, "_LOCATION") &&
               !cmHasLiteralPrefix(prop, "XCODE_ATTRIBUTE_")) {
        std::string configName(prop.c_str(), prop.size() - 9);
        if (configName != "IMPORTED") {
          if (!tgt->IsImported() &&
              !HandleLocationPropertyPolicy(tgt->GetName(), messenger,
                                            context)) {
            return CM_NULLPTR;
          }
          return ComputeLocation(tgt, configName);
        }
      }
    }
    return CM_NULLPTR;
  }

  template <typename Target>
  static const char* GetSources(Target const* tgt, cmMessenger* messenger,
                                cmListFileBacktrace const& context);
};

#endif
