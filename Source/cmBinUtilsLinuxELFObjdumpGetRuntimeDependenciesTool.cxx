/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool.h"

#include <sstream>

#include <cmsys/RegularExpression.hxx>

#include "cmRuntimeDependencyArchive.h"
#include "cmSystemTools.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"

cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool::
  cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive)
  : cmBinUtilsLinuxELFGetRuntimeDependenciesTool(archive)
{
}

bool cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool::GetFileInfo(
  std::string const& file, std::vector<std::string>& needed,
  std::vector<std::string>& rpaths, std::vector<std::string>& runpaths)
{
  cmUVProcessChainBuilder builder;
  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT);

  std::vector<std::string> command;
  if (!this->Archive->GetGetRuntimeDependenciesCommand("objdump", command)) {
    this->SetError("Could not find objdump");
    return false;
  }
  command.emplace_back("-p");
  command.push_back(file);
  builder.AddCommand(command);

  auto process = builder.Start();
  if (!process.Valid() || process.GetStatus(0).SpawnResult != 0) {
    std::ostringstream e;
    e << "Failed to start objdump process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  std::string line;
  static const cmsys::RegularExpression neededRegex("^ *NEEDED *([^\n]*)$");
  static const cmsys::RegularExpression rpathRegex("^ *RPATH *([^\n]*)$");
  static const cmsys::RegularExpression runpathRegex("^ *RUNPATH *([^\n]*)$");
  cmUVPipeIStream output(process.GetLoop(), process.OutputStream());
  while (std::getline(output, line)) {
    cmsys::RegularExpressionMatch match;
    if (neededRegex.find(line.c_str(), match)) {
      needed.push_back(match.match(1));
    } else if (rpathRegex.find(line.c_str(), match)) {
      std::vector<std::string> rpathSplit =
        cmSystemTools::SplitString(match.match(1), ':');
      rpaths.reserve(rpaths.size() + rpathSplit.size());
      for (auto const& rpath : rpathSplit) {
        rpaths.push_back(rpath);
      }
    } else if (runpathRegex.find(line.c_str(), match)) {
      std::vector<std::string> runpathSplit =
        cmSystemTools::SplitString(match.match(1), ':');
      runpaths.reserve(runpaths.size() + runpathSplit.size());
      for (auto const& runpath : runpathSplit) {
        runpaths.push_back(runpath);
      }
    }
  }

  if (!process.Wait()) {
    std::ostringstream e;
    e << "Failed to wait on objdump process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }
  if (process.GetStatus(0).ExitStatus != 0) {
    std::ostringstream e;
    e << "Failed to run objdump on:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  return true;
}
