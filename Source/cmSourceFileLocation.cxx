/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSourceFileLocation.h"

#include <cassert>

#include <cm/string_view>

#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmSourceFileLocation::cmSourceFileLocation() = default;

cmSourceFileLocation::cmSourceFileLocation(const cmSourceFileLocation& loc)
  : Makefile(loc.Makefile)
{
  this->AmbiguousDirectory = loc.AmbiguousDirectory;
  this->AmbiguousExtension = loc.AmbiguousExtension;
  this->Directory = loc.Directory;
  this->Name = loc.Name;
}

cmSourceFileLocation::cmSourceFileLocation(cmMakefile const* mf,
                                           const std::string& name,
                                           cmSourceFileLocationKind kind)
  : Makefile(mf)
{
  this->AmbiguousDirectory = !cmSystemTools::FileIsFullPath(name);
  this->AmbiguousExtension = true;
  this->Directory = cmSystemTools::GetFilenamePath(name);
  if (cmSystemTools::FileIsFullPath(this->Directory)) {
    this->Directory = cmSystemTools::CollapseFullPath(this->Directory);
  }
  this->Name = cmSystemTools::GetFilenameName(name);
  if (kind == cmSourceFileLocationKind::Known) {
    this->DirectoryUseSource();
    this->AmbiguousExtension = false;
  } else {
    this->UpdateExtension(name);
  }
}

std::string cmSourceFileLocation::GetFullPath() const
{
  std::string path = this->GetDirectory();
  if (!path.empty()) {
    path += '/';
  }
  path += this->GetName();
  return path;
}

void cmSourceFileLocation::Update(cmSourceFileLocation const& loc)
{
  if (this->AmbiguousDirectory && !loc.AmbiguousDirectory) {
    this->Directory = loc.Directory;
    this->AmbiguousDirectory = false;
  }
  if (this->AmbiguousExtension && !loc.AmbiguousExtension) {
    this->Name = loc.Name;
    this->AmbiguousExtension = false;
  }
}

void cmSourceFileLocation::DirectoryUseSource()
{
  assert(this->Makefile);
  if (this->AmbiguousDirectory) {
    this->Directory = cmSystemTools::CollapseFullPath(
      this->Directory, this->Makefile->GetCurrentSourceDirectory());
    this->AmbiguousDirectory = false;
  }
}

void cmSourceFileLocation::DirectoryUseBinary()
{
  assert(this->Makefile);
  if (this->AmbiguousDirectory) {
    this->Directory = cmSystemTools::CollapseFullPath(
      this->Directory, this->Makefile->GetCurrentBinaryDirectory());
    this->AmbiguousDirectory = false;
  }
}

void cmSourceFileLocation::UpdateExtension(const std::string& name)
{
  assert(this->Makefile);
  // Check the extension.
  std::string ext = cmSystemTools::GetFilenameLastExtension(name);
  if (!ext.empty()) {
    ext = ext.substr(1);
  }

  // The global generator checks extensions of enabled languages.
  cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();
  cmMakefile const* mf = this->Makefile;
  auto* cm = mf->GetCMakeInstance();
  if (!gg->GetLanguageFromExtension(ext.c_str()).empty() ||
      cm->IsAKnownExtension(ext)) {
    // This is a known extension.  Use the given filename with extension.
    this->Name = cmSystemTools::GetFilenameName(name);
    this->AmbiguousExtension = false;
  } else {
    // This is not a known extension.  See if the file exists on disk as
    // named.
    std::string tryPath;
    if (this->AmbiguousDirectory) {
      // Check the source tree only because a file in the build tree should
      // be specified by full path at least once.  We do not want this
      // detection to depend on whether the project has already been built.
      tryPath = cmStrCat(this->Makefile->GetCurrentSourceDirectory(), '/');
    }
    if (!this->Directory.empty()) {
      tryPath += this->Directory;
      tryPath += "/";
    }
    tryPath += this->Name;
    if (cmSystemTools::FileExists(tryPath, true)) {
      // We found a source file named by the user on disk.  Trust it's
      // extension.
      this->Name = cmSystemTools::GetFilenameName(name);
      this->AmbiguousExtension = false;

      // If the directory was ambiguous, it isn't anymore.
      if (this->AmbiguousDirectory) {
        this->DirectoryUseSource();
      }
    }
  }
}

