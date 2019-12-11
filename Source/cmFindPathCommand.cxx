/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFindPathCommand.h"

#include "cmsys/Glob.hxx"

#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

cmFindPathCommand::cmFindPathCommand(cmExecutionStatus& status)
  : cmFindBase(status)
{
  this->EnvironmentPath = "INCLUDE";
  this->IncludeFileInPath = false;
}

// cmFindPathCommand
bool cmFindPathCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  this->DebugMode = ComputeIfDebugModeWanted();
  this->VariableDocumentation = "Path to a file.";
  this->CMakePathName = "INCLUDE";
  if (!this->ParseArguments(argsIn)) {
    return false;
  }
  if (this->AlreadyInCache) {
    // If the user specifies the entry on the command line without a
    // type we should add the type and docstring but keep the original
    // value.
    if (this->AlreadyInCacheWithoutMetaInfo) {
      this->Makefile->AddCacheDefinition(
        this->VariableName, "", this->VariableDocumentation.c_str(),
        (this->IncludeFileInPath ? cmStateEnums::FILEPATH
                                 : cmStateEnums::PATH));
    }
    return true;
  }

  std::string result = this->FindHeader();
  if (!result.empty()) {
    this->Makefile->AddCacheDefinition(
      this->VariableName, result.c_str(), this->VariableDocumentation.c_str(),
      (this->IncludeFileInPath) ? cmStateEnums::FILEPATH : cmStateEnums::PATH);
    return true;
  }
  this->Makefile->AddCacheDefinition(
    this->VariableName, (this->VariableName + "-NOTFOUND").c_str(),
    this->VariableDocumentation.c_str(),
    (this->IncludeFileInPath) ? cmStateEnums::FILEPATH : cmStateEnums::PATH);
  return true;
}

std::string cmFindPathCommand::FindHeader()
{
  std::string debug_name = this->IncludeFileInPath ? "find_file" : "find_path";
  cmFindBaseDebugState debug(debug_name, this);
  std::string header;
  if (this->SearchFrameworkFirst || this->SearchFrameworkOnly) {
    header = this->FindFrameworkHeader(debug);
  }
  if (header.empty() && !this->SearchFrameworkOnly) {
    header = this->FindNormalHeader(debug);
  }
  if (header.empty() && this->SearchFrameworkLast) {
    header = this->FindFrameworkHeader(debug);
  }

  return header;
}

std::string cmFindPathCommand::FindHeaderInFramework(std::string const& file,
                                                     std::string const& dir)
{
  std::string fileName = file;
  std::string frameWorkName;
  std::string::size_type pos = fileName.find('/');
  // if there is a / in the name try to find the header as a framework
  // For example bar/foo.h would look for:
  // bar.framework/Headers/foo.h
  if (pos != std::string::npos) {
    // remove the name from the slash;
    fileName = fileName.substr(pos + 1);
    frameWorkName = file;
    frameWorkName =
      frameWorkName.substr(0, frameWorkName.size() - fileName.size() - 1);
    // if the framework has a path in it then just use the filename
    if (frameWorkName.find('/') != std::string::npos) {
      fileName = file;
      frameWorkName.clear();
    }
    if (!frameWorkName.empty()) {
      std::string fpath = cmStrCat(dir, frameWorkName, ".framework");
      std::string intPath = cmStrCat(fpath, "/Headers/", fileName);
      if (cmSystemTools::FileExists(intPath)) {
        if (this->IncludeFileInPath) {
          return intPath;
        }
        return fpath;
      }
    }
  }
  // if it is not found yet or not a framework header, then do a glob search
  // for all frameworks in the directory: dir/*.framework/Headers/<file>
  std::string glob = cmStrCat(dir, "*.framework/Headers/", file);
  cmsys::Glob globIt;
  globIt.FindFiles(glob);
  std::vector<std::string> files = globIt.GetFiles();
  if (!files.empty()) {
    std::string fheader = cmSystemTools::CollapseFullPath(files[0]);
    if (this->IncludeFileInPath) {
      return fheader;
    }
    fheader.resize(fheader.size() - file.size());
    return fheader;
  }
  return "";
}

std::string cmFindPathCommand::FindNormalHeader(cmFindBaseDebugState& debug)
{
  std::string tryPath;
  for (std::string const& n : this->Names) {
    for (std::string const& sp : this->SearchPaths) {
      tryPath = cmStrCat(sp, n);
      if (cmSystemTools::FileExists(tryPath)) {
        debug.FoundAt(tryPath);
        if (this->IncludeFileInPath) {
          return tryPath;
        }
        return sp;
      }
      debug.FailedAt(tryPath);
    }
  }
  return "";
}

std::string cmFindPathCommand::FindFrameworkHeader(cmFindBaseDebugState& debug)
{
  for (std::string const& n : this->Names) {
    for (std::string const& sp : this->SearchPaths) {
      std::string fwPath = this->FindHeaderInFramework(n, sp);
      fwPath.empty() ? debug.FailedAt(fwPath) : debug.FoundAt(fwPath);
      if (!fwPath.empty()) {
        return fwPath;
      }
    }
  }
  return "";
}

bool cmFindPath(std::vector<std::string> const& args,
                cmExecutionStatus& status)
{
  return cmFindPathCommand(status).InitialPass(args);
}
