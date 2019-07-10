/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLinkLineDeviceComputer.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

class cmOutputConverter;

cmLinkLineDeviceComputer::cmLinkLineDeviceComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir)
  : cmLinkLineComputer(outputConverter, stateDir)
{
}

cmLinkLineDeviceComputer::~cmLinkLineDeviceComputer() = default;

static bool cmLinkItemValidForDevice(std::string const& item)
{
  // Valid items are:
  // * Non-flags (does not start in '-')
  // * Specific flags --library, --library-path, -l, -L
  // For example:
  // * 'cublas_device' => pass-along
  // * '--library pthread' => pass-along
  // * '-lpthread' => pass-along
  // * '-pthread' => drop
  // * '-a' => drop
  // * '-framework Name' (as one string) => drop
  return (!cmHasLiteralPrefix(item, "-") || //
          cmHasLiteralPrefix(item, "-l") || //
          cmHasLiteralPrefix(item, "-L") || //
          cmHasLiteralPrefix(item, "--library"));
}

bool cmLinkLineDeviceComputer::ComputeRequiresDeviceLinking(
  cmComputeLinkInformation& cli)
{
  // Determine if this item might requires device linking.
  // For this we only consider targets
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector const& items = cli.GetItems();
  std::string config = cli.GetConfig();
  for (auto const& item : items) {
    if (item.Target &&
        item.Target->GetType() == cmStateEnums::STATIC_LIBRARY) {
      if ((!item.Target->GetPropertyAsBool("CUDA_RESOLVE_DEVICE_SYMBOLS")) &&
          item.Target->GetPropertyAsBool("CUDA_SEPARABLE_COMPILATION")) {
        // this dependency requires us to device link it
        return true;
      }
    }
  }
  return false;
}

std::string cmLinkLineDeviceComputer::ComputeLinkLibraries(
  cmComputeLinkInformation& cli, std::string const& stdLibString)
{
  // Write the library flags to the build rule.
  std::ostringstream fout;

  // Generate the unique set of link items when device linking.
  // The nvcc device linker is designed so that each static library
  // with device symbols only needs to be listed once as it doesn't
  // care about link order.
  std::set<std::string> emitted;
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector const& items = cli.GetItems();
  std::string config = cli.GetConfig();
  bool skipItemAfterFramework = false;
  for (auto const& item : items) {
    if (skipItemAfterFramework) {
      skipItemAfterFramework = false;
      continue;
    }

    if (item.Target) {
      bool skip = false;
      switch (item.Target->GetType()) {
        case cmStateEnums::MODULE_LIBRARY:
        case cmStateEnums::INTERFACE_LIBRARY:
          skip = true;
          break;
        case cmStateEnums::STATIC_LIBRARY:
          skip = item.Target->GetPropertyAsBool("CUDA_RESOLVE_DEVICE_SYMBOLS");
          break;
        default:
          break;
      }
      if (skip) {
        continue;
      }
    }

    std::string out;
    if (item.IsPath) {
      // nvcc understands absolute paths to libraries ending in '.a' or '.lib'.
      // These should be passed to nvlink.  Other extensions need to be left
      // out because nvlink may not understand or need them.  Even though it
      // can tolerate '.so' or '.dylib' it cannot tolerate '.so.1'.
      if (cmHasLiteralSuffix(item.Value, ".a") ||
          cmHasLiteralSuffix(item.Value, ".lib")) {
        out += this->ConvertToOutputFormat(
          this->ConvertToLinkReference(item.Value));
      }
    } else if (item.Value == "-framework") {
      // This is the first part of '-framework Name' where the framework
      // name is specified as a following item.  Ignore both.
      skipItemAfterFramework = true;
      continue;
    } else if (cmLinkItemValidForDevice(item.Value)) {
      out += item.Value;
    }

    if (emitted.insert(out).second) {
      fout << out << " ";
    }
  }

  if (!stdLibString.empty()) {
    fout << stdLibString << " ";
  }

  return fout.str();
}

std::string cmLinkLineDeviceComputer::GetLinkerLanguage(cmGeneratorTarget*,
                                                        std::string const&)
{
  return "CUDA";
}

bool requireDeviceLinking(cmGeneratorTarget& target, cmLocalGenerator& lg,
                          const std::string& config)
{
  if (!target.GetGlobalGenerator()->GetLanguageEnabled("CUDA")) {
    return false;
  }

  if (target.GetType() == cmStateEnums::OBJECT_LIBRARY) {
    return false;
  }

  if (const char* resolveDeviceSymbols =
        target.GetProperty("CUDA_RESOLVE_DEVICE_SYMBOLS")) {
    // If CUDA_RESOLVE_DEVICE_SYMBOLS has been explicitly set we need
    // to honor the value no matter what it is.
    return cmSystemTools::IsOn(resolveDeviceSymbols);
  }

  if (const char* separableCompilation =
        target.GetProperty("CUDA_SEPARABLE_COMPILATION")) {
    if (cmSystemTools::IsOn(separableCompilation)) {
      bool doDeviceLinking = false;
      switch (target.GetType()) {
        case cmStateEnums::SHARED_LIBRARY:
        case cmStateEnums::MODULE_LIBRARY:
        case cmStateEnums::EXECUTABLE:
          doDeviceLinking = true;
          break;
        default:
          break;
      }
      return doDeviceLinking;
    }
  }

  // Determine if we have any dependencies that require
  // us to do a device link step
  const std::string cuda_lang("CUDA");
  cmGeneratorTarget::LinkClosure const* closure =
    target.GetLinkClosure(config);

  bool closureHasCUDA =
    (std::find(closure->Languages.begin(), closure->Languages.end(),
               cuda_lang) != closure->Languages.end());
  if (closureHasCUDA) {
    cmComputeLinkInformation* pcli = target.GetLinkInformation(config);
    if (pcli) {
      cmLinkLineDeviceComputer deviceLinkComputer(
        &lg, lg.GetStateSnapshot().GetDirectory());
      return deviceLinkComputer.ComputeRequiresDeviceLinking(*pcli);
    }
    return true;
  }
  return false;
}
