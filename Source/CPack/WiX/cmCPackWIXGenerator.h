/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackWIXGenerator_h
#define cmCPackWIXGenerator_h

#include <CPack/cmCPackGenerator.h>

#include <string>
#include <map>

struct cmWIXShortcut
{
  std::string textLabel;
  std::string workingDirectoryId;
};

class cmWIXSourceWriter;

/** \class cmCPackWIXGenerator
 * \brief A generator for WIX files
 */
class cmCPackWIXGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackWIXGenerator, cmCPackGenerator);

protected:
  virtual int InitializeInternal();

  virtual int PackageFiles();

  virtual const char* GetOutputExtension()
    {
    return ".msi";
    }

  virtual enum CPackSetDestdirSupport SupportsSetDestdir() const
    {
    return SETDESTDIR_UNSUPPORTED;
    }

  virtual bool SupportsAbsoluteDestination() const
    {
    return false;
    }

  virtual bool SupportsComponentInstallation() const
    {
    return true;
    }

private:
  typedef std::map<std::string, std::string> id_map_t;
  typedef std::map<std::string, size_t> ambiguity_map_t;
  typedef std::map<std::string, cmWIXShortcut> shortcut_map_t;
  typedef std::set<std::string> extension_set_t;

  bool InitializeWiXConfiguration();

  bool PackageFilesImpl();

  bool CreateWiXVariablesIncludeFile();

  void CopyDefinition(
    cmWIXSourceWriter &source, const std::string &name);

  void AddDefinition(cmWIXSourceWriter& source,
    const std::string& name, const std::string& value);

  bool CreateWiXSourceFiles();

  bool CreateFeatureHierarchy(
    cmWIXSourceWriter& featureDefinitions);

  bool EmitFeatureForComponentGroup(
    cmWIXSourceWriter& featureDefinitions,
    cmCPackComponentGroup const& group);

  bool EmitFeatureForComponent(
    cmWIXSourceWriter& featureDefinitions,
    cmCPackComponent const& component);

  bool AddComponentsToFeature(
    std::string const& rootPath,
    std::string const& featureId,
    cmWIXSourceWriter& directoryDefinitions,
    cmWIXSourceWriter& fileDefinitions,
    cmWIXSourceWriter& featureDefinitions,
    shortcut_map_t& shortcutMap);

  bool CreateStartMenuShortcuts(
    std::string const& cpackComponentName,
    std::string const& featureId,
    shortcut_map_t& shortcutMap,
    cmWIXSourceWriter& fileDefinitions,
    cmWIXSourceWriter& featureDefinitions);

  void CreateUninstallShortcut(
    std::string const& packageName,
    cmWIXSourceWriter& fileDefinitions);

  void AppendUserSuppliedExtraSources();

  void AppendUserSuppliedExtraObjects(std::ostream& stream);

  bool CreateLicenseFile();

  bool RunWiXCommand(const std::string& command);

  bool RunCandleCommand(
    const std::string& sourceFile, const std::string& objectFile);

  bool RunLightCommand(const std::string& objectFiles);

  void AddDirectoryAndFileDefinitons(const std::string& topdir,
    const std::string& directoryId,
    cmWIXSourceWriter& directoryDefinitions,
    cmWIXSourceWriter& fileDefinitions,
    cmWIXSourceWriter& featureDefinitions,
    const std::vector<std::string>& pkgExecutables,
    shortcut_map_t& shortcutMap);

  bool RequireOption(const std::string& name, std::string& value) const;

  std::string GetArchitecture() const;

  static std::string GenerateGUID();

  static std::string QuotePath(const std::string& path);

  static std::string GetRightmostExtension(const std::string& filename);

  std::string PathToId(const std::string& path);

  std::string CreateNewIdForPath(const std::string& path);

  static std::string CreateHashedId(
    const std::string& path, const std::string& normalizedFilename);

  std::string NormalizeComponentForId(
    const std::string& component, size_t& replacementCount);

  static bool IsLegalIdCharacter(char c);

  void CollectExtensions(
       const std::string& variableName, extension_set_t& extensions);

  void AddCustomFlags(
    const std::string& variableName, std::ostream& stream);

  void CreateStartMenuFolder(cmWIXSourceWriter& directoryDefinitions);

  std::vector<std::string> wixSources;
  id_map_t pathToIdMap;
  ambiguity_map_t idAmbiguityCounter;

  extension_set_t candleExtensions;
  extension_set_t lightExtensions;
};

#endif
