/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsWindowsPELinker.h"

#include <sstream>
#include <vector>

#include <cm/memory>

#include "cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool.h"
#include "cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#ifdef _WIN32
#  include <windows.h>
#endif

cmBinUtilsWindowsPELinker::cmBinUtilsWindowsPELinker(
  cmRuntimeDependencyArchive* archive)
  : cmBinUtilsLinker(archive)
{
}

bool cmBinUtilsWindowsPELinker::Prepare()
{
  std::string tool = this->Archive->GetGetRuntimeDependenciesTool();
  if (tool.empty()) {
    std::vector<std::string> command;
    if (this->Archive->GetGetRuntimeDependenciesCommand("dumpbin", command)) {
      tool = "dumpbin";
    } else {
      tool = "objdump";
    }
  }
  if (tool == "dumpbin") {
    this->Tool =
      cm::make_unique<cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool>(
        this->Archive);
  } else if (tool == "objdump") {
    this->Tool =
      cm::make_unique<cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool>(
        this->Archive);
  } else {
    std::ostringstream e;
    e << "Invalid value for CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL: " << tool;
    this->SetError(e.str());
    return false;
  }

  return true;
}

bool cmBinUtilsWindowsPELinker::ScanDependencies(
  std::string const& file, cmStateEnums::TargetType /* unused */)
{
  std::vector<std::string> needed;
  if (!this->Tool->GetFileInfo(file, needed)) {
    return false;
  }
  for (auto& n : needed) {
    n = cmSystemTools::LowerCase(n);
  }
  std::string origin = cmSystemTools::GetFilenamePath(file);

  for (auto const& lib : needed) {
    if (!this->Archive->IsPreExcluded(lib)) {
      std::string path;
      bool resolved = false;
      if (!this->ResolveDependency(lib, origin, path, resolved)) {
        return false;
      }
      if (resolved) {
        if (!this->Archive->IsPostExcluded(path)) {
          bool unique;
          this->Archive->AddResolvedPath(lib, path, unique);
          if (unique &&
              !this->ScanDependencies(path, cmStateEnums::SHARED_LIBRARY)) {
            return false;
          }
        }
      } else {
        this->Archive->AddUnresolvedPath(lib);
      }
    }
  }

  return true;
}

bool cmBinUtilsWindowsPELinker::ResolveDependency(std::string const& name,
                                                  std::string const& origin,
                                                  std::string& path,
                                                  bool& resolved)
{
  auto dirs = this->Archive->GetSearchDirectories();

#ifdef _WIN32
  char buf[MAX_PATH];
  unsigned int len;
  if ((len = GetWindowsDirectoryA(buf, MAX_PATH)) > 0) {
    dirs.insert(dirs.begin(), std::string(buf, len));
  }
  if ((len = GetSystemDirectoryA(buf, MAX_PATH)) > 0) {
    dirs.insert(dirs.begin(), std::string(buf, len));
  }
#endif

  dirs.insert(dirs.begin(), origin);

  for (auto const& searchPath : dirs) {
    path = cmStrCat(searchPath, '/', name);
    if (cmSystemTools::PathExists(path)) {
      resolved = true;
      return true;
    }
  }

  resolved = false;
  return true;
}
