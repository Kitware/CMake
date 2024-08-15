/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsLinuxELFLinker.h"

#include <queue>
#include <sstream>
#include <unordered_set>
#include <utility>

#include <cm/memory>
#include <cm/string_view>

#include <cmsys/RegularExpression.hxx>

#include "cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool.h"
#include "cmELF.h"
#include "cmLDConfigLDConfigTool.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static std::string ReplaceOrigin(const std::string& rpath,
                                 const std::string& origin)
{
  static const cmsys::RegularExpression originRegex(
    "(\\$ORIGIN)([^a-zA-Z0-9_]|$)");
  static const cmsys::RegularExpression originCurlyRegex("\\${ORIGIN}");

  cmsys::RegularExpressionMatch match;
  if (originRegex.find(rpath.c_str(), match)) {
    cm::string_view pathv(rpath);
    auto begin = pathv.substr(0, match.start(1));
    auto end = pathv.substr(match.end(1));
    return cmStrCat(begin, origin, end);
  }
  if (originCurlyRegex.find(rpath.c_str(), match)) {
    cm::string_view pathv(rpath);
    auto begin = pathv.substr(0, match.start());
    auto end = pathv.substr(match.end());
    return cmStrCat(begin, origin, end);
  }
  return rpath;
}

cmBinUtilsLinuxELFLinker::cmBinUtilsLinuxELFLinker(
  cmRuntimeDependencyArchive* archive)
  : cmBinUtilsLinker(archive)
{
}

bool cmBinUtilsLinuxELFLinker::Prepare()
{
  std::string tool = this->Archive->GetGetRuntimeDependenciesTool();
  if (tool.empty()) {
    tool = "objdump";
  }
  if (tool == "objdump") {
    this->Tool =
      cm::make_unique<cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool>(
        this->Archive);
  } else {
    std::ostringstream e;
    e << "Invalid value for CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL: " << tool;
    this->SetError(e.str());
    return false;
  }

  std::string ldConfigTool =
    this->Archive->GetMakefile()->GetSafeDefinition("CMAKE_LDCONFIG_TOOL");
  if (ldConfigTool.empty()) {
    ldConfigTool = "ldconfig";
  }
  if (ldConfigTool == "ldconfig") {
    this->LDConfigTool =
      cm::make_unique<cmLDConfigLDConfigTool>(this->Archive);
    if (!this->LDConfigTool->GetLDConfigPaths(this->LDConfigPaths)) {
      return false;
    }
  } else {
    std::ostringstream e;
    e << "Invalid value for CMAKE_LDCONFIG_TOOL: " << ldConfigTool;
    this->SetError(e.str());
    return false;
  }

  return true;
}

bool cmBinUtilsLinuxELFLinker::ScanDependencies(
  std::string const& file, cmStateEnums::TargetType /* unused */)
{
  cmELF elf(file.c_str());
  if (!elf) {
    return false;
  }
  if (elf.GetMachine() != 0) {
    if (this->Machine != 0) {
      if (elf.GetMachine() != this->Machine) {
        this->SetError("All files must have the same architecture.");
        return false;
      }
    } else {
      this->Machine = elf.GetMachine();
    }
  }

  return this->ScanDependencies(file);
}

bool cmBinUtilsLinuxELFLinker::ScanDependencies(std::string const& mainFile)
{
  std::unordered_set<std::string> resolvedDependencies;
  std::queue<std::pair<std::string, std::vector<std::string>>> queueToResolve;
  queueToResolve.push(std::make_pair(mainFile, std::vector<std::string>{}));

  while (!queueToResolve.empty()) {
    std::string file = std::move(queueToResolve.front().first);
    std::vector<std::string> parentRpaths =
      std::move(queueToResolve.front().second);
    queueToResolve.pop();

    std::string origin = cmSystemTools::GetFilenamePath(file);
    std::vector<std::string> needed;
    std::vector<std::string> rpaths;
    std::vector<std::string> runpaths;
    if (!this->Tool->GetFileInfo(file, needed, rpaths, runpaths)) {
      return false;
    }
    for (auto& runpath : runpaths) {
      runpath = ReplaceOrigin(runpath, origin);
    }
    for (auto& rpath : rpaths) {
      rpath = ReplaceOrigin(rpath, origin);
    }

    std::vector<std::string> searchPaths;
    if (!runpaths.empty()) {
      searchPaths = runpaths;
    } else {
      searchPaths = rpaths;
      searchPaths.insert(searchPaths.end(), parentRpaths.begin(),
                         parentRpaths.end());
    }

    searchPaths.insert(searchPaths.end(), this->LDConfigPaths.begin(),
                       this->LDConfigPaths.end());

    for (auto const& dep : needed) {
      if (resolvedDependencies.count(dep) != 0 ||
          this->Archive->IsPreExcluded(dep)) {
        continue;
      }

      std::string path;
      bool resolved = false;
      if (dep.find('/') != std::string::npos) {
        this->SetError("Paths to dependencies are not supported");
        return false;
      }
      if (!this->ResolveDependency(dep, searchPaths, path, resolved)) {
        return false;
      }
      if (resolved) {
        resolvedDependencies.emplace(dep);
        if (!this->Archive->IsPostExcluded(path)) {
          bool unique;
          this->Archive->AddResolvedPath(dep, path, unique);
          if (unique) {
            std::vector<std::string> combinedParentRpaths = parentRpaths;
            combinedParentRpaths.insert(combinedParentRpaths.end(),
                                        rpaths.begin(), rpaths.end());

            queueToResolve.push(std::make_pair(path, combinedParentRpaths));
          }
        }
      } else {
        this->Archive->AddUnresolvedPath(dep);
      }
    }
  }

  return true;
}

namespace {
bool FileHasArchitecture(const char* filename, std::uint16_t machine)
{
  cmELF elf(filename);
  if (!elf) {
    return false;
  }
  return machine == 0 || machine == elf.GetMachine();
}
}

bool cmBinUtilsLinuxELFLinker::ResolveDependency(
  std::string const& name, std::vector<std::string> const& searchPaths,
  std::string& path, bool& resolved)
{
  for (auto const& searchPath : searchPaths) {
    path = cmStrCat(searchPath, '/', name);
    if (cmSystemTools::PathExists(path) &&
        FileHasArchitecture(path.c_str(), this->Machine)) {
      resolved = true;
      return true;
    }
  }

  for (auto const& searchPath : this->Archive->GetSearchDirectories()) {
    path = cmStrCat(searchPath, '/', name);
    if (cmSystemTools::PathExists(path) &&
        FileHasArchitecture(path.c_str(), this->Machine)) {
      std::ostringstream warning;
      warning << "Dependency " << name << " found in search directory:\n  "
              << searchPath
              << "\nSee file(GET_RUNTIME_DEPENDENCIES) documentation for "
              << "more information.";
      this->Archive->GetMakefile()->IssueMessage(MessageType::WARNING,
                                                 warning.str());
      resolved = true;
      return true;
    }
  }

  resolved = false;
  return true;
}
