/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool.h"

#include <sstream>

#include <cmsys/RegularExpression.hxx>

#include "cmRuntimeDependencyArchive.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"

cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool::
  cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive)
  : cmBinUtilsMacOSMachOGetRuntimeDependenciesTool(archive)
{
}

bool cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool::GetFileInfo(
  std::string const& file, std::vector<std::string>& libs,
  std::vector<std::string>& rpaths)
{
  std::vector<std::string> command;
  if (!this->Archive->GetGetRuntimeDependenciesCommand("otool", command)) {
    this->SetError("Could not find otool");
    return false;
  }
  command.emplace_back("-l");
  command.emplace_back(file);

  cmUVProcessChainBuilder builder;
  builder.SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .AddCommand(command);

  auto process = builder.Start();
  if (!process.Valid() || process.GetStatus(0).SpawnResult != 0) {
    std::ostringstream e;
    e << "Failed to start otool process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  std::string line;
  static cmsys::RegularExpression const rpathRegex("^ *cmd LC_RPATH$");
  static cmsys::RegularExpression const loadDylibRegex(
    "^ *cmd LC_LOAD(_WEAK)?_DYLIB$");
  static cmsys::RegularExpression const pathRegex(
    "^ *path (.*) \\(offset [0-9]+\\)$");
  static cmsys::RegularExpression const nameRegex(
    "^ *name (.*) \\(offset [0-9]+\\)$");
  cmUVPipeIStream output(process.GetLoop(), process.OutputStream());
  while (std::getline(output, line)) {
    cmsys::RegularExpressionMatch cmdMatch;
    if (rpathRegex.find(line.c_str(), cmdMatch)) {
      // NOLINTNEXTLINE(misc-redundant-expression)
      if (!std::getline(output, line) || !std::getline(output, line)) {
        this->SetError("Invalid output from otool");
        return false;
      }

      cmsys::RegularExpressionMatch pathMatch;
      if (pathRegex.find(line.c_str(), pathMatch)) {
        rpaths.push_back(pathMatch.match(1));
      } else {
        this->SetError("Invalid output from otool");
        return false;
      }
    } else if (loadDylibRegex.find(line.c_str(), cmdMatch)) {
      // NOLINTNEXTLINE(misc-redundant-expression)
      if (!std::getline(output, line) || !std::getline(output, line)) {
        this->SetError("Invalid output from otool");
        return false;
      }

      cmsys::RegularExpressionMatch nameMatch;
      if (nameRegex.find(line.c_str(), nameMatch)) {
        libs.push_back(nameMatch.match(1));
      } else {
        this->SetError("Invalid output from otool");
        return false;
      }
    }
  }

  if (!process.Wait()) {
    std::ostringstream e;
    e << "Failed to wait on otool process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }
  if (process.GetStatus(0).ExitStatus != 0) {
    std::ostringstream e;
    e << "Failed to run otool on:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  return true;
}
