/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackIFWGenerator.h"

#include <sstream>
#include <utility>

#include "cmCPackComponentGroup.h"
#include "cmCPackGenerator.h"
#include "cmCPackIFWCommon.h"
#include "cmCPackIFWInstaller.h"
#include "cmCPackIFWPackage.h"
#include "cmCPackIFWRepository.h"
#include "cmCPackLog.h" // IWYU pragma: keep
#include "cmDuration.h"
#include "cmGeneratedFileStream.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

cmCPackIFWGenerator::cmCPackIFWGenerator()
{
  this->Generator = this;
}

cmCPackIFWGenerator::~cmCPackIFWGenerator() = default;

int cmCPackIFWGenerator::PackageFiles()
{
  cmCPackIFWLogger(OUTPUT, "- Configuration" << std::endl);

  // Installer configuragion
  this->Installer.GenerateInstallerFile();

  // Packages configuration
  this->Installer.GeneratePackageFiles();

  std::string ifwTLD = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  std::string ifwTmpFile = cmStrCat(ifwTLD, "/IFWOutput.log");

  // Create repositories
  if (!this->RunRepogen(ifwTmpFile)) {
    return 0;
  }

  // Create installer
  if (!this->RunBinaryCreator(ifwTmpFile)) {
    return 0;
  }

  return 1;
}

std::vector<std::string> cmCPackIFWGenerator::BuildRepogenCommand()
{
  std::vector<std::string> ifwCmd;
  std::string ifwArg;

  ifwCmd.emplace_back(this->RepoGen);

  if (!this->IsVersionLess("4.2")) {
    if (!this->ArchiveFormat.empty()) {
      ifwCmd.emplace_back("--archive-format");
      ifwCmd.emplace_back(this->ArchiveFormat);
    }
    if (!this->ArchiveCompression.empty()) {
      ifwCmd.emplace_back("--compression");
      ifwCmd.emplace_back(this->ArchiveCompression);
    }
  }

  if (this->IsVersionLess("2.0.0")) {
    ifwCmd.emplace_back("-c");
    ifwCmd.emplace_back(this->toplevel + "/config/config.xml");
  }

  ifwCmd.emplace_back("-p");
  ifwCmd.emplace_back(this->toplevel + "/packages");

  if (!this->PkgsDirsVector.empty()) {
    for (std::string const& it : this->PkgsDirsVector) {
      ifwCmd.emplace_back("-p");
      ifwCmd.emplace_back(it);
    }
  }

  if (!this->RepoDirsVector.empty()) {
    if (!this->IsVersionLess("3.1")) {
      for (std::string const& rd : this->RepoDirsVector) {
        ifwCmd.emplace_back("--repository");
        ifwCmd.emplace_back(rd);
      }
    } else {
      cmCPackIFWLogger(WARNING,
                       "The \"CPACK_IFW_REPOSITORIES_DIRECTORIES\" "
                         << "variable is set, but content will be skipped, "
                         << "because this feature available only since "
                         << "QtIFW 3.1. Please update your QtIFW instance."
                         << std::endl);
    }
  }

  if (!this->OnlineOnly && !this->DownloadedPackages.empty()) {
    ifwCmd.emplace_back("-i");
    auto it = this->DownloadedPackages.begin();
    ifwArg = (*it)->Name;
    ++it;
    while (it != this->DownloadedPackages.end()) {
      ifwArg += "," + (*it)->Name;
      ++it;
    }
    ifwCmd.emplace_back(ifwArg);
  }
  ifwCmd.emplace_back(this->toplevel + "/repository");

  return ifwCmd;
}

