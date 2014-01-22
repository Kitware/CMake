/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSourceFileLocation.h"

#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmSourceFileLocation
::cmSourceFileLocation(cmMakefile const* mf, const char* name): Makefile(mf)
{
  this->AmbiguousDirectory = !cmSystemTools::FileIsFullPath(name);
  this->AmbiguousExtension = true;
  this->Directory = cmSystemTools::GetFilenamePath(name);
  this->Name = cmSystemTools::GetFilenameName(name);
  this->UpdateExtension(name);
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::Update(const char* name)
{
  if(this->AmbiguousDirectory)
    {
    this->UpdateDirectory(name);
    }
  if(this->AmbiguousExtension)
    {
    this->UpdateExtension(name);
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::Update(cmSourceFileLocation const& loc)
{
  if(this->AmbiguousDirectory && !loc.AmbiguousDirectory)
    {
    this->Directory = loc.Directory;
    this->AmbiguousDirectory = false;
    }
  if(this->AmbiguousExtension && !loc.AmbiguousExtension)
    {
    this->Name = loc.Name;
    this->AmbiguousExtension = false;
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::DirectoryUseSource()
{
  if(this->AmbiguousDirectory)
    {
    this->Directory =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentDirectory());
    this->AmbiguousDirectory = false;
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::DirectoryUseBinary()
{
  if(this->AmbiguousDirectory)
    {
    this->Directory =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentOutputDirectory());
    this->AmbiguousDirectory = false;
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::UpdateExtension(const char* name)
{
  // Check the extension.
  std::string ext = cmSystemTools::GetFilenameLastExtension(name);
  if(!ext.empty()) { ext = ext.substr(1); }

  // The global generator checks extensions of enabled languages.
  cmGlobalGenerator* gg =
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator();
  cmMakefile const* mf = this->Makefile;
  const std::vector<std::string>& srcExts = mf->GetSourceExtensions();
  const std::vector<std::string>& hdrExts = mf->GetHeaderExtensions();
  if(gg->GetLanguageFromExtension(ext.c_str()) ||
     std::find(srcExts.begin(), srcExts.end(), ext) != srcExts.end() ||
     std::find(hdrExts.begin(), hdrExts.end(), ext) != hdrExts.end())
    {
    // This is a known extension.  Use the given filename with extension.
    this->Name = cmSystemTools::GetFilenameName(name);
    this->AmbiguousExtension = false;
    }
  else
    {
    // This is not a known extension.  See if the file exists on disk as
    // named.
    std::string tryPath;
    if(this->AmbiguousDirectory)
      {
      // Check the source tree only because a file in the build tree should
      // be specified by full path at least once.  We do not want this
      // detection to depend on whether the project has already been built.
      tryPath = this->Makefile->GetCurrentDirectory();
      tryPath += "/";
      }
    if(!this->Directory.empty())
      {
      tryPath += this->Directory;
      tryPath += "/";
      }
    tryPath += this->Name;
    if(cmSystemTools::FileExists(tryPath.c_str(), true))
      {
      // We found a source file named by the user on disk.  Trust it's
      // extension.
      this->Name = cmSystemTools::GetFilenameName(name);
      this->AmbiguousExtension = false;

      // If the directory was ambiguous, it isn't anymore.
      if(this->AmbiguousDirectory)
        {
        this->DirectoryUseSource();
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmSourceFileLocation::UpdateDirectory(const char* name)
{
  // If a full path was given we know the directory.
  if(cmSystemTools::FileIsFullPath(name))
    {
    this->Directory = cmSystemTools::GetFilenamePath(name);
    this->AmbiguousDirectory = false;
    }
}

//----------------------------------------------------------------------------
bool
cmSourceFileLocation
::MatchesAmbiguousExtension(cmSourceFileLocation const& loc) const
{
  // This location's extension is not ambiguous but loc's extension
  // is.  See if the names match as-is.
  if(this->Name == loc.Name)
    {
    return true;
    }

  // Check if loc's name could possibly be extended to our name by
  // adding an extension.
  if(!(this->Name.size() > loc.Name.size() &&
       this->Name.substr(0, loc.Name.size()) == loc.Name &&
       this->Name[loc.Name.size()] == '.'))
    {
    return false;
    }

  // Only a fixed set of extensions will be tried to match a file on
  // disk.  One of these must match if loc refers to this source file.
  std::string ext = this->Name.substr(loc.Name.size()+1);
  cmMakefile const* mf = this->Makefile;
  const std::vector<std::string>& srcExts = mf->GetSourceExtensions();
  if(std::find(srcExts.begin(), srcExts.end(), ext) != srcExts.end())
    {
    return true;
    }
  const std::vector<std::string>& hdrExts = mf->GetHeaderExtensions();
  if(std::find(hdrExts.begin(), hdrExts.end(), ext) != hdrExts.end())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmSourceFileLocation::Matches(cmSourceFileLocation const& loc)
{
  if(this->AmbiguousExtension && loc.AmbiguousExtension)
    {
    // Both extensions are ambiguous.  Since only the old fixed set of
    // extensions will be tried, the names must match at this point to
    // be the same file.
    if(this->Name != loc.Name)
      {
      return false;
      }
    }
  else if(this->AmbiguousExtension)
    {
    // Only "this" extension is ambiguous.
    if(!loc.MatchesAmbiguousExtension(*this))
      {
      return false;
      }
    }
  else if(loc.AmbiguousExtension)
    {
    // Only "loc" extension is ambiguous.
    if(!this->MatchesAmbiguousExtension(loc))
      {
      return false;
      }
    }
  else
    {
    // Neither extension is ambiguous.
    if(this->Name != loc.Name)
      {
      return false;
      }
    }

  if(!this->AmbiguousDirectory && !loc.AmbiguousDirectory)
    {
    // Both sides have absolute directories.
    if(this->Directory != loc.Directory)
      {
      return false;
      }
    }
  else if(this->AmbiguousDirectory && loc.AmbiguousDirectory &&
          this->Makefile == loc.Makefile)
    {
    // Both sides have directories relative to the same location.
    if(this->Directory != loc.Directory)
      {
      return false;
      }
    }
  else if(this->AmbiguousDirectory && loc.AmbiguousDirectory)
    {
    // Each side has a directory relative to a different location.
    // This can occur when referencing a source file from a different
    // directory.  This is not yet allowed.
    this->Makefile->IssueMessage(
      cmake::INTERNAL_ERROR,
      "Matches error: Each side has a directory relative to a different "
      "location. This can occur when referencing a source file from a "
      "different directory.  This is not yet allowed."
      );
    return false;
    }
  else if(this->AmbiguousDirectory)
    {
    // Compare possible directory combinations.
    std::string srcDir =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentDirectory());
    std::string binDir =
      cmSystemTools::CollapseFullPath(
        this->Directory.c_str(), this->Makefile->GetCurrentOutputDirectory());
    if(srcDir != loc.Directory &&
       binDir != loc.Directory)
      {
      return false;
      }
    }
  else if(loc.AmbiguousDirectory)
    {
    // Compare possible directory combinations.
    std::string srcDir =
      cmSystemTools::CollapseFullPath(
        loc.Directory.c_str(), loc.Makefile->GetCurrentDirectory());
    std::string binDir =
      cmSystemTools::CollapseFullPath(
        loc.Directory.c_str(), loc.Makefile->GetCurrentOutputDirectory());
    if(srcDir != this->Directory &&
       binDir != this->Directory)
      {
      return false;
      }
    }

  // File locations match.
  this->Update(loc);
  return true;
}
