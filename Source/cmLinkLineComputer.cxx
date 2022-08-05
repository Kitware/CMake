/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLinkLineComputer.h"

#include <sstream>
#include <utility>
#include <vector>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmListFileCache.h"
#include "cmOutputConverter.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"

cmLinkLineComputer::cmLinkLineComputer(cmOutputConverter* outputConverter,
                                       cmStateDirectory const& stateDir)
  : StateDir(stateDir)
  , OutputConverter(outputConverter)
{
}

cmLinkLineComputer::~cmLinkLineComputer() = default;

void cmLinkLineComputer::SetUseWatcomQuote(bool useWatcomQuote)
{
  this->UseWatcomQuote = useWatcomQuote;
}

void cmLinkLineComputer::SetUseNinjaMulti(bool useNinjaMulti)
{
  this->UseNinjaMulti = useNinjaMulti;
}

void cmLinkLineComputer::SetForResponse(bool forResponse)
{
  this->ForResponse = forResponse;
}

void cmLinkLineComputer::SetRelink(bool relink)
{
  this->Relink = relink;
}

std::string cmLinkLineComputer::ConvertToLinkReference(
  std::string const& lib) const
{
  return this->OutputConverter->MaybeRelativeToCurBinDir(lib);
}

std::string cmLinkLineComputer::ComputeLinkLibs(cmComputeLinkInformation& cli)
{
  std::string linkLibs;
  std::vector<BT<std::string>> linkLibsList;
  this->ComputeLinkLibs(cli, linkLibsList);
  cli.AppendValues(linkLibs, linkLibsList);
  return linkLibs;
}

void cmLinkLineComputer::ComputeLinkLibs(
  cmComputeLinkInformation& cli, std::vector<BT<std::string>>& linkLibraries)
{
  using ItemVector = cmComputeLinkInformation::ItemVector;
  ItemVector const& items = cli.GetItems();
  for (auto const& item : items) {
    if (item.Target &&
        item.Target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }

    BT<std::string> linkLib;
    if (item.IsPath == cmComputeLinkInformation::ItemIsPath::Yes) {
      linkLib = item.GetFormattedItem(this->ConvertToOutputFormat(
        this->ConvertToLinkReference(item.Value.Value)));
    } else {
      linkLib = item.Value;
    }
    linkLib.Value += " ";

    linkLibraries.emplace_back(linkLib);
  }
}

std::string cmLinkLineComputer::ConvertToOutputFormat(std::string const& input)
{
  cmOutputConverter::OutputFormat shellFormat = cmOutputConverter::SHELL;
  if (this->ForResponse) {
    shellFormat = cmOutputConverter::RESPONSE;
  } else if (this->UseWatcomQuote) {
    shellFormat = cmOutputConverter::WATCOMQUOTE;
  } else if (this->UseNinjaMulti) {
    shellFormat = cmOutputConverter::NINJAMULTI;
  }

  return this->OutputConverter->ConvertToOutputFormat(input, shellFormat);
}

std::string cmLinkLineComputer::ConvertToOutputForExisting(
  std::string const& input)
{
  cmOutputConverter::OutputFormat shellFormat = cmOutputConverter::SHELL;
  if (this->ForResponse) {
    shellFormat = cmOutputConverter::RESPONSE;
  } else if (this->UseWatcomQuote) {
    shellFormat = cmOutputConverter::WATCOMQUOTE;
  } else if (this->UseNinjaMulti) {
    shellFormat = cmOutputConverter::NINJAMULTI;
  }

  return this->OutputConverter->ConvertToOutputForExisting(input, shellFormat);
}

std::string cmLinkLineComputer::ComputeLinkPath(
  cmComputeLinkInformation& cli, std::string const& libPathFlag,
  std::string const& libPathTerminator)
{
  std::string linkPath;
  std::vector<BT<std::string>> linkPathList;
  this->ComputeLinkPath(cli, libPathFlag, libPathTerminator, linkPathList);
  cli.AppendValues(linkPath, linkPathList);
  return linkPath;
}

