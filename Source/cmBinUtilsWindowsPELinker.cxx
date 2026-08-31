/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmBinUtilsWindowsPELinker.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/filesystem>
#include <cm/memory>

#include "cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool.h"
#include "cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmSystemTools.h"
#include "cmTargetTypes.h"

#ifdef _WIN32
#  include <windows.h>

#  include "cmsys/Encoding.hxx"

#  include "cmStringAlgorithms.h"
#else
#  include "cmsys/Directory.hxx"
#endif

namespace {

#ifdef _WIN32
std::string ReplaceWithActualNameCasing(std::string path)
{
  WIN32_FIND_DATAW findData;
  HANDLE hFind = ::FindFirstFileW(
    cmsys::Encoding::ToWindowsExtendedPath(path).c_str(), &findData);
  if (hFind != INVALID_HANDLE_VALUE) {
    auto onDiskName = cmsys::Encoding::ToNarrow(findData.cFileName);
    ::FindClose(hFind);
    path.replace(path.end() - onDiskName.size(), path.end(), onDiskName);
  }
  return path;
}
#else
bool FindCaseInsensitive(std::string const& dir, std::string const& lowerName,
                         std::string& foundPath)
{
  if (!cmSystemTools::FileIsDirectory(dir)) {
    return false;
  }
  cmsys::Directory directory;
  if (!directory.Load(dir)) {
    return false;
  }
  for (std::size_t i = 0; i < directory.GetNumberOfFiles(); ++i) {
    cm::filesystem::path fileName = directory.GetFile(i);

    if (cmSystemTools::LowerCase(fileName) == lowerName) {
      foundPath += dir / fileName;
      return true;
    }
  }

  return false;
}
#endif
}

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

cmBinUtilsWindowsPELinker::Dependency::Dependency(std::string casedName)
  : CasedName(std::move(casedName))
  , LowerName(cmSystemTools::LowerCase(CasedName))
{
}

bool cmBinUtilsWindowsPELinker::ScanDependencies(std::string const& file,
                                                 cm::TargetType /* unused */)
{
  std::vector<std::string> needed;
  if (!this->Tool->GetFileInfo(file, needed)) {
    return false;
  }

  std::vector<Dependency> depends;
  depends.reserve(needed.size());
  std::move(needed.begin(), needed.end(), std::back_inserter(depends));
  std::string origin = cmSystemTools::GetFilenamePath(file);

  for (Dependency const& lib : depends) {
    if (this->Archive->IsPreExcluded(lib.LowerName)) {
      continue;
    }
    Dependency path;
    bool resolved = false;
    if (!this->ResolveDependency(lib, origin, path, resolved)) {
      return false;
    }
    if (resolved) {
      if (this->Archive->IsPostExcluded(path.LowerName, path.CasedName)) {
        continue;
      }
      bool unique;
      this->Archive->AddResolvedPath(lib.CasedName, path.CasedName, unique);
      if (unique &&
          !this->ScanDependencies(path.CasedName,
                                  cm::TargetType::SHARED_LIBRARY)) {
        return false;
      }
    } else {
      this->Archive->AddUnresolvedPath(lib.CasedName);
    }
  }

  return true;
}

bool cmBinUtilsWindowsPELinker::ResolveDependency(Dependency const& lib,
                                                  std::string const& origin,
                                                  Dependency& path,
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
#ifdef _WIN32
    path.LowerName = cmStrCat(searchPath, '/', lib.LowerName);
    if (!cmSystemTools::PathExists(path.LowerName)) {
      continue;
    }
    this->NormalizePath(path.LowerName);
    path.CasedName = ReplaceWithActualNameCasing(path.LowerName);
#else
    if (!FindCaseInsensitive(searchPath, lib.LowerName, path.CasedName)) {
      continue;
    }
    this->NormalizePath(path.CasedName);
    path.LowerName = [&lib](std::string libPath) -> std::string {
      libPath.replace(libPath.end() - lib.LowerName.size(), libPath.end(),
                      lib.LowerName);
      return libPath;
    }(path.CasedName);
#endif
    resolved = true;
    return true;
  }

  resolved = false;
  return true;
}
