/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLinkLineDeviceComputer.h"

#include <sstream>

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmStateTypes.h"

class cmOutputConverter;

cmLinkLineDeviceComputer::cmLinkLineDeviceComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir)
  : cmLinkLineComputer(outputConverter, stateDir)
{
}

cmLinkLineDeviceComputer::~cmLinkLineDeviceComputer()
{
}

std::string cmLinkLineDeviceComputer::ComputeLinkLibraries(
  cmComputeLinkInformation& cli, std::string const& stdLibString)
{
  // Write the library flags to the build rule.
  std::ostringstream fout;
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector const& items = cli.GetItems();
  std::string config = cli.GetConfig();
  for (auto const& item : items) {
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

    if (item.IsPath) {
      // nvcc understands absolute paths to libraries ending in '.a' should
      // be passed to nvlink.  Other extensions like '.so' or '.dylib' are
      // rejected by the nvcc front-end even though nvlink knows to ignore
      // them.  Bypass the front-end via '-Xnvlink'.
      if (!cmHasLiteralSuffix(item.Value, ".a")) {
        fout << "-Xnvlink ";
      }
      fout << this->ConvertToOutputFormat(
        this->ConvertToLinkReference(item.Value));
    } else {
      fout << item.Value;
    }
    fout << " ";
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

cmNinjaLinkLineDeviceComputer::cmNinjaLinkLineDeviceComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir,
  cmGlobalNinjaGenerator const* gg)
  : cmLinkLineDeviceComputer(outputConverter, stateDir)
  , GG(gg)
{
}

std::string cmNinjaLinkLineDeviceComputer::ConvertToLinkReference(
  std::string const& lib) const
{
  return GG->ConvertToNinjaPath(lib);
}
