/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmTargetPropertyComputer.h"

#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessenger.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSystemTools.h"
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

const char* cmTargetPropertyComputer::ComputeLocationForBuild(
  cmTarget const* tgt)
{
  static std::string loc;
  if (tgt->IsImported()) {
    loc = tgt->ImportedGetFullPath("", false);
    return loc.c_str();
  }

  cmGlobalGenerator* gg = tgt->GetMakefile()->GetGlobalGenerator();
  if (!gg->GetConfigureDoneCMP0026()) {
    gg->CreateGenerationObjects();
  }
  cmGeneratorTarget* gt = gg->FindGeneratorTarget(tgt->GetName());
  loc = gt->GetLocationForBuild();
  return loc.c_str();
}

const char* cmTargetPropertyComputer::ComputeLocation(
  cmTarget const* tgt, std::string const& config)
{
  static std::string loc;
  if (tgt->IsImported()) {
    loc = tgt->ImportedGetFullPath(config, false);
    return loc.c_str();
  }

  cmGlobalGenerator* gg = tgt->GetMakefile()->GetGlobalGenerator();
  if (!gg->GetConfigureDoneCMP0026()) {
    gg->CreateGenerationObjects();
  }
  cmGeneratorTarget* gt = gg->FindGeneratorTarget(tgt->GetName());
  loc = gt->GetFullPath(config, false);
  return loc.c_str();
}

const char* cmTargetPropertyComputer::GetProperty(
  cmTarget const* tgt, const std::string& prop, cmMessenger* messenger,
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

const char* cmTargetPropertyComputer::GetLocation(
  cmTarget const* tgt, const std::string& prop, cmMessenger* messenger,
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
          !HandleLocationPropertyPolicy(tgt->GetName(), messenger, context)) {
        return CM_NULLPTR;
      }
      return ComputeLocationForBuild(tgt);
    }

    // Support "LOCATION_<CONFIG>".
    else if (cmHasLiteralPrefix(prop, "LOCATION_")) {
      if (!tgt->IsImported() &&
          !HandleLocationPropertyPolicy(tgt->GetName(), messenger, context)) {
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

const char* cmTargetPropertyComputer::GetSources(
  cmTarget const* tgt,cmMessenger* messenger,
  cmListFileBacktrace const& context)
{
  cmStringRange entries = tgt->GetSourceEntries();
  if (entries.empty()) {
    return CM_NULLPTR;
  }

  std::ostringstream ss;
  const char* sep = "";
  for (std::vector<std::string>::const_iterator i = entries.begin();
       i != entries.end(); ++i) {
    std::string const& entry = *i;

    std::vector<std::string> files;
    cmSystemTools::ExpandListArgument(entry, files);
    for (std::vector<std::string>::const_iterator li = files.begin();
         li != files.end(); ++li) {
      if (cmHasLiteralPrefix(*li, "$<TARGET_OBJECTS:") &&
          (*li)[li->size() - 1] == '>') {
        std::string objLibName = li->substr(17, li->size() - 18);

        if (cmGeneratorExpression::Find(objLibName) != std::string::npos) {
          ss << sep;
          sep = ";";
          ss << *li;
          continue;
        }

        bool addContent = false;
        bool noMessage = true;
        std::ostringstream e;
        cmake::MessageType messageType = cmake::AUTHOR_WARNING;
        switch (context.GetBottom().GetPolicy(cmPolicies::CMP0051)) {
          case cmPolicies::WARN:
            e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0051) << "\n";
            noMessage = false;
          case cmPolicies::OLD:
            break;
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::NEW:
            addContent = true;
        }
        if (!noMessage) {
          e << "Target \"" << tgt->GetName()
            << "\" contains "
               "$<TARGET_OBJECTS> generator expression in its sources "
               "list.  "
               "This content was not previously part of the SOURCES "
               "property "
               "when that property was read at configure time.  Code "
               "reading "
               "that property needs to be adapted to ignore the generator "
               "expression using the string(GENEX_STRIP) command.";
          messenger->IssueMessage(messageType, e.str(), context);
        }
        if (addContent) {
          ss << sep;
          sep = ";";
          ss << *li;
        }
      } else if (cmGeneratorExpression::Find(*li) == std::string::npos) {
        ss << sep;
        sep = ";";
        ss << *li;
      } else {
        cmSourceFile* sf = tgt->GetMakefile()->GetOrCreateSource(*li);
        // Construct what is known about this source file location.
        cmSourceFileLocation const& location = sf->GetLocation();
        std::string sname = location.GetDirectory();
        if (!sname.empty()) {
          sname += "/";
        }
        sname += location.GetName();

        ss << sep;
        sep = ";";
        // Append this list entry.
        ss << sname;
      }
    }
  }
  static std::string srcs;
  srcs = ss.str();
  return srcs.c_str();
}
