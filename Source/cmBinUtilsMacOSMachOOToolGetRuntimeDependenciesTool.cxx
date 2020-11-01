/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool.h"

#include <sstream>

#include <cmsys/RegularExpression.hxx>

#include "cmRuntimeDependencyArchive.h"
#include "cmUVProcessChain.h"

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
  if (!process.Valid()) {
    std::ostringstream e;
    e << "Failed to start otool process for:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  std::string line;
  static const cmsys::RegularExpression rpathRegex("^ *cmd LC_RPATH$");
  static const cmsys::RegularExpression loadDylibRegex(
    "^ *cmd LC_LOAD_DYLIB$");
  static const cmsys::RegularExpression pathRegex(
    "^ *path (.*) \\(offset [0-9]+\\)$");
  static const cmsys::RegularExpression nameRegex(
    "^ *name (.*) \\(offset [0-9]+\\)$");
  while (std::getline(*process.OutputStream(), line)) {
    cmsys::RegularExpressionMatch cmdMatch;
    if (rpathRegex.find(line.c_str(), cmdMatch)) {
      if (!std::getline(*process.OutputStream(), line) ||
          !std::getline(*process.OutputStream(), line)) {
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
      if (!std::getline(*process.OutputStream(), line) ||
          !std::getline(*process.OutputStream(), line)) {
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
  auto status = process.GetStatus();
  if (!status[0] || status[0]->ExitStatus != 0) {
    std::ostringstream e;
    e << "Failed to run otool on:\n  " << file;
    this->SetError(e.str());
    return false;
  }

  return true;
}
