/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmStateDirectory.h"

class cmComputeLinkInformation;
class cmGeneratorTarget;
class cmOutputConverter;
template <typename T>
class BT;

class cmLinkLineComputer
{
public:
  cmLinkLineComputer(cmOutputConverter* outputConverter,
                     cmStateDirectory const& stateDir);
  virtual ~cmLinkLineComputer();

  cmLinkLineComputer(cmLinkLineComputer const&) = delete;
  cmLinkLineComputer& operator=(cmLinkLineComputer const&) = delete;

  void SetUseWatcomQuote(bool useWatcomQuote);
  void SetUseNinjaMulti(bool useNinjaMulti);
  void SetForResponse(bool forResponse);
  void SetRelink(bool relink);

  virtual std::string ConvertToLinkReference(std::string const& input) const;

  std::string ComputeLinkPath(cmComputeLinkInformation& cli,
                              std::string const& libPathFlag,
                              std::string const& libPathTerminator);

  void ComputeLinkPath(cmComputeLinkInformation& cli,
                       std::string const& libPathFlag,
                       std::string const& libPathTerminator,
                       std::vector<BT<std::string>>& linkPath);

  std::string ComputeFrameworkPath(cmComputeLinkInformation& cli,
                                   std::string const& fwSearchFlag);

  std::string ComputeLinkLibraries(cmComputeLinkInformation& cli,
                                   std::string const& stdLibString);

  virtual void ComputeLinkLibraries(
    cmComputeLinkInformation& cli, std::string const& stdLibString,
    std::vector<BT<std::string>>& linkLibraries);

  virtual std::string GetLinkerLanguage(cmGeneratorTarget* target,
                                        std::string const& config);

protected:
  std::string ComputeLinkLibs(cmComputeLinkInformation& cli);
  void ComputeLinkLibs(cmComputeLinkInformation& cli,
                       std::vector<BT<std::string>>& linkLibraries);
  std::string ComputeRPath(cmComputeLinkInformation& cli);

  std::string ConvertToOutputFormat(std::string const& input);
  std::string ConvertToOutputForExisting(std::string const& input);

  cmStateDirectory StateDir;
  cmOutputConverter* OutputConverter;

  bool ForResponse = false;
  bool UseWatcomQuote = false;
  bool UseNinjaMulti = false;
  bool Relink = false;
};
