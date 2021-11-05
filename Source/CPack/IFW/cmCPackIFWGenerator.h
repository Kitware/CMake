/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmCPackGenerator.h"
#include "cmCPackIFWCommon.h"
#include "cmCPackIFWInstaller.h"
#include "cmCPackIFWPackage.h"
#include "cmCPackIFWRepository.h"

class cmCPackComponent;
class cmCPackComponentGroup;

/** \class cmCPackIFWGenerator
 * \brief A generator for Qt Installer Framework tools
 *
 * http://qt-project.org/doc/qtinstallerframework/index.html
 */
class cmCPackIFWGenerator
  : public cmCPackGenerator
  , public cmCPackIFWCommon
{
public:
  cmCPackTypeMacro(cmCPackIFWGenerator, cmCPackGenerator);

  using PackagesMap = std::map<std::string, cmCPackIFWPackage>;
  using RepositoriesMap = std::map<std::string, cmCPackIFWRepository>;
  using DependenceMap =
    std::map<std::string, cmCPackIFWPackage::DependenceStruct>;

  using cmCPackIFWCommon::GetOption;
  using cmCPackIFWCommon::IsOn;
  using cmCPackIFWCommon::IsSetToOff;
  using cmCPackIFWCommon::IsSetToEmpty;

  /**
   * Construct IFW generator
   */
  cmCPackIFWGenerator();

  /**
   * Destruct IFW generator
   */
  ~cmCPackIFWGenerator() override;

protected:
  // cmCPackGenerator reimplementation

  /**
   * @brief Initialize generator
   * @return 0 on failure
   */
  int InitializeInternal() override;
  int PackageFiles() override;
  const char* GetPackagingInstallPrefix() override;

  /**
   * @brief Target binary extension
   * @return Executable suffix or disk image format
   */
  const char* GetOutputExtension() override;

  std::string GetComponentInstallDirNameSuffix(
    const std::string& componentName) override;

  /**
   * @brief Get Component
   * @param projectName Project name
   * @param componentName Component name
   *
   * This method calls the base implementation.
   *
   * @return Pointer to component
   */
  cmCPackComponent* GetComponent(const std::string& projectName,
                                 const std::string& componentName) override;

  /**
   * @brief Get group of component
   * @param projectName Project name
   * @param groupName Component group name
   *
   * This method calls the base implementation.
   *
   * @return Pointer to component group
   */
  cmCPackComponentGroup* GetComponentGroup(
    const std::string& projectName, const std::string& groupName) override;

  enum cmCPackGenerator::CPackSetDestdirSupport SupportsSetDestdir()
    const override;
  bool SupportsAbsoluteDestination() const override;
  bool SupportsComponentInstallation() const override;

protected:
  // Methods

  bool IsOnePackage() const;

  std::string GetRootPackageName();

  std::string GetGroupPackageName(cmCPackComponentGroup* group) const;
  std::string GetComponentPackageName(cmCPackComponent* component) const;

  cmCPackIFWPackage* GetGroupPackage(cmCPackComponentGroup* group) const;
  cmCPackIFWPackage* GetComponentPackage(cmCPackComponent* component) const;

  cmCPackIFWRepository* GetRepository(const std::string& repositoryName);

protected:
  // Data

  friend class cmCPackIFWPackage;
  friend class cmCPackIFWCommon;
  friend class cmCPackIFWInstaller;
  friend class cmCPackIFWRepository;

  // Installer
  cmCPackIFWInstaller Installer;
  // Repository
  cmCPackIFWRepository Repository;
  // Collection of packages
  PackagesMap Packages;
  // Collection of repositories
  RepositoriesMap Repositories;
  // Collection of binary packages
  std::set<cmCPackIFWPackage*> BinaryPackages;
  // Collection of downloaded packages
  std::set<cmCPackIFWPackage*> DownloadedPackages;
  // Dependent packages
  DependenceMap DependentPackages;
  std::map<cmCPackComponent*, cmCPackIFWPackage*> ComponentPackages;
  std::map<cmCPackComponentGroup*, cmCPackIFWPackage*> GroupPackages;

private:
  std::vector<std::string> BuildRepogenCommand();
  int RunRepogen(const std::string& ifwTmpFile);

  std::vector<std::string> BuildBinaryCreatorCommmand();
  int RunBinaryCreator(const std::string& ifwTmpFile);

  std::string RepoGen;
  std::string BinCreator;
  std::string FrameworkVersion;
  std::string ExecutableSuffix;
  std::string OutputExtension;
  std::string ArchiveFormat;
  std::string ArchiveCompression;

  bool OnlineOnly{};
  bool ResolveDuplicateNames{};
  std::vector<std::string> PkgsDirsVector;
  std::vector<std::string> RepoDirsVector;
};
