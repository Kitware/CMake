/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackIFWGenerator_h
#define cmCPackIFWGenerator_h

#include <cmConfigure.h>

#include "CPack/cmCPackComponentGroup.h"
#include "CPack/cmCPackGenerator.h"
#include "cmCPackIFWInstaller.h"
#include "cmCPackIFWPackage.h"
#include "cmCPackIFWRepository.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class cmXMLWriter;

/** \class cmCPackIFWGenerator
 * \brief A generator for Qt Installer Framework tools
 *
 * http://qt-project.org/doc/qtinstallerframework/index.html
 */
class cmCPackIFWGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackIFWGenerator, cmCPackGenerator);

  typedef std::map<std::string, cmCPackIFWPackage> PackagesMap;
  typedef std::map<std::string, cmCPackIFWRepository> RepositoriesMap;
  typedef std::map<std::string, cmCPackComponent> ComponentsMap;
  typedef std::map<std::string, cmCPackComponentGroup> ComponentGoupsMap;
  typedef std::map<std::string, cmCPackIFWPackage::DependenceStruct>
    DependenceMap;

  /**
   * Construct IFW generator
   */
  cmCPackIFWGenerator();

  /**
   * Destruct IFW generator
   */
  ~cmCPackIFWGenerator() CM_OVERRIDE;

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionLess(const char* version);

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionGreater(const char* version);

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionEqual(const char* version);

protected:
  // cmCPackGenerator reimplementation

  /**
   * @brief Initialize generator
   * @return 0 on failure
   */
  int InitializeInternal() CM_OVERRIDE;
  int PackageFiles() CM_OVERRIDE;
  const char* GetPackagingInstallPrefix() CM_OVERRIDE;

  /**
   * @brief Extension of binary installer
   * @return Executable suffix or value from default implementation
   */
  const char* GetOutputExtension() CM_OVERRIDE;

  std::string GetComponentInstallDirNameSuffix(
    const std::string& componentName) CM_OVERRIDE;

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
                                 const std::string& componentName) CM_OVERRIDE;

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
    const std::string& projectName, const std::string& groupName) CM_OVERRIDE;

  enum cmCPackGenerator::CPackSetDestdirSupport SupportsSetDestdir() const
    CM_OVERRIDE;
  bool SupportsAbsoluteDestination() const CM_OVERRIDE;
  bool SupportsComponentInstallation() const CM_OVERRIDE;

protected:
  // Methods

  bool IsOnePackage() const;

  std::string GetRootPackageName();

  std::string GetGroupPackageName(cmCPackComponentGroup* group) const;
  std::string GetComponentPackageName(cmCPackComponent* component) const;

  cmCPackIFWPackage* GetGroupPackage(cmCPackComponentGroup* group) const;
  cmCPackIFWPackage* GetComponentPackage(cmCPackComponent* component) const;

  cmCPackIFWRepository* GetRepository(const std::string& repositoryName);

  void WriteGeneratedByToStrim(cmXMLWriter& xout);

protected:
  // Data

  friend class cmCPackIFWPackage;
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
  std::string RepoGen;
  std::string BinCreator;
  std::string FrameworkVersion;
  std::string ExecutableSuffix;

  bool OnlineOnly;
  bool ResolveDuplicateNames;
  std::vector<std::string> PkgsDirsVector;
};

#endif
