/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool.h"

#include <sstream>

#include <cmsys/RegularExpression.hxx>

#include "cmRuntimeDependencyArchive.h"
#include "cmSystemTools.h"
#include "cmUVProcessChain.h"

cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool::
  cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive)
  : cmBinUtilsWindowsPEGetRuntimeDependenciesTool(archive)
{
}

bool cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool::GetFileInfo(
  const std::string& file, std::vector<std::string>& needed)
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
  if (!process.Valid()) {
    std::ostringstream e;
    e << "Failed to start objdump process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  std::string line;
  static const cmsys::RegularExpression regex(
    "^\t*DLL Name: ([^\n]*\\.[Dd][Ll][Ll])$");
  while (cmSystemTools::GetLineFromStream(*process.OutputStream(), line)) {
    cmsys::RegularExpressionMatch match;
    if (regex.find(line.c_str(), match)) {
      needed.push_back(match.match(1));
    }
  }

  if (!process.Wait()) {
    std::ostringstream e;
    e << "Failed to wait on objdump process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }
  auto status = process.GetStatus();
  if (!status[0] || status[0]->ExitStatus != 0) {
    std::ostringstream e;
    e << "Failed to run objdump on:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  return true;
}