int cmCPackIFWGenerator::RunRepogen(const std::string& ifwTmpFile)
{
  if (this->Installer.RemoteRepositories.empty()) {
    return 1;
  }

  std::vector<std::string> ifwCmd = this->BuildRepogenCommand();
  cmCPackIFWLogger(VERBOSE,
                   "Execute: " << cmSystemTools::PrintSingleCommand(ifwCmd)
                               << std::endl);
  std::string output;
  int retVal = 1;
  cmCPackIFWLogger(OUTPUT, "- Generate repository" << std::endl);
  bool res = cmSystemTools::RunSingleCommand(ifwCmd, &output, &output, &retVal,
                                             nullptr, this->GeneratorVerbose,
                                             cmDuration::zero());
  if (!res || retVal) {
    cmGeneratedFileStream ofs(ifwTmpFile);
    ofs << "# Run command: " << cmSystemTools::PrintSingleCommand(ifwCmd)
        << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackIFWLogger(
      ERROR,
      "Problem running IFW command: "
        << cmSystemTools::PrintSingleCommand(ifwCmd) << std::endl
        << "Please check \"" << ifwTmpFile << "\" for errors" << std::endl);
    return 0;
  }

  if (!this->Repository.RepositoryUpdate.empty() &&
      !this->Repository.PatchUpdatesXml()) {
    cmCPackIFWLogger(WARNING,
                     "Problem patch IFW \"Updates\" "
                       << "file: \"" << this->toplevel
                       << "/repository/Updates.xml\"" << std::endl);
  }

  cmCPackIFWLogger(OUTPUT,
                   "- repository: \"" << this->toplevel
                                      << "/repository\" generated"
                                      << std::endl);
  return 1;
}

std::vector<std::string> cmCPackIFWGenerator::BuildBinaryCreatorCommmand()
{
  std::vector<std::string> ifwCmd;
  std::string ifwArg;

  ifwCmd.emplace_back(this->BinCreator);

  if (!this->IsVersionLess("4.2")) {
    if (!this->ArchiveFormat.empty()) {
      ifwCmd.emplace_back("--archive-format");
      ifwCmd.emplace_back(this->ArchiveFormat);
    }
    if (!this->ArchiveCompression.empty()) {
      ifwCmd.emplace_back("--compression");
      ifwCmd.emplace_back(this->ArchiveCompression);
    }
  }

  if (!this->IsVersionLess("3.0")) {
#ifdef __APPLE__
    // macOS only
    std::string signingIdentity = this->Installer.SigningIdentity;
    if (!signingIdentity.empty()) {
      ifwCmd.emplace_back("--sign");
      ifwCmd.emplace_back(signingIdentity);
    }
#endif
  }

  ifwCmd.emplace_back("-c");
  ifwCmd.emplace_back(this->toplevel + "/config/config.xml");

  if (!this->Installer.Resources.empty()) {
    ifwCmd.emplace_back("-r");
    auto it = this->Installer.Resources.begin();
    std::string path = this->toplevel + "/resources/";
    ifwArg = path + *it;
    ++it;
    while (it != this->Installer.Resources.end()) {
      ifwArg += "," + path + *it;
      ++it;
    }
    ifwCmd.emplace_back(ifwArg);
  }

  ifwCmd.emplace_back("-p");
  ifwCmd.emplace_back(this->toplevel + "/packages");

  if (!this->PkgsDirsVector.empty()) {
    for (std::string const& it : this->PkgsDirsVector) {
      ifwCmd.emplace_back("-p");
      ifwCmd.emplace_back(it);
    }
  }

  if (!this->RepoDirsVector.empty()) {
    if (!this->IsVersionLess("3.1")) {
      for (std::string const& rd : this->RepoDirsVector) {
        ifwCmd.emplace_back("--repository");
        ifwCmd.emplace_back(rd);
      }
    } else {
      cmCPackIFWLogger(WARNING,
                       "The \"CPACK_IFW_REPOSITORIES_DIRECTORIES\" "
                         << "variable is set, but content will be skipped, "
                         << "because this feature available only since "
                         << "QtIFW 3.1. Please update your QtIFW instance."
                         << std::endl);
    }
  }

  if (this->OnlineOnly) {
    ifwCmd.emplace_back("--online-only");
  } else if (!this->DownloadedPackages.empty() &&
             !this->Installer.RemoteRepositories.empty()) {
    ifwCmd.emplace_back("-e");
    auto it = this->DownloadedPackages.begin();
    ifwArg = (*it)->Name;
    ++it;
    while (it != this->DownloadedPackages.end()) {
      ifwArg += "," + (*it)->Name;
      ++it;
    }
    ifwCmd.emplace_back(ifwArg);
  } else if (!this->DependentPackages.empty()) {
    ifwCmd.emplace_back("-i");
    ifwArg.clear();
    // Binary
    auto bit = this->BinaryPackages.begin();
    while (bit != this->BinaryPackages.end()) {
      ifwArg += (*bit)->Name + ",";
      ++bit;
    }
    // Depend
    auto it = this->DependentPackages.begin();
    ifwArg += it->second.Name;
    ++it;
    while (it != this->DependentPackages.end()) {
      ifwArg += "," + it->second.Name;
      ++it;
    }
    ifwCmd.emplace_back(ifwArg);
  }
  // TODO: set correct name for multipackages
  if (!this->packageFileNames.empty()) {
    ifwCmd.emplace_back(this->packageFileNames[0]);
  } else {
    ifwCmd.emplace_back("installer" + this->OutputExtension);
  }

  return ifwCmd;
}

