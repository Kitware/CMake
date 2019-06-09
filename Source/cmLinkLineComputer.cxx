/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLinkLineComputer.h"

#include <sstream>
#include <vector>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmOutputConverter.h"
#include "cmStateDirectory.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

cmLinkLineComputer::cmLinkLineComputer(cmOutputConverter* outputConverter,
                                       cmStateDirectory const& stateDir)
  : StateDir(stateDir)
  , OutputConverter(outputConverter)
  , ForResponse(false)
  , UseWatcomQuote(false)
  , Relink(false)
{
}

cmLinkLineComputer::~cmLinkLineComputer() = default;

void cmLinkLineComputer::SetUseWatcomQuote(bool useWatcomQuote)
{
  this->UseWatcomQuote = useWatcomQuote;
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
  std::string relLib = lib;

  if (this->StateDir.ContainsBoth(this->StateDir.GetCurrentBinary(), lib)) {
    relLib = cmSystemTools::ForceToRelativePath(
      this->StateDir.GetCurrentBinary(), lib);
  }
  return relLib;
}

std::string cmLinkLineComputer::ComputeLinkLibs(cmComputeLinkInformation& cli)
{
  std::string linkLibs;
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector const& items = cli.GetItems();
  for (auto const& item : items) {
    if (item.Target &&
        item.Target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }
    if (item.IsPath) {
      linkLibs +=
        this->ConvertToOutputFormat(this->ConvertToLinkReference(item.Value));
    } else {
      linkLibs += item.Value;
    }
    linkLibs += " ";
  }
  return linkLibs;
}

std::string cmLinkLineComputer::ConvertToOutputFormat(std::string const& input)
{
  cmOutputConverter::OutputFormat shellFormat = (this->ForResponse)
    ? cmOutputConverter::RESPONSE
    : ((this->UseWatcomQuote) ? cmOutputConverter::WATCOMQUOTE
                              : cmOutputConverter::SHELL);

  return this->OutputConverter->ConvertToOutputFormat(input, shellFormat);
}

std::string cmLinkLineComputer::ConvertToOutputForExisting(
  std::string const& input)
{
  cmOutputConverter::OutputFormat shellFormat = (this->ForResponse)
    ? cmOutputConverter::RESPONSE
    : ((this->UseWatcomQuote) ? cmOutputConverter::WATCOMQUOTE
                              : cmOutputConverter::SHELL);

  return this->OutputConverter->ConvertToOutputForExisting(input, shellFormat);
}

std::string cmLinkLineComputer::ComputeLinkPath(
  cmComputeLinkInformation& cli, std::string const& libPathFlag,
  std::string const& libPathTerminator)
{
  std::string linkPath;

  if (cli.GetLinkLanguage() == "Swift") {
    for (const cmComputeLinkInformation::Item& item : cli.GetItems()) {
      const cmGeneratorTarget* target = item.Target;
      if (!target) {
        continue;
      }

      if (target->GetType() == cmStateEnums::STATIC_LIBRARY ||
          target->GetType() == cmStateEnums::SHARED_LIBRARY) {
        cmStateEnums::ArtifactType type = cmStateEnums::RuntimeBinaryArtifact;
        if (target->GetType() == cmStateEnums::SHARED_LIBRARY &&
            target->IsDLLPlatform()) {
          type = cmStateEnums::ImportLibraryArtifact;
        }

        linkPath += " " + libPathFlag +
          item.Target->GetDirectory(cli.GetConfig(), type) +
          libPathTerminator + " ";
      }
    }
  }

  for (std::string const& libDir : cli.GetDirectories()) {
    linkPath += " " + libPathFlag + this->ConvertToOutputForExisting(libDir) +
      libPathTerminator + " ";
  }

  return linkPath;
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
  std::ostringstream fout;
  fout << this->ComputeRPath(cli);

  // Write the library flags to the build rule.
  fout << this->ComputeLinkLibs(cli);

  // Add the linker runtime search path if any.
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

  return fout.str();
}

std::string cmLinkLineComputer::GetLinkerLanguage(cmGeneratorTarget* target,
                                                  std::string const& config)
{
  return target->GetLinkerLanguage(config);
}
