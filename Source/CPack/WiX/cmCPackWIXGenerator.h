/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackWIXGenerator_h
#define cmCPackWIXGenerator_h

#include <map>
#include <string>

#include "cmCPackGenerator.h"
#include "cmWIXPatch.h"
#include "cmWIXShortcut.h"

class cmWIXSourceWriter;
class cmWIXDirectoriesSourceWriter;
class cmWIXFilesSourceWriter;
class cmWIXFeaturesSourceWriter;

/** \class cmCPackWIXGenerator
 * \brief A generator for WIX files
 */
class cmCPackWIXGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackWIXGenerator, cmCPackGenerator);

  cmCPackWIXGenerator();
  ~cmCPackWIXGenerator();

protected:
  int InitializeInternal() override;

  int PackageFiles() override;

  const char* GetOutputExtension() override { return ".msi"; }

  enum CPackSetDestdirSupport SupportsSetDestdir() const override
  {
    return SETDESTDIR_UNSUPPORTED;
  }

  bool SupportsAbsoluteDestination() const override { return false; }

  bool SupportsComponentInstallation() const override { return true; }

private:
  using id_map_t = std::map<std::string, std::string>;
  using ambiguity_map_t = std::map<std::string, size_t>;
  using extension_set_t = std::set<std::string>;

  enum class DefinitionType
  {
    STRING,
    PATH
  };

  bool InitializeWiXConfiguration();

  bool PackageFilesImpl();

  void CreateWiXVariablesIncludeFile();

  void CreateWiXPropertiesIncludeFile();

  void CreateWiXProductFragmentIncludeFile();

  void CopyDefinition(cmWIXSourceWriter& source, std::string const& name,
                      DefinitionType type = DefinitionType::STRING);

  void AddDefinition(cmWIXSourceWriter& source, std::string const& name,
                     std::string const& value);

  bool CreateWiXSourceFiles();

  std::string GetRootFolderId() const;

  bool GenerateMainSourceFileFromTemplate();

  bool CreateFeatureHierarchy(cmWIXFeaturesSourceWriter& featureDefinitions);

  bool AddComponentsToFeature(
    std::string const& rootPath, std::string const& featureId,
    cmWIXDirectoriesSourceWriter& directoryDefinitions,
    cmWIXFilesSourceWriter& fileDefinitions,
    cmWIXFeaturesSourceWriter& featureDefinitions, cmWIXShortcuts& shortcuts);

  bool CreateShortcuts(std::string const& cpackComponentName,
                       std::string const& featureId,
                       cmWIXShortcuts const& shortcuts,
                       bool emitUninstallShortcut,
                       cmWIXFilesSourceWriter& fileDefinitions,
                       cmWIXFeaturesSourceWriter& featureDefinitions);

  bool CreateShortcutsOfSpecificType(
    cmWIXShortcuts::Type type, std::string const& cpackComponentName,
    std::string const& featureId, std::string const& idPrefix,
    cmWIXShortcuts const& shortcuts, bool emitUninstallShortcut,
    cmWIXFilesSourceWriter& fileDefinitions,
    cmWIXFeaturesSourceWriter& featureDefinitions);

  void AppendUserSuppliedExtraSources();

  void AppendUserSuppliedExtraObjects(std::ostream& stream);

  bool CreateLicenseFile();

  bool RunWiXCommand(std::string const& command);

  bool RunCandleCommand(std::string const& sourceFile,
                        std::string const& objectFile);

  bool RunLightCommand(std::string const& objectFiles);

  void AddDirectoryAndFileDefinitions(
    std::string const& topdir, std::string const& directoryId,
    cmWIXDirectoriesSourceWriter& directoryDefinitions,
    cmWIXFilesSourceWriter& fileDefinitions,
    cmWIXFeaturesSourceWriter& featureDefinitions,
    std::vector<std::string> const& packageExecutables,
    std::vector<std::string> const& desktopExecutables,
    cmWIXShortcuts& shortcuts);

  bool RequireOption(std::string const& name, std::string& value) const;

  std::string GetArchitecture() const;

  static std::string GenerateGUID();

  static std::string QuotePath(std::string const& path);

  static std::string GetRightmostExtension(std::string const& filename);

  std::string PathToId(std::string const& path);

  std::string CreateNewIdForPath(std::string const& path);

  static std::string CreateHashedId(std::string const& path,
                                    std::string const& normalizedFilename);

  std::string NormalizeComponentForId(std::string const& component,
                                      size_t& replacementCount);

  static bool IsLegalIdCharacter(char c);

  void CollectExtensions(std::string const& variableName,
                         extension_set_t& extensions);

  void AddCustomFlags(std::string const& variableName, std::ostream& stream);

  std::string RelativePathWithoutComponentPrefix(std::string const& path);

  std::vector<std::string> WixSources;
  id_map_t PathToIdMap;
  ambiguity_map_t IdAmbiguityCounter;

  extension_set_t CandleExtensions;
  extension_set_t LightExtensions;

  std::string CPackTopLevel;

  cmWIXPatch* Patch;

  cmWIXSourceWriter::GuidType ComponentGuidType;
};

#endif
