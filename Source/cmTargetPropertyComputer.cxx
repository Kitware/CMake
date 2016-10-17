/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmTargetPropertyComputer.h"

#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessenger.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmTarget.h"

#if defined(CMake_HAVE_CXX_UNORDERED_SET)
#include <unordered_set>
#define UNORDERED_SET std::unordered_set
#elif defined(CMAKE_BUILD_WITH_CMAKE)
#include <cmsys/hash_set.hxx>
#define UNORDERED_SET cmsys::hash_set
#else
#define UNORDERED_SET std::set
#endif

bool cmTargetPropertyComputer::HandleLocationPropertyPolicy(
  std::string const& tgtName, cmMessenger* messenger,
  cmListFileBacktrace const& context)
{
  std::ostringstream e;
  const char* modal = CM_NULLPTR;
  cmake::MessageType messageType = cmake::AUTHOR_WARNING;
  switch (context.GetBottom().GetPolicy(cmPolicies::CMP0026)) {
    case cmPolicies::WARN:
      e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0026) << "\n";
      modal = "should";
    case cmPolicies::OLD:
      break;
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::NEW:
      modal = "may";
      messageType = cmake::FATAL_ERROR;
  }

  if (modal) {
    e << "The LOCATION property " << modal << " not be read from target \""
      << tgtName
      << "\".  Use the target name directly with "
         "add_custom_command, or use the generator expression $<TARGET_FILE>, "
         "as appropriate.\n";
    messenger->IssueMessage(messageType, e.str(), context);
  }

  return messageType != cmake::FATAL_ERROR;
}

bool cmTargetPropertyComputer::WhiteListedInterfaceProperty(
  const std::string& prop)
{
  if (cmHasLiteralPrefix(prop, "INTERFACE_")) {
    return true;
  }
  static UNORDERED_SET<std::string> builtIns;
  if (builtIns.empty()) {
    builtIns.insert("COMPATIBLE_INTERFACE_BOOL");
    builtIns.insert("COMPATIBLE_INTERFACE_NUMBER_MAX");
    builtIns.insert("COMPATIBLE_INTERFACE_NUMBER_MIN");
    builtIns.insert("COMPATIBLE_INTERFACE_STRING");
    builtIns.insert("EXPORT_NAME");
    builtIns.insert("IMPORTED");
    builtIns.insert("NAME");
    builtIns.insert("TYPE");
  }

  if (builtIns.count(prop)) {
    return true;
  }

  if (cmHasLiteralPrefix(prop, "MAP_IMPORTED_CONFIG_")) {
    return true;
  }

  return false;
}

bool cmTargetPropertyComputer::PassesWhitelist(
  cmState::TargetType tgtType, std::string const& prop, cmMessenger* messenger,
  cmListFileBacktrace const& context)
{
  if (tgtType == cmState::INTERFACE_LIBRARY &&
      !WhiteListedInterfaceProperty(prop)) {
    std::ostringstream e;
    e << "INTERFACE_LIBRARY targets may only have whitelisted properties.  "
         "The property \""
      << prop << "\" is not allowed.";
    messenger->IssueMessage(cmake::FATAL_ERROR, e.str(), context);
    return false;
  }
  return true;
}
