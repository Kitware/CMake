/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmOSXBundleGenerator.h"

#include <cassert>

#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

class cmSourceFile;

cmOSXBundleGenerator::cmOSXBundleGenerator(cmGeneratorTarget* target)
  : GT(target)
  , Makefile(target->Target->GetMakefile())
  , LocalGenerator(target->GetLocalGenerator())
  , MacContentFolders(nullptr)
{
  if (this->MustSkip()) {
    return;
  }
}

bool cmOSXBundleGenerator::MustSkip()
{
  return !this->GT->HaveWellDefinedOutputFiles();
}

void cmOSXBundleGenerator::CreateAppBundle(const std::string& targetName,
                                           std::string& outpath,
                                           const std::string& config)
{
  if (this->MustSkip()) {
    return;
  }

  // Compute bundle directory names.
  std::string out = cmStrCat(
    outpath, '/',
    this->GT->GetAppBundleDirectory(config, cmGeneratorTarget::FullLevel));
  cmSystemTools::MakeDirectory(out);
  this->Makefile->AddCMakeOutputFile(out);

  // Configure the Info.plist file.  Note that it needs the executable name
  // to be set.
  std::string plist = cmStrCat(
    outpath, '/',
    this->GT->GetAppBundleDirectory(config, cmGeneratorTarget::ContentLevel),
    "/Info.plist");
  this->LocalGenerator->GenerateAppleInfoPList(this->GT, targetName, plist);
  this->Makefile->AddCMakeOutputFile(plist);
  outpath = out;
}

void cmOSXBundleGenerator::CreateFramework(const std::string& targetName,
                                           const std::string& outpath,
                                           const std::string& config)
{
  if (this->MustSkip()) {
    return;
  }

  assert(this->MacContentFolders);

  // Compute the location of the top-level foo.framework directory.
  std::string contentdir = cmStrCat(
    outpath, '/',
    this->GT->GetFrameworkDirectory(config, cmGeneratorTarget::ContentLevel),
    '/');

  std::string newoutpath = outpath + "/" +
    this->GT->GetFrameworkDirectory(config, cmGeneratorTarget::FullLevel);

  std::string frameworkVersion = this->GT->GetFrameworkVersion();

  // Configure the Info.plist file
  std::string plist = newoutpath;
  if (!this->Makefile->PlatformIsAppleEmbedded()) {
    // Put the Info.plist file into the Resources directory.
    this->MacContentFolders->insert("Resources");
    plist += "/Resources";
  }
  plist += "/Info.plist";
  std::string name = cmSystemTools::GetFilenameName(targetName);
  this->LocalGenerator->GenerateFrameworkInfoPList(this->GT, name, plist);

  // Generate Versions directory only for MacOSX frameworks
  if (this->Makefile->PlatformIsAppleEmbedded()) {
    return;
  }

  // TODO: Use the cmMakefileTargetGenerator::ExtraFiles vector to
  // drive rules to create these files at build time.
  std::string oldName;
  std::string newName;

  // Make foo.framework/Versions
  std::string versions = cmStrCat(contentdir, "Versions");
  cmSystemTools::MakeDirectory(versions);

  // Make foo.framework/Versions/version
  cmSystemTools::MakeDirectory(newoutpath);

  // Current -> version
  oldName = frameworkVersion;
  newName = cmStrCat(versions, "/Current");
  cmSystemTools::RemoveFile(newName);
  cmSystemTools::CreateSymlink(oldName, newName);
  this->Makefile->AddCMakeOutputFile(newName);

  // foo -> Versions/Current/foo
  oldName = cmStrCat("Versions/Current/", name);
  newName = cmStrCat(contentdir, name);
  cmSystemTools::RemoveFile(newName);
  cmSystemTools::CreateSymlink(oldName, newName);
  this->Makefile->AddCMakeOutputFile(newName);

  // Resources -> Versions/Current/Resources
  if (this->MacContentFolders->find("Resources") !=
      this->MacContentFolders->end()) {
    oldName = "Versions/Current/Resources";
    newName = cmStrCat(contentdir, "Resources");
    cmSystemTools::RemoveFile(newName);
    cmSystemTools::CreateSymlink(oldName, newName);
    this->Makefile->AddCMakeOutputFile(newName);
  }

  // Headers -> Versions/Current/Headers
  if (this->MacContentFolders->find("Headers") !=
      this->MacContentFolders->end()) {
    oldName = "Versions/Current/Headers";
    newName = cmStrCat(contentdir, "Headers");
    cmSystemTools::RemoveFile(newName);
    cmSystemTools::CreateSymlink(oldName, newName);
    this->Makefile->AddCMakeOutputFile(newName);
  }

  // PrivateHeaders -> Versions/Current/PrivateHeaders
  if (this->MacContentFolders->find("PrivateHeaders") !=
      this->MacContentFolders->end()) {
    oldName = "Versions/Current/PrivateHeaders";
    newName = cmStrCat(contentdir, "PrivateHeaders");
    cmSystemTools::RemoveFile(newName);
    cmSystemTools::CreateSymlink(oldName, newName);
    this->Makefile->AddCMakeOutputFile(newName);
  }
}

void cmOSXBundleGenerator::CreateCFBundle(const std::string& targetName,
                                          const std::string& root,
                                          const std::string& config)
{
  if (this->MustSkip()) {
    return;
  }

  // Compute bundle directory names.
  std::string out = cmStrCat(
    root, '/',
    this->GT->GetCFBundleDirectory(config, cmGeneratorTarget::FullLevel));
  cmSystemTools::MakeDirectory(out);
  this->Makefile->AddCMakeOutputFile(out);

  // Configure the Info.plist file.  Note that it needs the executable name
  // to be set.
  std::string plist = cmStrCat(
    root, '/',
    this->GT->GetCFBundleDirectory(config, cmGeneratorTarget::ContentLevel),
    "/Info.plist");
  std::string name = cmSystemTools::GetFilenameName(targetName);
  this->LocalGenerator->GenerateAppleInfoPList(this->GT, name, plist);
  this->Makefile->AddCMakeOutputFile(plist);
}

void cmOSXBundleGenerator::GenerateMacOSXContentStatements(
  std::vector<cmSourceFile const*> const& sources,
  MacOSXContentGeneratorType* generator, const std::string& config)
{
  if (this->MustSkip()) {
    return;
  }

  for (cmSourceFile const* source : sources) {
    cmGeneratorTarget::SourceFileFlags tsFlags =
      this->GT->GetTargetSourceFileFlags(source);
    if (tsFlags.Type != cmGeneratorTarget::SourceFileTypeNormal) {
      (*generator)(*source, tsFlags.MacFolder, config);
    }
  }
}

std::string cmOSXBundleGenerator::InitMacOSXContentDirectory(
  const char* pkgloc, const std::string& config)
{
  // Construct the full path to the content subdirectory.

  std::string macdir = cmStrCat(this->GT->GetMacContentDirectory(
                                  config, cmStateEnums::RuntimeBinaryArtifact),
                                '/', pkgloc);
  cmSystemTools::MakeDirectory(macdir);

  // Record use of this content location.  Only the first level
  // directory is needed.
  {
    std::string loc = pkgloc;
    loc = loc.substr(0, loc.find('/'));
    this->MacContentFolders->insert(loc);
  }

  return macdir;
}
