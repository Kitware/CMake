/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLDConfigLDConfigTool.h"

#include <istream>
#include <string>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmList.h"
#include "cmMakefile.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmSystemTools.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"

cmLDConfigLDConfigTool::cmLDConfigLDConfigTool(
  cmRuntimeDependencyArchive* archive)
  : cmLDConfigTool(archive)
{
}

bool cmLDConfigLDConfigTool::GetLDConfigPaths(std::vector<std::string>& paths)
{
  std::string ldConfigPath =
    this->Archive->GetMakefile()->GetSafeDefinition("CMAKE_LDCONFIG_COMMAND");
  if (ldConfigPath.empty()) {
    ldConfigPath = cmSystemTools::FindProgram(
      "ldconfig", { "/sbin", "/usr/sbin", "/usr/local/sbin" });
    if (ldConfigPath.empty()) {
      this->Archive->SetError("Could not find ldconfig");
      return false;
    }
  }

  cmList ldConfigCommand{ ldConfigPath };
  ldConfigCommand.emplace_back("-v");
  ldConfigCommand.emplace_back("-N"); // Don't rebuild the cache.
  ldConfigCommand.emplace_back("-X"); // Don't update links.

  cmUVProcessChainBuilder builder;
  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .AddCommand(ldConfigCommand);
  auto process = builder.Start();
  if (!process.Valid() || process.GetStatus(0).SpawnResult != 0) {
    this->Archive->SetError("Failed to start ldconfig process");
    return false;
  }

  std::string line;
  static const cmsys::RegularExpression regex("^([^\t:]*):");
  cmUVPipeIStream output(process.GetLoop(), process.OutputStream());
  while (std::getline(output, line)) {
    cmsys::RegularExpressionMatch match;
    if (regex.find(line.c_str(), match)) {
      paths.push_back(match.match(1));
    }
  }

  if (!process.Wait()) {
    this->Archive->SetError("Failed to wait on ldconfig process");
    return false;
  }
  if (process.GetStatus(0).ExitStatus != 0) {
    this->Archive->SetError("Failed to run ldconfig");
    return false;
  }

  return true;
}