int cmCPackIFWGenerator::RunBinaryCreator(const std::string& ifwTmpFile)
{
  std::vector<std::string> ifwCmd = this->BuildBinaryCreatorCommmand();
  cmCPackIFWLogger(VERBOSE,
                   "Execute: " << cmSystemTools::PrintSingleCommand(ifwCmd)
                               << std::endl);
  std::string output;
  int retVal = 1;
  cmCPackIFWLogger(OUTPUT, "- Generate package" << std::endl);
  bool res = cmSystemTools::RunSingleCommand(ifwCmd, &output, &output, &retVal,
                                             nullptr, this->GeneratorVerbose,
                                             cmDuration::zero());
  if (!res || retVal) {
    cmGeneratedFileStream ofs(ifwTmpFile);
    ofs << "# Run command: " << cmSystemTools::PrintSingleCommand(ifwCmd)
        << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackIFWLogger(
      ERROR,
      "Problem running IFW command: "
        << cmSystemTools::PrintSingleCommand(ifwCmd) << std::endl
        << "Please check \"" << ifwTmpFile << "\" for errors" << std::endl);
    return 0;
  }

  return 1;
}

const char* cmCPackIFWGenerator::GetPackagingInstallPrefix()
{
  const char* defPrefix = this->cmCPackGenerator::GetPackagingInstallPrefix();

  std::string tmpPref = defPrefix ? defPrefix : "";

  if (this->Components.empty()) {
    tmpPref += "packages/" + this->GetRootPackageName() + "/data";
  }

  this->SetOption("CPACK_IFW_PACKAGING_INSTALL_PREFIX", tmpPref);

  return this->GetOption("CPACK_IFW_PACKAGING_INSTALL_PREFIX")->c_str();
}

const char* cmCPackIFWGenerator::GetOutputExtension()
{
  return this->OutputExtension.c_str();
}

