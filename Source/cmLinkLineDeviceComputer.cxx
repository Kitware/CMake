/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLinkLineDeviceComputer.h"

#include <set>
#include <utility>

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"

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
  using ItemVector = cmComputeLinkInformation::ItemVector;
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

void cmLinkLineDeviceComputer::ComputeLinkLibraries(
  cmComputeLinkInformation& cli, std::string const& stdLibString,
  std::vector<BT<std::string>>& linkLibraries)
{
  // Generate the unique set of link items when device linking.
  // The nvcc device linker is designed so that each static library
  // with device symbols only needs to be listed once as it doesn't
  // care about link order.
  std::set<std::string> emitted;
  using ItemVector = cmComputeLinkInformation::ItemVector;
  ItemVector const& items = cli.GetItems();
  std::string config = cli.GetConfig();
  bool skipItemAfterFramework = false;
  // Note:
  // Any modification of this algorithm should be reflected also in
  // cmVisualStudio10TargetGenerator::ComputeCudaLinkOptions
  for (auto const& item : items) {
    if (skipItemAfterFramework) {
      skipItemAfterFramework = false;
      continue;
    }

    if (item.Target) {
      bool skip = false;
      switch (item.Target->GetType()) {
        case cmStateEnums::SHARED_LIBRARY:
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

    BT<std::string> linkLib;
    if (item.IsPath) {
      // nvcc understands absolute paths to libraries ending in '.a' or '.lib'.
      // These should be passed to nvlink.  Other extensions need to be left
      // out because nvlink may not understand or need them.  Even though it
      // can tolerate '.so' or '.dylib' it cannot tolerate '.so.1'.
      if (cmHasLiteralSuffix(item.Value.Value, ".a") ||
          cmHasLiteralSuffix(item.Value.Value, ".lib")) {
        linkLib.Value += this->ConvertToOutputFormat(
          this->ConvertToLinkReference(item.Value.Value));
      }
    } else if (item.Value == "-framework") {
      // This is the first part of '-framework Name' where the framework
      // name is specified as a following item.  Ignore both.
      skipItemAfterFramework = true;
      continue;
    } else if (cmLinkItemValidForDevice(item.Value.Value)) {
      linkLib.Value += item.Value.Value;
    }

    if (emitted.insert(linkLib.Value).second) {
      linkLib.Value += " ";

      const cmLinkImplementation* linkImpl =
        cli.GetTarget()->GetLinkImplementation(cli.GetConfig());

      for (const cmLinkImplItem& iter : linkImpl->Libraries) {
        if (iter.Target != nullptr &&
            iter.Target->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
          std::string libPath = iter.Target->GetLocation(cli.GetConfig());
          if (item.Value == libPath) {
            linkLib.Backtrace = iter.Backtrace;
            break;
          }
        }
      }

      linkLibraries.emplace_back(linkLib);
    }
  }

  if (!stdLibString.empty()) {
    linkLibraries.emplace_back(cmStrCat(stdLibString, ' '));
  }
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

  if (!lg.GetMakefile()->IsOn("CMAKE_CUDA_COMPILER_HAS_DEVICE_LINK_PHASE")) {
    return false;
  }

  if (const char* resolveDeviceSymbols =
        target.GetProperty("CUDA_RESOLVE_DEVICE_SYMBOLS")) {
    // If CUDA_RESOLVE_DEVICE_SYMBOLS has been explicitly set we need
    // to honor the value no matter what it is.
    return cmIsOn(resolveDeviceSymbols);
  }

  // Determine if we have any dependencies that require
  // us to do a device link step
  cmGeneratorTarget::LinkClosure const* closure =
    target.GetLinkClosure(config);

  if (cmContains(closure->Languages, "CUDA")) {
    if (const char* separableCompilation =
          target.GetProperty("CUDA_SEPARABLE_COMPILATION")) {
      if (cmIsOn(separableCompilation)) {
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
