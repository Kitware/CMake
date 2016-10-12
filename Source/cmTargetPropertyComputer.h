/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmTargetPropertyComputer_h
#define cmTargetPropertyComputer_h

#include <cmConfigure.h> // IWYU pragma: keep

#include "cmListFileCache.h"

#include <map>
#include <string>

class cmTarget;
class cmMessenger;

class cmTargetPropertyComputer
{
public:
  static const char* GetProperty(cmTarget const* tgt, const std::string& prop,
                                 cmMessenger* messenger,
                                 cmListFileBacktrace const& context);

  static std::map<std::string, std::string> ComputeFileLocations(
    cmTarget const* tgt);

  static bool WhiteListedInterfaceProperty(const std::string& prop);

private:
  static bool HandleLocationPropertyPolicy(std::string const& tgtName,
                                           cmMessenger* messenger,
                                           cmListFileBacktrace const& context);

  static const char* ComputeLocationForBuild(cmTarget const* tgt);
  static const char* ComputeLocation(cmTarget const* tgt,
                                     std::string const& config);
  static const char* GetLocation(cmTarget const* tgt, std::string const& prop,
                                 cmMessenger* messenger,
                                 cmListFileBacktrace const& context);

  static const char* GetSources(cmTarget const* tgt, cmMessenger* messenger,
                                cmListFileBacktrace const& context);
};

#endif
