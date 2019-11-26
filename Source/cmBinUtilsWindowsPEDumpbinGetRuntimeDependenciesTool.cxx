/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool.h"

#include <sstream>

#include <cmsys/RegularExpression.hxx>

#include "cmRuntimeDependencyArchive.h"
#include "cmUVProcessChain.h"

cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool::
  cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive)
  : cmBinUtilsWindowsPEGetRuntimeDependenciesTool(archive)
{
}

bool cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool::GetFileInfo(
  const std::string& file, std::vector<std::string>& needed)
{
  cmUVProcessChainBuilder builder;
  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT);

  std::vector<std::string> command;
  if (!this->Archive->GetGetRuntimeDependenciesCommand("dumpbin", command)) {
    this->SetError("Could not find dumpbin");
    return false;
  }
  command.emplace_back("/dependents");
  command.push_back(file);
  builder.AddCommand(command);

  auto process = builder.Start();
  if (!process.Valid()) {
    std::ostringstream e;
    e << "Failed to start dumpbin process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  std::string line;
  static const cmsys::RegularExpression regex(
    "^    ([^\n]*\\.[Dd][Ll][Ll])\r$");
  while (std::getline(*process.OutputStream(), line)) {
    cmsys::RegularExpressionMatch match;
    if (regex.find(line.c_str(), match)) {
      needed.push_back(match.match(1));
    }
  }

  if (!process.Wait()) {
    std::ostringstream e;
    e << "Failed to wait on dumpbin process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }
  auto status = process.GetStatus();
  if (!status[0] || status[0]->ExitStatus != 0) {
    std::ostringstream e;
    e << "Failed to run dumpbin on:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  return true;
}
