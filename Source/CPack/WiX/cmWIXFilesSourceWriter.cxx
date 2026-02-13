/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#if defined(__CYGWIN__)
// For S_IWRITE symbol
#  define _DEFAULT_SOURCE
#endif

#include "cmWIXFilesSourceWriter.h"

#include "cm_sys_stat.h"

#include "cmCMakeToWixPath.h"
#include "cmInstalledFile.h"
#include "cmSystemTools.h"
#include "cmUuid.h"
#include "cmWIXAccessControlList.h"

#ifdef _WIN32
#  include "cmsys/Encoding.hxx"
#endif

cmWIXFilesSourceWriter::cmWIXFilesSourceWriter(
  unsigned long wixVersion, cmCPackLog* logger, std::string const& filename,
  GuidType componentGuidType, cmWIXInstallScope installScope,
  std::string componentKeysRegistryPath)
  : cmWIXSourceWriter(wixVersion, logger, filename, componentGuidType)
  , ComponentKeysRegistryPath(std::move(componentKeysRegistryPath))
{
  switch (installScope) {
    case cmWIXInstallScope::PER_USER:
      this->PerUserInstall = true;
      break;
    case cmWIXInstallScope::PER_MACHINE:
    case cmWIXInstallScope::NONE:
      this->PerUserInstall = false;
      break;
    default:
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Unhandled install scope value, this is a CPack Bug.");
      break;
  }
}

void cmWIXFilesSourceWriter::EmitShortcut(std::string const& id,
                                          cmWIXShortcut const& shortcut,
                                          std::string const& shortcutPrefix,
                                          size_t shortcutIndex)
{
  std::ostringstream shortcutId;
  shortcutId << shortcutPrefix << id;

  if (shortcutIndex > 0) {
    shortcutId << "_" << shortcutIndex;
  }

  std::string fileId = std::string("CM_F") + id;

  BeginElement("Shortcut");
  AddAttribute("Id", shortcutId.str());
  AddAttribute("Name", shortcut.label);
  std::string target = "[#" + fileId + "]";
  AddAttribute("Target", target);
  AddAttribute("WorkingDirectory", shortcut.workingDirectoryId);
  EndElement("Shortcut");
}

void cmWIXFilesSourceWriter::EmitRemoveFolder(std::string const& id)
{
  BeginElement("RemoveFolder");
  AddAttribute("Id", id);
  AddAttribute("On", "uninstall");
  EndElement("RemoveFolder");
}

void cmWIXFilesSourceWriter::EmitInstallRegistryValue(
  std::string const& registryKey, std::string const& cpackComponentName,
  std::string const& suffix)
{
  std::string valueName;
  if (!cpackComponentName.empty()) {
    valueName = cpackComponentName + "_";
  }

  valueName += "installed";
  valueName += suffix;

  BeginElement("RegistryValue");
  AddAttribute("Root", "HKCU");
  AddAttribute("Key", registryKey);
  AddAttribute("Name", valueName);
  AddAttribute("Type", "integer");
  AddAttribute("Value", "1");
  AddAttribute("KeyPath", "yes");
  EndElement("RegistryValue");
}

void cmWIXFilesSourceWriter::EmitUninstallShortcut(
  std::string const& packageName)
{
  BeginElement("Shortcut");
  AddAttribute("Id", "UNINSTALL");
  AddAttribute("Name", "Uninstall " + packageName);
  AddAttribute("Description", "Uninstalls " + packageName);
  AddAttribute("Target", "[SystemFolder]msiexec.exe");
  AddAttribute("Arguments", "/x [ProductCode]");
  EndElement("Shortcut");
}

std::string cmWIXFilesSourceWriter::EmitComponentCreateFolder(
  std::string const& directoryId, std::string const& guid,
  cmInstalledFile const* installedFile)
{
  std::string componentId = std::string("CM_C_EMPTY_") + directoryId;

  BeginElement("DirectoryRef");
  AddAttribute("Id", directoryId);

  BeginElement("Component");
  AddAttribute("Id", componentId);
  AddAttribute("Guid", guid);

  BeginElement("CreateFolder");

  if (installedFile) {
    cmWIXAccessControlList acl(Logger, *installedFile, *this);
    acl.Apply();
  }

  EndElement("CreateFolder");
  EndElement("Component");
  EndElement("DirectoryRef");

  return componentId;
}

std::string cmWIXFilesSourceWriter::EmitComponentFile(
  std::string const& directoryId, std::string const& id,
  std::string const& filePath, cmWIXPatch& patch,
  cmInstalledFile const* installedFile, int diskId)
{
  std::string componentId = std::string("CM_C") + id;
  std::string fileId = std::string("CM_F") + id;

  // Wix doesn't support automatic GUIDs for components which have both
  // registry entries and files.
  std::string guid = this->PerUserInstall
    ? CreateCmakeGeneratedGuidFromComponentId(componentId)
    : CreateGuidFromComponentId(componentId);

  BeginElement("DirectoryRef");
  AddAttribute("Id", directoryId);

  BeginElement("Component");
  AddAttribute("Id", componentId);
  AddAttribute("Guid", guid);

  if (diskId) {
    AddAttribute("DiskId", std::to_string(diskId));
  }

  if (installedFile) {
    if (installedFile->GetPropertyAsBool("CPACK_NEVER_OVERWRITE")) {
      AddAttribute("NeverOverwrite", "yes");
    }
    if (installedFile->GetPropertyAsBool("CPACK_PERMANENT")) {
      AddAttribute("Permanent", "yes");
    }
  }

  patch.ApplyFragment(componentId, *this);

  if (this->PerUserInstall) {
    // For perUser installs, MSI requires using a registry entry as the key
    // path for components.
    BeginElement("RegistryValue");

    AddAttribute("Root", "HKCU");
    AddAttribute("Key", this->ComponentKeysRegistryPath);
    AddAttribute("Name", fileId);
    AddAttribute("Type", "string");
    AddAttribute("Value", "1");
    AddAttribute("KeyPath", "yes");

    EndElement("RegistryValue");
  }

  BeginElement("File");
  AddAttribute("Id", fileId);

  std::string sourcePath = CMakeToWixPath(filePath);
#ifdef _WIN32
  // WiX cannot handle long paths natively, but since v4,
  // it supports long paths via UNC prefixes.
  if (this->WixVersion >= 4) {
    sourcePath = cmsys::Encoding::ToNarrow(
      cmsys::Encoding::ToWindowsExtendedPath(sourcePath));
  }
#endif
  AddAttribute("Source", sourcePath);

  if (!this->PerUserInstall) {
    AddAttribute("KeyPath", "yes");
  }

  mode_t fileMode = 0;
  cmSystemTools::GetPermissions(filePath.c_str(), fileMode);

  if (!(fileMode & S_IWRITE)) {
    AddAttribute("ReadOnly", "yes");
  }
  patch.ApplyFragment(fileId, *this);

  if (installedFile) {
    cmWIXAccessControlList acl(Logger, *installedFile, *this);
    acl.Apply();
  }

  EndElement("File");

  EndElement("Component");
  EndElement("DirectoryRef");

  return componentId;
}