void cmLinkLineComputer::ComputeLinkPath(
  cmComputeLinkInformation& cli, std::string const& libPathFlag,
  std::string const& libPathTerminator, std::vector<BT<std::string>>& linkPath)
{
  if (cli.GetLinkLanguage() == "Swift") {
    std::string linkPathNoBT;

    for (const cmComputeLinkInformation::Item& item : cli.GetItems()) {
      const cmGeneratorTarget* target = item.Target;
      if (!target) {
        continue;
      }

      if (target->GetType() == cmStateEnums::STATIC_LIBRARY ||
          target->GetType() == cmStateEnums::SHARED_LIBRARY) {
        cmStateEnums::ArtifactType type = cmStateEnums::RuntimeBinaryArtifact;
        if (target->HasImportLibrary(cli.GetConfig())) {
          type = cmStateEnums::ImportLibraryArtifact;
        }

        linkPathNoBT +=
          cmStrCat(" ", libPathFlag,
                   this->ConvertToOutputForExisting(
                     item.Target->GetDirectory(cli.GetConfig(), type)),
                   libPathTerminator, " ");
      }
    }

    if (!linkPathNoBT.empty()) {
      linkPath.emplace_back(std::move(linkPathNoBT));
    }
  }

  for (BT<std::string> libDir : cli.GetDirectoriesWithBacktraces()) {
    libDir.Value = cmStrCat(" ", libPathFlag,
                            this->ConvertToOutputForExisting(libDir.Value),
                            libPathTerminator, " ");
    linkPath.emplace_back(libDir);
  }
}

std::string cmLinkLineComputer::ComputeRPath(cmComputeLinkInformation& cli)
{
  std::string rpath;
  // Check what kind of rpath flags to use.
  if (cli.GetRuntimeSep().empty()) {
    // Each rpath entry gets its own option ("-R a -R b -R c")
    std::vector<std::string> runtimeDirs;
    cli.GetRPath(runtimeDirs, this->Relink);

    for (std::string const& rd : runtimeDirs) {
      rpath += cli.GetRuntimeFlag();
      rpath += this->ConvertToOutputFormat(rd);
      rpath += " ";
    }
  } else {
    // All rpath entries are combined ("-Wl,-rpath,a:b:c").
    std::string rpathString = cli.GetRPathString(this->Relink);

    // Store the rpath option in the stream.
    if (!rpathString.empty()) {
      rpath += cli.GetRuntimeFlag();
      rpath +=
        this->OutputConverter->EscapeForShell(rpathString, !this->ForResponse);
      rpath += " ";
    }
  }
  return rpath;
}

std::string cmLinkLineComputer::ComputeFrameworkPath(
  cmComputeLinkInformation& cli, std::string const& fwSearchFlag)
{
  std::string frameworkPath;
  if (!fwSearchFlag.empty()) {
    std::vector<std::string> const& fwDirs = cli.GetFrameworkPaths();
    for (std::string const& fd : fwDirs) {
      frameworkPath += fwSearchFlag;
      frameworkPath += this->ConvertToOutputFormat(fd);
      frameworkPath += " ";
    }
  }
  return frameworkPath;
}

std::string cmLinkLineComputer::ComputeLinkLibraries(
  cmComputeLinkInformation& cli, std::string const& stdLibString)
{
  std::string linkLibraries;
  std::vector<BT<std::string>> linkLibrariesList;
  this->ComputeLinkLibraries(cli, stdLibString, linkLibrariesList);
  cli.AppendValues(linkLibraries, linkLibrariesList);
  return linkLibraries;
}

void cmLinkLineComputer::ComputeLinkLibraries(
  cmComputeLinkInformation& cli, std::string const& stdLibString,
  std::vector<BT<std::string>>& linkLibraries)
{
  std::ostringstream rpathOut;
  rpathOut << this->ComputeRPath(cli);

  std::string rpath = rpathOut.str();
  if (!rpath.empty()) {
    linkLibraries.emplace_back(std::move(rpath));
  }

  // Write the library flags to the build rule.
  this->ComputeLinkLibs(cli, linkLibraries);

  // Add the linker runtime search path if any.
  std::ostringstream fout;
  std::string rpath_link = cli.GetRPathLinkString();
  if (!cli.GetRPathLinkFlag().empty() && !rpath_link.empty()) {
    fout << cli.GetRPathLinkFlag();
    fout << this->OutputConverter->EscapeForShell(rpath_link,
                                                  !this->ForResponse);
    fout << " ";
  }

  if (!stdLibString.empty()) {
    fout << stdLibString << " ";
  }

  std::string remainingLibs = fout.str();
  if (!remainingLibs.empty()) {
    linkLibraries.emplace_back(remainingLibs);
  }
}

std::string cmLinkLineComputer::GetLinkerLanguage(cmGeneratorTarget* target,
                                                  std::string const& config)
{
  return target->GetLinkerLanguage(config);
}
