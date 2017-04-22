/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackIFWPackage_h
#define cmCPackIFWPackage_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <set>
#include <string>
#include <vector>

class cmCPackComponent;
class cmCPackComponentGroup;
class cmCPackIFWGenerator;
class cmCPackIFWInstaller;
class cmXMLWriter;

/** \class cmCPackIFWPackage
 * \brief A single component to be installed by CPack IFW generator
 */
class cmCPackIFWPackage
{
public:
  // Types

  enum CompareTypes
  {
    CompareNone = 0x0,
    CompareEqual = 0x1,
    CompareLess = 0x2,
    CompareLessOrEqual = 0x3,
    CompareGreater = 0x4,
    CompareGreaterOrEqual = 0x5
  };

  struct CompareStruct
  {
    CompareStruct();

    unsigned int Type;
    std::string Value;
  };

  struct DependenceStruct
  {
    DependenceStruct();
    DependenceStruct(const std::string& dependence);

    std::string Name;
    CompareStruct Compare;

    std::string NameWithCompare() const;

    bool operator<(const DependenceStruct& other) const
    {
      return Name < other.Name;
    }
  };

public:
  // [Con|De]structor

  /**
   * Construct package
   */
  cmCPackIFWPackage();

public:
  // Configuration

  /// Human-readable name of the component
  std::string DisplayName;

  /// Human-readable description of the component
  std::string Description;

  /// Version number of the component
  std::string Version;

  /// Date when this component version was released
  std::string ReleaseDate;

  /// Domain-like identification for this component
  std::string Name;

  /// File name of a script being loaded
  std::string Script;

  /// List of license agreements to be accepted by the installing user
  std::vector<std::string> Licenses;

  /// List of pages to load
  std::vector<std::string> UserInterfaces;

  /// List of translation files to load
  std::vector<std::string> Translations;

  /// Priority of the component in the tree
  std::string SortingPriority;

  /// Description added to the component description
  std::string UpdateText;

  /// Set to true to preselect the component in the installer
  std::string Default;

  /// Marks the package as essential to force a restart of the MaintenanceTool
  std::string Essential;

  /// Set to true to hide the component from the installer
  std::string Virtual;

  /// Determines that the package must always be installed
  std::string ForcedInstallation;

  /// Package needs to be installed with elevated permissions
  std::string RequiresAdminRights;

public:
  // Internal implementation

  const char* GetOption(const std::string& op) const;
  bool IsOn(const std::string& op) const;
  bool IsSetToOff(const std::string& op) const;
  bool IsSetToEmpty(const std::string& op) const;

  bool IsVersionLess(const char* version);
  bool IsVersionGreater(const char* version);
  bool IsVersionEqual(const char* version);

  std::string GetComponentName(cmCPackComponent* component);

  void DefaultConfiguration();

  int ConfigureFromOptions();
  int ConfigureFromComponent(cmCPackComponent* component);
  int ConfigureFromGroup(cmCPackComponentGroup* group);
  int ConfigureFromGroup(const std::string& groupName);
  int ConfigureFromPrefix(const std::string& prefix);

  void GeneratePackageFile();

  // Pointer to generator
  cmCPackIFWGenerator* Generator;
  // Pointer to installer
  cmCPackIFWInstaller* Installer;
  // Collection of dependencies
  std::set<cmCPackIFWPackage*> Dependencies;
  // Collection of unresolved dependencies
  std::set<DependenceStruct*> AlienDependencies;
  // Collection of unresolved automatic dependency on
  std::set<DependenceStruct*> AlienAutoDependOn;
  // Patch to package directory
  std::string Directory;

protected:
  void WriteGeneratedByToStrim(cmXMLWriter& xout);
};

#endif // cmCPackIFWPackage_h
