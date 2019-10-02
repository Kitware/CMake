/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsMacOSMachOLinker.h"

#include <sstream>
#include <string>
#include <vector>

#include <cm/memory>

#include "cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool.h"
#include "cmRuntimeDependencyArchive.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmBinUtilsMacOSMachOLinker::cmBinUtilsMacOSMachOLinker(
  cmRuntimeDependencyArchive* archive)
  : cmBinUtilsLinker(archive)
{
}

bool cmBinUtilsMacOSMachOLinker::Prepare()
{
  std::string tool = this->Archive->GetGetRuntimeDependenciesTool();
  if (tool.empty()) {
    tool = "otool";
  }
  if (tool == "otool") {
    this->Tool =
      cm::make_unique<cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool>(
        this->Archive);
  } else {
    std::ostringstream e;
    e << "Invalid value for CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL: " << tool;
    this->SetError(e.str());
    return false;
  }

  return true;
}

bool cmBinUtilsMacOSMachOLinker::ScanDependencies(
  std::string const& file, cmStateEnums::TargetType type)
{
  std::string executableFile;
  if (type == cmStateEnums::EXECUTABLE) {
    executableFile = file;
  } else {
    executableFile = this->Archive->GetBundleExecutable();
  }
  std::string executablePath;
  if (!executableFile.empty()) {
    executablePath = cmSystemTools::GetFilenamePath(executableFile);
  }
  return this->ScanDependencies(file, executablePath);
}

bool cmBinUtilsMacOSMachOLinker::ScanDependencies(
  std::string const& file, std::string const& executablePath)
{
  std::vector<std::string> libs;
  std::vector<std::string> rpaths;
  if (!this->Tool->GetFileInfo(file, libs, rpaths)) {
    return false;
  }

  std::string loaderPath = cmSystemTools::GetFilenamePath(file);
  return this->GetFileDependencies(libs, executablePath, loaderPath, rpaths);
}

bool cmBinUtilsMacOSMachOLinker::GetFileDependencies(
  std::vector<std::string> const& names, std::string const& executablePath,
  std::string const& loaderPath, std::vector<std::string> const& rpaths)
{
  for (std::string const& name : names) {
    if (!this->Archive->IsPreExcluded(name)) {
      std::string path;
      bool resolved;
      if (!this->ResolveDependency(name, executablePath, loaderPath, rpaths,
                                   path, resolved)) {
        return false;
      }
      if (resolved) {
        if (!this->Archive->IsPostExcluded(path)) {
          auto filename = cmSystemTools::GetFilenameName(path);
          bool unique;
          this->Archive->AddResolvedPath(filename, path, unique);
          if (unique && !this->ScanDependencies(path, executablePath)) {
            return false;
          }
        }
      } else {
        this->Archive->AddUnresolvedPath(name);
      }
    }
  }

  return true;
}

bool cmBinUtilsMacOSMachOLinker::ResolveDependency(
  std::string const& name, std::string const& executablePath,
  std::string const& loaderPath, std::vector<std::string> const& rpaths,
  std::string& path, bool& resolved)
{
  resolved = false;
  if (cmHasLiteralPrefix(name, "@rpath/")) {
    if (!this->ResolveRPathDependency(name, executablePath, loaderPath, rpaths,
                                      path, resolved)) {
      return false;
    }
  } else if (cmHasLiteralPrefix(name, "@loader_path/")) {
    if (!this->ResolveLoaderPathDependency(name, loaderPath, path, resolved)) {
      return false;
    }
  } else if (cmHasLiteralPrefix(name, "@executable_path/")) {
    if (!this->ResolveExecutablePathDependency(name, executablePath, path,
                                               resolved)) {
      return false;
    }
  } else {
    resolved = true;
    path = name;
  }

  if (resolved && !cmSystemTools::FileIsFullPath(path)) {
    this->SetError("Resolved path is not absolute");
    return false;
  }

  return true;
}

bool cmBinUtilsMacOSMachOLinker::ResolveExecutablePathDependency(
  std::string const& name, std::string const& executablePath,
  std::string& path, bool& resolved)
{
  if (executablePath.empty()) {
    resolved = false;
    return true;
  }

  // 16 is == "@executable_path".length()
  path = name;
  path.replace(0, 16, executablePath);

  if (!cmSystemTools::PathExists(path)) {
    resolved = false;
    return true;
  }

  resolved = true;
  return true;
}

bool cmBinUtilsMacOSMachOLinker::ResolveLoaderPathDependency(
  std::string const& name, std::string const& loaderPath, std::string& path,
  bool& resolved)
{
  if (loaderPath.empty()) {
    resolved = false;
    return true;
  }

  // 12 is "@loader_path".length();
  path = name;
  path.replace(0, 12, loaderPath);

  if (!cmSystemTools::PathExists(path)) {
    resolved = false;
    return true;
  }

  resolved = true;
  return true;
}

bool cmBinUtilsMacOSMachOLinker::ResolveRPathDependency(
  std::string const& name, std::string const& executablePath,
  std::string const& loaderPath, std::vector<std::string> const& rpaths,
  std::string& path, bool& resolved)
{
  for (std::string const& rpath : rpaths) {
    std::string searchFile = name;
    searchFile.replace(0, 6, rpath);
    if (cmHasLiteralPrefix(searchFile, "@loader_path/")) {
      if (!this->ResolveLoaderPathDependency(searchFile, loaderPath, path,
                                             resolved)) {
        return false;
      }
      if (resolved) {
        return true;
      }
    } else if (cmHasLiteralPrefix(searchFile, "@executable_path/")) {
      if (!this->ResolveExecutablePathDependency(searchFile, executablePath,
                                                 path, resolved)) {
        return false;
      }
      if (resolved) {
        return true;
      }
    } else if (cmSystemTools::PathExists(searchFile)) {
      /*
       * paraphrasing @ben.boeckel:
       *  if /b/libB.dylib is supposed to be used,
       *  /a/libbB.dylib will be found first if it exists. CMake tries to
       *  sort rpath directories to avoid this, but sometimes there is no
       *  right answer.
       *
       *  I believe it is possible to resolve this using otools -l
       *  then checking the LC_LOAD_DYLIB command whose name is
       *  equal to the value of search_file, UNLESS the build
       *  specifically sets the RPath to paths that will match
       *  duplicate libs; at this point can we just point to
       *  user error, or is there a reason why the advantages
       *  to this scenario outweigh its disadvantages?
       *
       *  Also priority seems to be the order as passed in when compiled
       *  so as long as this method's resolution guarantees priority
       *  in that manner further checking should not be necessary?
       */
      path = searchFile;
      resolved = true;
      return true;
    }
  }

  resolved = false;
  return true;
}