bool cmSourceFileLocation::MatchesAmbiguousExtension(
  cmSourceFileLocation const& loc) const
{
  assert(this->Makefile);
  // This location's extension is not ambiguous but loc's extension
  // is.  See if the names match as-is.
  if (this->Name == loc.Name) {
    return true;
  }

  // Check if loc's name could possibly be extended to our name by
  // adding an extension.
  if (!(this->Name.size() > loc.Name.size() &&
        this->Name[loc.Name.size()] == '.' &&
        cmHasPrefix(this->Name, loc.Name))) {
    return false;
  }

  // Only a fixed set of extensions will be tried to match a file on
  // disk.  One of these must match if loc refers to this source file.
  auto ext = cm::string_view(this->Name).substr(loc.Name.size() + 1);
  cmMakefile const* mf = this->Makefile;
  auto* cm = mf->GetCMakeInstance();
  return cm->IsAKnownExtension(ext);
}

bool cmSourceFileLocation::Matches(cmSourceFileLocation const& loc)
{
  assert(this->Makefile);
  if (this->AmbiguousExtension == loc.AmbiguousExtension) {
    // Both extensions are similarly ambiguous.  Since only the old fixed set
    // of extensions will be tried, the names must match at this point to be
    // the same file.
    if (this->Name.size() != loc.Name.size() ||
        !cmSystemTools::ComparePath(this->Name, loc.Name)) {
      return false;
    }
  } else {
    const cmSourceFileLocation* loc1;
    const cmSourceFileLocation* loc2;
    if (this->AmbiguousExtension) {
      // Only "this" extension is ambiguous.
      loc1 = &loc;
      loc2 = this;
    } else {
      // Only "loc" extension is ambiguous.
      loc1 = this;
      loc2 = &loc;
    }
    if (!loc1->MatchesAmbiguousExtension(*loc2)) {
      return false;
    }
  }

  if (!this->AmbiguousDirectory && !loc.AmbiguousDirectory) {
    // Both sides have absolute directories.
    if (this->Directory != loc.Directory) {
      return false;
    }
  } else if (this->AmbiguousDirectory && loc.AmbiguousDirectory) {
    if (this->Makefile == loc.Makefile) {
      // Both sides have directories relative to the same location.
      if (this->Directory != loc.Directory) {
        return false;
      }
    } else {
      // Each side has a directory relative to a different location.
      // This can occur when referencing a source file from a different
      // directory.  This is not yet allowed.
      this->Makefile->IssueMessage(
        MessageType::INTERNAL_ERROR,
        "Matches error: Each side has a directory relative to a different "
        "location. This can occur when referencing a source file from a "
        "different directory.  This is not yet allowed.");
      return false;
    }
  } else if (this->AmbiguousDirectory) {
    // Compare possible directory combinations.
    std::string const srcDir = cmSystemTools::CollapseFullPath(
      this->Directory, this->Makefile->GetCurrentSourceDirectory());
    std::string const binDir = cmSystemTools::CollapseFullPath(
      this->Directory, this->Makefile->GetCurrentBinaryDirectory());
    if (srcDir != loc.Directory && binDir != loc.Directory) {
      return false;
    }
  } else if (loc.AmbiguousDirectory) {
    // Compare possible directory combinations.
    std::string const srcDir = cmSystemTools::CollapseFullPath(
      loc.Directory, loc.Makefile->GetCurrentSourceDirectory());
    std::string const binDir = cmSystemTools::CollapseFullPath(
      loc.Directory, loc.Makefile->GetCurrentBinaryDirectory());
    if (srcDir != this->Directory && binDir != this->Directory) {
      return false;
    }
  }

  // File locations match.
  this->Update(loc);
  return true;
}
