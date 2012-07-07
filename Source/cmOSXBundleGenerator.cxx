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

cmOSXBundleGenerator::
cmOSXBundleGenerator(cmTarget* target,
                     std::string targetNameOut,
                     const char* configName)
 : Target(target)
 , Makefile(target->GetMakefile())
 , LocalGenerator(this->Makefile->GetLocalGenerator())
 , TargetNameOut(targetNameOut)
 , ConfigName(configName)
 , MacContentDirectory()
 , FrameworkVersion()
 , MacContentFolders(0)
{
  if(this->Target->IsAppBundleOnApple())
    {
    this->MacContentDirectory = this->Target->GetDirectory(this->ConfigName);
    this->MacContentDirectory += "/";
    this->MacContentDirectory += this->TargetNameOut;
    this->MacContentDirectory += ".app/Contents/";
    }
  else if(this->Target->IsFrameworkOnApple())
    {
    this->FrameworkVersion = this->Target->GetFrameworkVersion();
    this->MacContentDirectory = this->Target->GetDirectory(this->ConfigName);
    this->MacContentDirectory += "/";
    this->MacContentDirectory += this->TargetNameOut;
    this->MacContentDirectory += ".framework/Versions/";
    this->MacContentDirectory += this->FrameworkVersion;
    this->MacContentDirectory += "/";
    }
  else if(this->Target->IsCFBundleOnApple())
    {
    this->MacContentDirectory = this->Target->GetDirectory(this->ConfigName);
    this->MacContentDirectory += "/";
    this->MacContentDirectory += this->TargetNameOut;
    this->MacContentDirectory += ".";
    const char *ext = this->Target->GetProperty("BUNDLE_EXTENSION");
    if (!ext)
      {
      ext = "bundle";
      }
    this->MacContentDirectory += ext;
    this->MacContentDirectory += "/Contents/";
    }
}

//----------------------------------------------------------------------------
void cmOSXBundleGenerator::CreateAppBundle(std::string& targetName,
                                           std::string& outpath)
{
  // Compute bundle directory names.
  outpath = this->MacContentDirectory;
  outpath += "MacOS";
  cmSystemTools::MakeDirectory(outpath.c_str());
  this->Makefile->AddCMakeOutputFile(outpath.c_str());
  outpath += "/";

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
