/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSearchPath.h"

#include <algorithm>
#include <cassert>
#include <utility>

#include <cm/optional>

#include "cmFindCommon.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmWindowsRegistry.h"

cmSearchPath::cmSearchPath(cmFindCommon* findCmd)
  : FC(findCmd)
{
}

cmSearchPath::~cmSearchPath() = default;

void cmSearchPath::ExtractWithout(std::set<std::string> const& ignorePaths,
                                  std::set<std::string> const& ignorePrefixes,
                                  std::vector<std::string>& outPaths) const
{
  for (auto const& path : this->Paths) {
    if (ignorePaths.find(path.Path) == ignorePaths.end() &&
        ignorePrefixes.find(path.Prefix) == ignorePrefixes.end()) {
      outPaths.push_back(path.Path);
    }
  }
}

void cmSearchPath::AddPath(std::string const& path)
{
  this->AddPathInternal(path, "");
}

void cmSearchPath::AddUserPath(std::string const& path)
{
  assert(this->FC);

  std::vector<std::string> outPaths;

  cmWindowsRegistry registry(*this->FC->Makefile,
                             cmWindowsRegistry::SimpleTypes);
  auto expandedPaths = registry.ExpandExpression(path, this->FC->RegistryView);
  if (expandedPaths) {
    for (auto const& expandedPath : expandedPaths.value()) {
      cmSystemTools::GlobDirs(expandedPath, outPaths);
    }
  }

  // Process them all from the current directory
  for (std::string const& p : outPaths) {
    this->AddPathInternal(
      cmSystemTools::CollapseFullPath(
        p, this->FC->Makefile->GetCurrentSourceDirectory()),
      "");
  }
}

void cmSearchPath::AddCMakePath(std::string const& variable)
{
  assert(this->FC);

  // Get a path from a CMake variable.
  if (cmValue value = this->FC->Makefile->GetDefinition(variable)) {
    cmList expanded{ *value };

    for (std::string const& p : expanded) {
      this->AddPathInternal(
        cmSystemTools::CollapseFullPath(
          p, this->FC->Makefile->GetCurrentSourceDirectory()),
        "");
    }
  }
}

void cmSearchPath::AddEnvPath(std::string const& variable)
{
  std::vector<std::string> expanded =
    cmSystemTools::GetEnvPathNormalized(variable);
  for (std::string const& p : expanded) {
    this->AddPathInternal(p, "");
  }
}

void cmSearchPath::AddCMakePrefixPath(std::string const& variable)
{
  assert(this->FC);

  // Get a path from a CMake variable.
  if (cmValue value = this->FC->Makefile->GetDefinition(variable)) {
    cmList expanded{ *value };
    for (std::string& p : expanded) {
      p = cmSystemTools::CollapseFullPath(
        p, this->FC->Makefile->GetCurrentSourceDirectory());
    }
    this->AddPrefixPaths(expanded);
  }
}

static std::string cmSearchPathStripBin(std::string const& s)
{
  // If the path is a PREFIX/bin case then add its parent instead.
  if ((cmHasLiteralSuffix(s, "/bin")) || (cmHasLiteralSuffix(s, "/sbin"))) {
    return cmSystemTools::GetFilenamePath(s);
  }
  return s;
}

void cmSearchPath::AddEnvPrefixPath(std::string const& variable, bool stripBin)
{
  std::vector<std::string> expanded =
    cmSystemTools::GetEnvPathNormalized(variable);
  if (stripBin) {
    std::transform(expanded.begin(), expanded.end(), expanded.begin(),
                   cmSearchPathStripBin);
  }
  this->AddPrefixPaths(expanded);
}

void cmSearchPath::AddSuffixes(std::vector<std::string> const& suffixes)
{
  std::vector<PathWithPrefix> inPaths;
  inPaths.swap(this->Paths);
  this->Paths.reserve(inPaths.size() * (suffixes.size() + 1));

  for (PathWithPrefix& inPath : inPaths) {
    cmSystemTools::ConvertToUnixSlashes(inPath.Path);
    cmSystemTools::ConvertToUnixSlashes(inPath.Prefix);

    // if *i is only / then do not add a //
    // this will get incorrectly considered a network
    // path on windows and cause huge delays.
    std::string p = inPath.Path;
    if (!p.empty() && p.back() != '/') {
      p += '/';
    }

    // Combine with all the suffixes
    for (std::string const& suffix : suffixes) {
      this->Paths.emplace_back(PathWithPrefix{ p + suffix, inPath.Prefix });
    }

    // And now the original w/o any suffix
    this->Paths.emplace_back(std::move(inPath));
  }
}

void cmSearchPath::AddPrefixPaths(std::vector<std::string> const& paths)
{
  assert(this->FC);

  // default for programs
  std::string subdir = "bin";

  if (this->FC->CMakePathName == "INCLUDE") {
    subdir = "include";
  } else if (this->FC->CMakePathName == "LIBRARY") {
    subdir = "lib";
  } else if (this->FC->CMakePathName == "FRAMEWORK") {
    subdir.clear(); // ? what to do for frameworks ?
  }

  for (std::string const& path : paths) {
    std::string dir = path;
    if (!subdir.empty() && !dir.empty() && dir.back() != '/') {
      dir += "/";
    }
    std::string prefix = dir;
    if (!prefix.empty() && prefix != "/") {
      prefix.erase(prefix.size() - 1);
    }
    if (subdir == "include" || subdir == "lib") {
      cmValue arch =
        this->FC->Makefile->GetDefinition("CMAKE_LIBRARY_ARCHITECTURE");
      if (cmNonempty(arch)) {
        std::string archNoUnknown = *arch;
        auto unknownAtPos = archNoUnknown.find("-unknown-");
        bool foundUnknown = unknownAtPos != std::string::npos;
        if (foundUnknown) {
          // Replace "-unknown-" with "-".
          archNoUnknown.replace(unknownAtPos, 9, "-");
        }
        if (this->FC->Makefile->IsDefinitionSet("CMAKE_SYSROOT") &&
            this->FC->Makefile->IsDefinitionSet(
              "CMAKE_PREFIX_LIBRARY_ARCHITECTURE")) {
          if (foundUnknown) {
            this->AddPathInternal(cmStrCat('/', archNoUnknown, dir, subdir),
                                  cmStrCat('/', archNoUnknown, prefix));
          }
          this->AddPathInternal(cmStrCat('/', *arch, dir, subdir),
                                cmStrCat('/', *arch, prefix));
        } else {
          if (foundUnknown) {
            this->AddPathInternal(cmStrCat(dir, subdir, '/', archNoUnknown),
                                  prefix);
          }
          this->AddPathInternal(cmStrCat(dir, subdir, '/', *arch), prefix);
        }
      }
    }
    std::string add = dir + subdir;
    if (add != "/") {
      this->AddPathInternal(add, prefix);
    }
    if (subdir == "bin") {
      this->AddPathInternal(dir + "sbin", prefix);
    }
    if (!subdir.empty() && path != "/") {
      this->AddPathInternal(path, prefix);
    }
  }
}

void cmSearchPath::AddPathInternal(std::string const& path,
                                   std::string const& prefix)
{
  assert(this->FC);

  if (path.empty()) {
    return;
  }

  // Insert the path if has not already been emitted.
  PathWithPrefix pathWithPrefix{ path, prefix };
  if (this->FC->SearchPathsEmitted.insert(pathWithPrefix).second) {
    this->Paths.emplace_back(std::move(pathWithPrefix));
  }
}