int cmCPackIFWGenerator::InitializeInternal()
{
  // Search Qt Installer Framework tools

  const std::string BinCreatorOpt = "CPACK_IFW_BINARYCREATOR_EXECUTABLE";
  const std::string RepoGenOpt = "CPACK_IFW_REPOGEN_EXECUTABLE";
  const std::string FrameworkVersionOpt = "CPACK_IFW_FRAMEWORK_VERSION";

  if (!this->IsSet(BinCreatorOpt) || !this->IsSet(RepoGenOpt) ||
      !this->IsSet(FrameworkVersionOpt)) {
    this->ReadListFile("CPackIFW.cmake");
  }

  // Look 'binarycreator' executable (needs)

  cmValue BinCreatorStr = this->GetOption(BinCreatorOpt);
  if (!BinCreatorStr || cmIsNOTFOUND(BinCreatorStr)) {
    this->BinCreator.clear();
  } else {
    this->BinCreator = *BinCreatorStr;
  }

  if (this->BinCreator.empty()) {
    cmCPackIFWLogger(ERROR,
                     "Cannot find QtIFW compiler \"binarycreator\": "
                     "likely it is not installed, or not in your PATH"
                       << std::endl);
    return 0;
  }

  // Look 'repogen' executable (optional)

  cmValue repoGen = this->GetOption(RepoGenOpt);
  if (!repoGen || cmIsNOTFOUND(repoGen)) {
    this->RepoGen.clear();
  } else {
    this->RepoGen = *repoGen;
  }

  // Framework version
  if (cmValue frameworkVersion = this->GetOption(FrameworkVersionOpt)) {
    this->FrameworkVersion = *frameworkVersion;
  } else {
    this->FrameworkVersion = "1.9.9";
  }

  // Variables that Change Behavior

  // Resolve duplicate names
  this->ResolveDuplicateNames =
    this->IsOn("CPACK_IFW_RESOLVE_DUPLICATE_NAMES");

  // Additional packages dirs
  this->PkgsDirsVector.clear();
  if (cmValue dirs = this->GetOption("CPACK_IFW_PACKAGES_DIRECTORIES")) {
    cmExpandList(dirs, this->PkgsDirsVector);
  }

  // Additional repositories dirs
  this->RepoDirsVector.clear();
  if (cmValue dirs = this->GetOption("CPACK_IFW_REPOSITORIES_DIRECTORIES")) {
    cmExpandList(dirs, this->RepoDirsVector);
  }

  // Archive format and compression level
  if (cmValue af = this->GetOption("CPACK_IFW_ARCHIVE_FORMAT")) {
    this->ArchiveFormat = *af;
  }
  if (cmValue ac = this->GetOption("CPACK_IFW_ARCHIVE_COMPRESSION")) {
    this->ArchiveCompression = *ac;
  }

  // Installer
  this->Installer.Generator = this;
  this->Installer.ConfigureFromOptions();

  // Repository
  this->Repository.Generator = this;
  this->Repository.Name = "Unspecified";
  if (cmValue site = this->GetOption("CPACK_DOWNLOAD_SITE")) {
    this->Repository.Url = *site;
    this->Installer.RemoteRepositories.push_back(&this->Repository);
  }

  // Repositories
  if (cmValue RepoAllStr = this->GetOption("CPACK_IFW_REPOSITORIES_ALL")) {
    std::vector<std::string> RepoAllVector = cmExpandedList(RepoAllStr);
    for (std::string const& r : RepoAllVector) {
      this->GetRepository(r);
    }
  }

  if (cmValue ifwDownloadAll = this->GetOption("CPACK_IFW_DOWNLOAD_ALL")) {
    this->OnlineOnly = cmIsOn(ifwDownloadAll);
  } else if (cmValue cpackDownloadAll =
               this->GetOption("CPACK_DOWNLOAD_ALL")) {
    this->OnlineOnly = cmIsOn(cpackDownloadAll);
  } else {
    this->OnlineOnly = false;
  }

  if (!this->Installer.RemoteRepositories.empty() && this->RepoGen.empty()) {
    cmCPackIFWLogger(ERROR,
                     "Cannot find QtIFW repository generator \"repogen\": "
                     "likely it is not installed, or not in your PATH"
                       << std::endl);
    return 0;
  }

  // Executable suffix
  std::string exeSuffix(this->GetOption("CMAKE_EXECUTABLE_SUFFIX"));
  std::string sysName(this->GetOption("CMAKE_SYSTEM_NAME"));
  if (sysName == "Linux") {
    this->ExecutableSuffix = ".run";
  } else if (sysName == "Windows") {
    this->ExecutableSuffix = ".exe";
  } else if (sysName == "Darwin") {
    this->ExecutableSuffix = ".app";
  } else {
    this->ExecutableSuffix = exeSuffix;
  }

  // Output extension
  if (cmValue optOutExt =
        this->GetOption("CPACK_IFW_PACKAGE_FILE_EXTENSION")) {
    this->OutputExtension = *optOutExt;
  } else if (sysName == "Darwin") {
    this->OutputExtension = ".dmg";
  } else {
    this->OutputExtension = this->ExecutableSuffix;
  }
  if (this->OutputExtension.empty()) {
    this->OutputExtension = this->cmCPackGenerator::GetOutputExtension();
  }

  return this->Superclass::InitializeInternal();
}

