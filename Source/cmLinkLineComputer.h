/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmLinkLineComputer_h
#define cmLinkLineComputer_h

#include "cmState.h"

class cmComputeLinkInformation;
class cmOutputConverter;

class cmLinkLineComputer
{
public:
  cmLinkLineComputer(cmOutputConverter* outputConverter,
                     cmState::Directory stateDir);
  virtual ~cmLinkLineComputer();

  void SetUseWatcomQuote(bool useWatcomQuote);
  void SetForResponse(bool forResponse);
  void SetRelink(bool relink);

  virtual std::string ConvertToLinkReference(std::string const& input) const;

  std::string ComputeLinkLibs(cmComputeLinkInformation& cli);

  std::string ComputeLinkPath(cmComputeLinkInformation& cli,
                              std::string const& libPathFlag,
                              std::string const& libPathTerminator);

  std::string ComputeRPath(cmComputeLinkInformation& cli);

private:
  std::string ConvertToOutputFormat(std::string const& input);
  std::string ConvertToOutputForExisting(std::string const& input);

  cmState::Directory StateDir;
  cmOutputConverter* OutputConverter;

  bool ForResponse;
  bool UseWatcomQuote;
  bool Relink;
};

#endif
