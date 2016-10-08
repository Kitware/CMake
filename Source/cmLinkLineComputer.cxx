/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLinkLineComputer.h"
#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmOutputConverter.h"

cmLinkLineComputer::cmLinkLineComputer(cmOutputConverter* outputConverter,
                                       cmState::Directory stateDir)
  : StateDir(stateDir)
  , OutputConverter(outputConverter)
  , ForResponse(false)
  , UseWatcomQuote(false)
{
}

cmLinkLineComputer::~cmLinkLineComputer()
{
}

void cmLinkLineComputer::SetUseWatcomQuote(bool useWatcomQuote)
{
  this->UseWatcomQuote = useWatcomQuote;
}

void cmLinkLineComputer::SetForResponse(bool forResponse)
{
  this->ForResponse = forResponse;
}

std::string cmLinkLineComputer::ConvertToLinkReference(
  std::string const& lib) const
{
  std::string relLib = lib;

  if (cmOutputConverter::ContainedInDirectory(
        this->StateDir.GetCurrentBinary(), lib, this->StateDir)) {
    relLib = cmOutputConverter::ForceToRelativePath(
      this->StateDir.GetCurrentBinary(), lib);
  }
  return relLib;
}

std::string cmLinkLineComputer::ComputeLinkLibs(cmComputeLinkInformation& cli)
{
  std::string linkLibs;
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector const& items = cli.GetItems();
  for (ItemVector::const_iterator li = items.begin(); li != items.end();
       ++li) {
    if (li->Target && li->Target->GetType() == cmState::INTERFACE_LIBRARY) {
      continue;
    }
    if (li->IsPath) {
      linkLibs +=
        this->ConvertToOutputFormat(this->ConvertToLinkReference(li->Value));
    } else {
      linkLibs += li->Value;
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