std::string cmCPackIFWGenerator::GetComponentInstallDirNameSuffix(
  const std::string& componentName)
{
  const std::string prefix = "packages/";
  const std::string suffix = "/data";

  if (this->componentPackageMethod == this->ONE_PACKAGE) {
    return cmStrCat(prefix, this->GetRootPackageName(), suffix);
  }

  return prefix +
    this->GetComponentPackageName(&this->Components[componentName]) + suffix;
}

cmCPackComponent* cmCPackIFWGenerator::GetComponent(
  const std::string& projectName, const std::string& componentName)
{
  auto cit = this->Components.find(componentName);
  if (cit != this->Components.end()) {
    return &(cit->second);
  }

  cmCPackComponent* component =
    this->cmCPackGenerator::GetComponent(projectName, componentName);
  if (!component) {
    return component;
  }

  std::string name = this->GetComponentPackageName(component);
  auto pit = this->Packages.find(name);
  if (pit != this->Packages.end()) {
    return component;
  }

  cmCPackIFWPackage* package = &this->Packages[name];
  package->Name = name;
  package->Generator = this;
  if (package->ConfigureFromComponent(component)) {
    package->Installer = &this->Installer;
    this->Installer.Packages.insert(
      std::pair<std::string, cmCPackIFWPackage*>(name, package));
    this->ComponentPackages.insert(
      std::pair<cmCPackComponent*, cmCPackIFWPackage*>(component, package));
    if (component->IsDownloaded) {
      this->DownloadedPackages.insert(package);
    } else {
      this->BinaryPackages.insert(package);
    }
  } else {
    this->Packages.erase(name);
    cmCPackIFWLogger(ERROR,
                     "Cannot configure package \""
                       << name << "\" for component \"" << component->Name
                       << "\"" << std::endl);
  }

  return component;
}

cmCPackComponentGroup* cmCPackIFWGenerator::GetComponentGroup(
  const std::string& projectName, const std::string& groupName)
{
  cmCPackComponentGroup* group =
    this->cmCPackGenerator::GetComponentGroup(projectName, groupName);
  if (!group) {
    return group;
  }

  std::string name = this->GetGroupPackageName(group);
  auto pit = this->Packages.find(name);
  if (pit != this->Packages.end()) {
    return group;
  }

  cmCPackIFWPackage* package = &this->Packages[name];
  package->Name = name;
  package->Generator = this;
  if (package->ConfigureFromGroup(group)) {
    package->Installer = &this->Installer;
    this->Installer.Packages.insert(
      std::pair<std::string, cmCPackIFWPackage*>(name, package));
    this->GroupPackages.insert(
      std::pair<cmCPackComponentGroup*, cmCPackIFWPackage*>(group, package));
    this->BinaryPackages.insert(package);
  } else {
    this->Packages.erase(name);
    cmCPackIFWLogger(ERROR,
                     "Cannot configure package \""
                       << name << "\" for component group \"" << group->Name
                       << "\"" << std::endl);
  }
  return group;
}

enum cmCPackGenerator::CPackSetDestdirSupport
cmCPackIFWGenerator::SupportsSetDestdir() const
{
  return cmCPackGenerator::SETDESTDIR_SHOULD_NOT_BE_USED;
}

bool cmCPackIFWGenerator::SupportsAbsoluteDestination() const
{
  return false;
}

bool cmCPackIFWGenerator::SupportsComponentInstallation() const
{
  return true;
}

bool cmCPackIFWGenerator::IsOnePackage() const
{
  return this->componentPackageMethod == cmCPackGenerator::ONE_PACKAGE;
}

