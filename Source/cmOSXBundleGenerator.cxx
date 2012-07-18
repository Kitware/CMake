/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmOSXBundleGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"
#include "cmLocalGenerator.h"

#include <cassert>

void cmOSXBundleGenerator::PrepareTargetProperties(cmTarget* target)
{
  if(target->IsCFBundleOnApple())
    {
    target->SetProperty("PREFIX", "");
    target->SetProperty("SUFFIX", "");
    }
}

//----------------------------------------------------------------------------
cmOSXBundleGenerator::
cmOSXBundleGenerator(cmTarget* target,
                     std::string targetNameOut,
                     const char* configName)
 : Target(target)
 , Makefile(target->GetMakefile())
 , LocalGenerator(Makefile->GetLocalGenerator())
 , TargetNameOut(targetNameOut)
 , ConfigName(configName)
 , MacContentDirectory()
 , FrameworkVersion()
 , MacContentFolders(0)
{
  if (this->MustSkip())
    return;

  this->MacContentDirectory =
    this->Target->GetMacContentDirectory(this->ConfigName,
                                         /*implib*/ false,
                                         /*includeMacOS*/ false);
  if(this->Target->IsFrameworkOnApple())
    this->FrameworkVersion = this->Target->GetFrameworkVersion();
}

//----------------------------------------------------------------------------
bool cmOSXBundleGenerator::MustSkip()
{
  return !this->Target->HaveWellDefinedOutputFiles();
}

//----------------------------------------------------------------------------
void cmOSXBundleGenerator::CreateAppBundle(std::string& targetName,
                                           std::string& outpath)
{
  if (this->MustSkip())
    return;

  // Compute bundle directory names.
  outpath = this->MacContentDirectory;
  outpath += "MacOS";
  cmSystemTools::MakeDirectory(outpath.c_str());
  outpath += "/";
  this->Makefile->AddCMakeOutputFile(outpath.c_str());

  // Configure the Info.plist file.  Note that it needs the executable name
  // to be set.
  std::string plist = this->MacContentDirectory + "Info.plist";
  this->LocalGenerator->GenerateAppleInfoPList(this->Target,
                                               targetName.c_str(),
                                               plist.c_str());
  this->Makefile->AddCMakeOutputFile(plist.c_str());
}

//----------------------------------------------------------------------------
void cmOSXBundleGenerator::CreateFramework(std::string const& targetName)
{
  if (this->MustSkip())
    return;

  assert(this->MacContentFolders);

  // Configure the Info.plist file into the Resources directory.
  this->MacContentFolders->insert("Resources");
  std::string plist = this->MacContentDirectory + "Resources/Info.plist";
  this->LocalGenerator->GenerateFrameworkInfoPList(this->Target,
                                                   targetName.c_str(),
                                                   plist.c_str());

  // TODO: Use the cmMakefileTargetGenerator::ExtraFiles vector to
  // drive rules to create these files at build time.
  std::string oldName;
  std::string newName;

  // Compute the location of the top-level foo.framework directory.
  std::string top = this->Target->GetDirectory(this->ConfigName);
  top += "/";
  top += this->TargetNameOut;
  top += ".framework/";

  // Make foo.framework/Versions
  std::string versions = top;
  versions += "Versions";
  cmSystemTools::MakeDirectory(versions.c_str());

  // Make foo.framework/Versions/version
  std::string version = versions;
  version += "/";
  version += this->FrameworkVersion;
  cmSystemTools::MakeDirectory(version.c_str());

  // Current -> version
  oldName = this->FrameworkVersion;
  newName = versions;
  newName += "/Current";
  cmSystemTools::RemoveFile(newName.c_str());
  cmSystemTools::CreateSymlink(oldName.c_str(), newName.c_str());
  this->Makefile->AddCMakeOutputFile(newName.c_str());

  // foo -> Versions/Current/foo
  oldName = "Versions/Current/";
  oldName += this->TargetNameOut;
  newName = top;
  newName += this->TargetNameOut;
  cmSystemTools::RemoveFile(newName.c_str());
  cmSystemTools::CreateSymlink(oldName.c_str(), newName.c_str());
  this->Makefile->AddCMakeOutputFile(newName.c_str());

  // Resources -> Versions/Current/Resources
  if(this->MacContentFolders->find("Resources") !=
     this->MacContentFolders->end())
    {
    oldName = "Versions/Current/Resources";
    newName = top;
    newName += "Resources";
    cmSystemTools::RemoveFile(newName.c_str());
    cmSystemTools::CreateSymlink(oldName.c_str(), newName.c_str());
    this->Makefile->AddCMakeOutputFile(newName.c_str());
    }

  // Headers -> Versions/Current/Headers
  if(this->MacContentFolders->find("Headers") !=
     this->MacContentFolders->end())
    {
    oldName = "Versions/Current/Headers";
    newName = top;
    newName += "Headers";
    cmSystemTools::RemoveFile(newName.c_str());
    cmSystemTools::CreateSymlink(oldName.c_str(), newName.c_str());
    this->Makefile->AddCMakeOutputFile(newName.c_str());
    }

  // PrivateHeaders -> Versions/Current/PrivateHeaders
  if(this->MacContentFolders->find("PrivateHeaders") !=
     this->MacContentFolders->end())
    {
    oldName = "Versions/Current/PrivateHeaders";
    newName = top;
    newName += "PrivateHeaders";
    cmSystemTools::RemoveFile(newName.c_str());
    cmSystemTools::CreateSymlink(oldName.c_str(), newName.c_str());
    this->Makefile->AddCMakeOutputFile(newName.c_str());
    }
}

//----------------------------------------------------------------------------
void cmOSXBundleGenerator::CreateCFBundle(std::string& targetName,
                                          std::string& outpath)
{
  if (this->MustSkip())
    return;

  // Compute bundle directory names.
  outpath = this->MacContentDirectory;
  outpath += "MacOS";
  cmSystemTools::MakeDirectory(outpath.c_str());
  outpath += "/";
  this->Makefile->AddCMakeOutputFile(outpath.c_str());

  // Configure the Info.plist file.  Note that it needs the executable name
  // to be set.
  std::string plist = this->MacContentDirectory;
  plist += "Info.plist";
  this->LocalGenerator->GenerateAppleInfoPList(this->Target,
                                               targetName.c_str(),
                                               plist.c_str());
  this->Makefile->AddCMakeOutputFile(plist.c_str());
}

//----------------------------------------------------------------------------
void
cmOSXBundleGenerator::
GenerateMacOSXContentStatements(std::vector<cmSourceFile*> const& sources,
                                MacOSXContentGeneratorType* generator)
{
  if (this->MustSkip())
    return;

  for(std::vector<cmSourceFile*>::const_iterator
        si = sources.begin(); si != sources.end(); ++si)
    {
    cmTarget::SourceFileFlags tsFlags =
      this->Target->GetTargetSourceFileFlags(*si);
    if(tsFlags.Type != cmTarget::SourceFileTypeNormal)
      {
      (*generator)(**si, tsFlags.MacFolder);
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmOSXBundleGenerator::InitMacOSXContentDirectory(const char* pkgloc)
{
  // Construct the full path to the content subdirectory.
  std::string macdir = this->MacContentDirectory;
  macdir += pkgloc;
  cmSystemTools::MakeDirectory(macdir.c_str());

  // Record use of this content location.  Only the first level
  // directory is needed.
  {
  std::string loc = pkgloc;
  loc = loc.substr(0, loc.find('/'));
  this->MacContentFolders->insert(loc);
  }

  return macdir;
}
