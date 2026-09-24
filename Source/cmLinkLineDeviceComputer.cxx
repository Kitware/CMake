/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmLinkLineDeviceComputer.h"

#include <algorithm>
#include <initializer_list>
#include <set>
#include <utility>
#include <vector>

#include <cmext/algorithm>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmTargetTypes.h"
#include "cmValue.h"

class cmOutputConverter;

cmLinkLineDeviceComputer::cmLinkLineDeviceComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir,
  std::string language)
  : cmLinkLineComputer(outputConverter, stateDir)
  , Language(std::move(language))
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
  return (!cmHasPrefix(item, '-') ||        //
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
  return std::any_of(
    items.begin(), items.end(),
    [this](cmComputeLinkInformation::Item const& item) -> bool {
      return item.Target &&
        item.Target->GetType() == cm::TargetType::STATIC_LIBRARY &&
        // this dependency requires us to device link it
        !item.Target->GetPropertyAsBool(
          cmStrCat(this->Language, "_RESOLVE_DEVICE_SYMBOLS")) &&
        item.Target->GetPropertyAsBool(
          cmStrCat(this->Language, "_SEPARABLE_COMPILATION"));
    });
}

bool cmLinkLineDeviceComputer::ComputeRequiresDeviceLinkingIPOFlag(
  cmComputeLinkInformation& cli)
{
  // Determine if this item might requires device linking.
  // For this we only consider targets
  using ItemVector = cmComputeLinkInformation::ItemVector;
  ItemVector const& items = cli.GetItems();
  std::string config = cli.GetConfig();
  return std::any_of(
    items.begin(), items.end(),
    [this, config](cmComputeLinkInformation::Item const& item) -> bool {
      return item.Target &&
        item.Target->GetType() == cm::TargetType::STATIC_LIBRARY &&
        // this dependency requires us to device link it
        !item.Target->GetPropertyAsBool(
          cmStrCat(this->Language, "_RESOLVE_DEVICE_SYMBOLS")) &&
        item.Target->GetPropertyAsBool(
          cmStrCat(this->Language, "_SEPARABLE_COMPILATION")) &&
        item.Target->IsIPOEnabled(this->Language, config);
    });
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

  for (auto const& item : items) {
    if (skipItemAfterFramework) {
      skipItemAfterFramework = false;
      continue;
    }

    if (item.Target) {
      bool skip = false;
      switch (item.Target->GetType()) {
        case cm::TargetType::SHARED_LIBRARY:
        case cm::TargetType::MODULE_LIBRARY:
        case cm::TargetType::OBJECT_LIBRARY:
        case cm::TargetType::INTERFACE_LIBRARY:
          skip = true;
          break;
        case cm::TargetType::STATIC_LIBRARY:
          skip = item.Target->GetPropertyAsBool(
            cmStrCat(this->Language, "_RESOLVE_DEVICE_SYMBOLS"));
          break;
        default:
          break;
      }
      if (skip) {
        continue;
      }
    }

    BT<std::string> linkLib;
    if (item.IsPath == cmComputeLinkInformation::ItemIsPath::Yes) {
      // nvcc understands absolute paths to libraries ending in '.o', .a', or
      // '.lib'. These should be passed to nvlink.  Other extensions need to be
      // left out because nvlink may not understand or need them.  Even though
      // it can tolerate '.so' or '.dylib' it cannot tolerate '.so.1'.
      if (cmHasLiteralSuffix(item.Value.Value, ".o") ||
          cmHasLiteralSuffix(item.Value.Value, ".obj") ||
          cmHasLiteralSuffix(item.Value.Value, ".a") ||
          cmHasLiteralSuffix(item.Value.Value, ".lib")) {
        linkLib.Value = item
                          .GetFormattedItem(this->ConvertToOutputFormat(
                            this->ConvertToLinkReference(item.Value.Value)))
                          .Value;
      }
    } else if (item.Value == "-framework") {
      // This is the first part of '-framework Name' where the framework
      // name is specified as a following item.  Ignore both.
      skipItemAfterFramework = true;
      continue;
    } else if (cmLinkItemValidForDevice(item.Value.Value)) {
      linkLib.Value = item.Value.Value;
    }

    if (emitted.insert(linkLib.Value).second) {
      linkLib.Value += " ";

      cmLinkImplementation const* linkImpl =
        cli.GetTarget()->GetLinkImplementation(cli.GetConfig(),
                                               cmGeneratorTarget::UseTo::Link);

      for (cmLinkItem const& iter : linkImpl->Libraries) {
        if (iter.Target &&
            iter.Target->GetType() != cm::TargetType::INTERFACE_LIBRARY) {
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
  return this->Language;
}

bool requireDeviceLinking(cmGeneratorTarget const& target,
                          cmLocalGenerator& lg, std::string const& config,
                          std::string const& language)
{
  if (!target.GetGlobalGenerator()->GetLanguageEnabled(language)) {
    return false;
  }

  if (target.GetType() == cm::TargetType::OBJECT_LIBRARY) {
    return false;
  }

  if (!lg.GetMakefile()->IsOn(
        cmStrCat("CMAKE_", language, "_COMPILER_HAS_DEVICE_LINK_PHASE"))) {
    return false;
  }

  if (cmValue resolveDeviceSymbols =
        target.GetProperty(cmStrCat(language, "_RESOLVE_DEVICE_SYMBOLS"))) {
    // If CUDA_RESOLVE_DEVICE_SYMBOLS has been explicitly set we need
    // to honor the value no matter what it is.
    return resolveDeviceSymbols.IsOn();
  }

  // Determine if we have any dependencies that require
  // us to do a device link step
  cmGeneratorTarget::LinkClosure const* closure =
    target.GetLinkClosure(config);

  if (cm::contains(closure->Languages, language)) {
    if (target.GetProperty(cmStrCat(language, "_SEPARABLE_COMPILATION"))
          .IsOn()) {
      bool doDeviceLinking = false;
      switch (target.GetType()) {
        case cm::TargetType::SHARED_LIBRARY:
        case cm::TargetType::MODULE_LIBRARY:
        case cm::TargetType::EXECUTABLE:
          doDeviceLinking = true;
          break;
        default:
          break;
      }
      return doDeviceLinking;
    }

    cmComputeLinkInformation* pcli = target.GetLinkInformation(config);
    if (pcli) {
      cmLinkLineDeviceComputer deviceLinkComputer(
        &lg, lg.GetStateSnapshot().GetDirectory(), language);
      return deviceLinkComputer.ComputeRequiresDeviceLinking(*pcli);
    }
    return true;
  }
  return false;
}

std::string deviceLinkLanguage(cmGeneratorTarget& target, cmLocalGenerator& lg,
                               std::string const& config)
{
  for (auto const* language : { "CUDA", "SYCL" }) {
    if (requireDeviceLinking(target, lg, config, language)) {
      return language;
    }
  }
  return {};
}