std::string cmCPackIFWGenerator::GetRootPackageName()
{
  // Default value
  std::string name = "root";
  if (cmValue optIFW_PACKAGE_GROUP =
        this->GetOption("CPACK_IFW_PACKAGE_GROUP")) {
    // Configure from root group
    cmCPackIFWPackage package;
    package.Generator = this;
    package.ConfigureFromGroup(optIFW_PACKAGE_GROUP);
    name = package.Name;
  } else if (cmValue optIFW_PACKAGE_NAME =
               this->GetOption("CPACK_IFW_PACKAGE_NAME")) {
    // Configure from root package name
    name = *optIFW_PACKAGE_NAME;
  } else if (cmValue optPACKAGE_NAME = this->GetOption("CPACK_PACKAGE_NAME")) {
    // Configure from package name
    name = *optPACKAGE_NAME;
  }
  return name;
}

std::string cmCPackIFWGenerator::GetGroupPackageName(
  cmCPackComponentGroup* group) const
{
  std::string name;
  if (!group) {
    return name;
  }
  if (cmCPackIFWPackage* package = this->GetGroupPackage(group)) {
    return package->Name;
  }
  cmValue option =
    this->GetOption("CPACK_IFW_COMPONENT_GROUP_" +
                    cmsys::SystemTools::UpperCase(group->Name) + "_NAME");
  name = option ? *option : group->Name;
  if (group->ParentGroup) {
    cmCPackIFWPackage* package = this->GetGroupPackage(group->ParentGroup);
    bool dot = !this->ResolveDuplicateNames;
    if (dot && !cmHasPrefix(name, package->Name)) {
      name = package->Name + "." + name;
    }
  }
  return name;
}

std::string cmCPackIFWGenerator::GetComponentPackageName(
  cmCPackComponent* component) const
{
  std::string name;
  if (!component) {
    return name;
  }
  if (cmCPackIFWPackage* package = this->GetComponentPackage(component)) {
    return package->Name;
  }
  std::string prefix = "CPACK_IFW_COMPONENT_" +
    cmsys::SystemTools::UpperCase(component->Name) + "_";
  cmValue option = this->GetOption(prefix + "NAME");
  name = option ? *option : component->Name;
  if (component->Group) {
    cmCPackIFWPackage* package = this->GetGroupPackage(component->Group);
    if ((this->componentPackageMethod ==
         cmCPackGenerator::ONE_PACKAGE_PER_GROUP) ||
        this->IsOn(prefix + "COMMON")) {
      return package->Name;
    }
    bool dot = !this->ResolveDuplicateNames;
    if (dot && !cmHasPrefix(name, package->Name)) {
      name = package->Name + "." + name;
    }
  }
  return name;
}

cmCPackIFWPackage* cmCPackIFWGenerator::GetGroupPackage(
  cmCPackComponentGroup* group) const
{
  auto pit = this->GroupPackages.find(group);
  return pit != this->GroupPackages.end() ? pit->second : nullptr;
}

cmCPackIFWPackage* cmCPackIFWGenerator::GetComponentPackage(
  cmCPackComponent* component) const
{
  auto pit = this->ComponentPackages.find(component);
  return pit != this->ComponentPackages.end() ? pit->second : nullptr;
}

cmCPackIFWRepository* cmCPackIFWGenerator::GetRepository(
  const std::string& repositoryName)
{
  auto rit = this->Repositories.find(repositoryName);
  if (rit != this->Repositories.end()) {
    return &(rit->second);
  }

  cmCPackIFWRepository* repository = &this->Repositories[repositoryName];
  repository->Name = repositoryName;
  repository->Generator = this;
  if (repository->ConfigureFromOptions()) {
    if (repository->Update == cmCPackIFWRepository::None) {
      this->Installer.RemoteRepositories.push_back(repository);
    } else {
      this->Repository.RepositoryUpdate.push_back(repository);
    }
  } else {
    this->Repositories.erase(repositoryName);
    repository = nullptr;
    cmCPackIFWLogger(WARNING,
                     "Invalid repository \""
                       << repositoryName << "\""
                       << " configuration. Repository will be skipped."
                       << std::endl);
  }
  return repository;
}
